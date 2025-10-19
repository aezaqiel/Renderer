#include "RenderGraph.hpp"

#include <queue>
#include <set>
#include <algorithm>

namespace Renderer {

    ResourceHandle RenderGraph::CreateImage(const std::string& name, ImageDesc desc, bool imported)
    {
        m_Resources.push_back(Resource {
            .type = ResourceType::Image,
            .name = name,
            .imageDesc = desc,
            .imported = imported
        });
        return static_cast<ResourceHandle>(m_Resources.size() - 1);
    }

    PassHandle RenderGraph::AddPass(const std::string& name, std::function<void(class RenderGraph::PassBuilder&)> setup, std::function<void(VkCommandBuffer, const std::unordered_map<ResourceHandle, VkImageView>&)> record)
    {
        Pass pass;
        pass.name = name;
        pass.record = record;

        PassBuilder builder(pass);
        setup(builder);
        m_Passes.push_back(std::move(pass));

        return static_cast<PassHandle>(m_Passes.size() - 1);
    }

    ExecutionPlan RenderGraph::Compile()
    {
        std::vector<std::vector<std::pair<PassHandle, AccessInfo>>> resourceUses(m_Resources.size());
        for (PassHandle pi = 0; pi < m_Passes.size(); ++pi) {
            for (const auto& ai : m_Passes[pi].accesses) {
                resourceUses.at(ai.resource).push_back({pi, ai});
            }
        }

        std::vector<char> passAlive(m_Passes.size(), 0);
        std::queue<PassHandle> q;
        for (ResourceHandle r = 0; r < m_Resources.size(); ++r) {
            if (m_Resources[r].imported) {
                for (auto& pr : resourceUses[r]) {
                    if (!passAlive.at(pr.first)) {
                        passAlive.at(pr.first) = 1;
                        q.push(pr.first);
                    }
                }
            }
        }

        if (q.empty()) {
            for (usize i = 0; i < passAlive.size(); ++i) passAlive[i] = 1;
        } else {
            while (!q.empty()) {
                auto pIdx = q.front();
                q.pop();

                for (const auto& ai : m_Passes.at(pIdx).accesses) {
                    auto r = ai.resource;

                    for (auto& pr : resourceUses.at(r)) {
                        auto otherPass = pr.first;
                        bool isWrite = (pr.second.type == AccessType::Write || pr.second.type == AccessType::ReadWrite);

                        if (otherPass < pIdx && !passAlive.at(otherPass) && isWrite) {
                            passAlive.at(otherPass) = 1;
                            q.push(otherPass);
                        }
                    }
                }
            }
        }

        std::vector<PassHandle> alivePasses;
        alivePasses.reserve(m_Passes.size());
        std::vector<i32> passRemap(m_Passes.size(), -1);
        for (PassHandle i = 0; i < m_Passes.size(); ++i) {
            if (passAlive[i]) {
                passRemap[i] = static_cast<i32>(alivePasses.size());
                alivePasses.push_back(i);
            }
        }

        if (alivePasses.empty()) return {};

        usize N = alivePasses.size();
        std::vector<std::vector<i32>> adj(N);
        std::vector<i32> indeg(N, 0);
        std::set<std::pair<i32, i32>> edges;

        for (ResourceHandle r = 0; r < m_Resources.size(); ++r) {
            std::vector<std::pair<PassHandle, AccessInfo>> uses;

            for (auto& pr : resourceUses.at(r)) {
                if (passRemap.at(pr.first) >= 0)
                    uses.push_back(pr);
            }

            if (uses.empty()) continue;
            std::sort(uses.begin(), uses.end(), [](auto& a, auto& b) {
                return a.first < b.first;
            });

            i32 lastWriterRemappedIdx = -1;
            std::vector<i32> lastReadersRemappedIdx;

            for (auto& pr : uses) {
                i32 remapped = passRemap.at(pr.first);
                bool isWrite = (pr.second.type != AccessType::Read);

                if (isWrite) {
                    if (lastWriterRemappedIdx != -1 && lastWriterRemappedIdx != remapped)
                        edges.insert({lastWriterRemappedIdx, remapped});

                    for (i32 readerIdx : lastReadersRemappedIdx) {
                        if (readerIdx != remapped)
                            edges.insert({readerIdx, remapped});
                    }

                    lastReadersRemappedIdx.clear();
                    lastWriterRemappedIdx = remapped;
                } else {
                    if (lastWriterRemappedIdx != -1 && lastWriterRemappedIdx != remapped)
                        edges.insert({lastWriterRemappedIdx, remapped});

                    lastReadersRemappedIdx.push_back(remapped);
                }
            }
        }

        for (const auto& edge : edges) {
            adj.at(edge.first).push_back(edge.second);
            indeg[edge.second]++;
        }

        std::queue<i32> kahn;
        for (i32 i = 0; i < static_cast<i32>(N); ++i) {
            if (indeg.at(i) == 0)
                kahn.push(i);
        }

        std::vector<i32> topo;
        topo.reserve(N);
        while (!kahn.empty()) {
            i32 v = kahn.front();
            kahn.pop();
            topo.push_back(v);

            for (i32 nx : adj.at(v)) {
                indeg[nx]--;
                if (indeg.at(nx) == 0)
                    kahn.push(nx);
            }
        }

        if (topo.size() != N) {
            LOG_ERROR("[RenderGraph] Cycle detected in pass dependencies");
            return {};
        }

        std::vector<PassHandle> execOrder;
        execOrder.reserve(topo.size());
        for (i32 idx : topo)
            execOrder.push_back(alivePasses.at(idx));

        for (auto& res : m_Resources) {
            res.firstUse = -1;
            res.lastUse = -1;
        }

        for (i32 i = 0; i < static_cast<i32>(execOrder.size()); ++i) {
            PassHandle p = execOrder[i];

            for (const auto& ai : m_Passes.at(p).accesses) {
                auto r = ai.resource;

                if (m_Resources[r].firstUse == -1)
                    m_Resources[r].firstUse = i;

                m_Resources[r].lastUse = i;
            }
        }

        i32 totalAllocs = 0;
        std::vector<i32> allocId(m_Resources.size(), -1);

        struct Interval
        {
            ResourceHandle resource;
            i32 start;
            i32 end;
            bool canAlias;
        };

        std::vector<Interval> intervals;
        for (ResourceHandle r = 0; r < m_Resources.size(); ++r) {
            if (m_Resources[r].firstUse != -1) {
                intervals.push_back({
                    r,
                    m_Resources[r].firstUse,
                    m_Resources[r].lastUse,
                    !m_Resources[r].imported && m_Resources[r].imageDesc.transient
                });
            }
        }

        std::sort(intervals.begin(), intervals.end(), [](const Interval& a, const Interval& b) {
            return a.start < b.start;
        });

        std::vector<i32> aliasedAllocEndTimes;
        i32 nextNonAliasedId = 0;

        for (const auto& it : intervals) {
            if (!it.canAlias) {
                allocId[it.resource] = nextNonAliasedId;
            } else {
                i32 foundId = -1;
                for (usize i = 0; i < aliasedAllocEndTimes.size(); ++i) {
                    if (aliasedAllocEndTimes[i] < it.start) {
                        foundId = static_cast<i32>(i);
                        break;
                    }
                }

                if (foundId != -1) {
                    allocId[it.resource] = foundId;
                    aliasedAllocEndTimes[foundId] = it.end;
                } else {
                    i32 newId = static_cast<i32>(aliasedAllocEndTimes.size());
                    allocId[it.resource] = newId;
                    aliasedAllocEndTimes.push_back(it.end);
                }
            }
        }

        i32 aliasedPoolSize = static_cast<i32>(aliasedAllocEndTimes.size());
        for (const auto& it : intervals) {
            if (!it.canAlias)
                allocId[it.resource] += aliasedPoolSize;
        }

        totalAllocs = aliasedPoolSize + nextNonAliasedId;
        (void)totalAllocs;

        std::vector<Barrier> barriers;
        for (ResourceHandle r = 0; r < m_Resources.size(); ++r) {
            std::vector<std::pair<i32, AccessInfo>> uses;

            for (i32 pos = 0; pos < static_cast<i32>(execOrder.size()); ++pos) {
                for (const auto& ai : m_Passes[execOrder[pos]].accesses) {
                    if (ai.resource == r)
                        uses.push_back({pos, ai});
                }
            }

            if (uses.empty()) continue;

            {
                auto& u = uses.front();
                barriers.push_back(Barrier {
                    .srcPass = std::numeric_limits<u32>::max(),
                    .dstPass = static_cast<PassHandle>(u.first),
                    .resource = r,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = u.second.layout,
                    .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    .dstStageMask = u.second.stage,
                    .srcAccessMask = 0,
                    .dstAccessMask = u.second.accessMask
                });
            }

            for (usize i = 0; i + 1 < uses.size(); ++i) {
                auto& cur = uses[i];
                auto& nxt = uses[i+1];

                if (cur.first == nxt.first) continue;

                bool curIsWrite = cur.second.type != AccessType::Read;
                bool nxtIsWrite = nxt.second.type != AccessType::Read;

                if (!curIsWrite && !nxtIsWrite && cur.second.layout == nxt.second.layout)
                    continue;

                barriers.push_back(Barrier {
                    .srcPass = static_cast<PassHandle>(cur.first),
                    .dstPass = static_cast<PassHandle>(nxt.first),
                    .resource = r,
                    .oldLayout = cur.second.layout,
                    .newLayout = nxt.second.layout,
                    .srcStageMask = cur.second.stage,
                    .dstStageMask = nxt.second.stage,
                    .srcAccessMask = cur.second.accessMask,
                    .dstAccessMask = nxt.second.accessMask
                });
            }
        }

        ExecutionPlan plan;
        plan.resources = m_Resources;
        plan.allocationIdPerResource = allocId;

        for (i32 i = 0; i < static_cast<i32>(execOrder.size()); ++i) {
            plan.orderedPasses.push_back({
                .pass = execOrder[i],
                .name = m_Passes[execOrder[i]].name
            });
        }

        for (auto& b : barriers) {
            if (b.srcPass != std::numeric_limits<u32>::max()) b.srcPass = execOrder[b.srcPass];
            if (b.dstPass != std::numeric_limits<u32>::max()) b.dstPass = execOrder[b.dstPass];
            plan.barriers.push_back(b);
        }

        return plan;
    }

}
