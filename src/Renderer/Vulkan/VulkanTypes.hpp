#pragma once

#include <volk.h>

#include "Core/Types.hpp"
#include "Core/Logger.hpp"

namespace Renderer {

#ifdef NDEBUG
    #define VK_CHECK(fn) (fn)
#else
    #include <cassert>
    #include "Core/Logger.hpp"

    #define VK_CHECK(fn) do { VkResult result_ = fn; if (result_ != VK_SUCCESS) { LOG_ERROR("VK_CHECK Failed ({}): {}", static_cast<i32>(result_), #fn); assert(result_ == VK_SUCCESS); } } while (false)
#endif

}
