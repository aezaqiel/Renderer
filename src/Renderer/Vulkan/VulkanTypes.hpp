#pragma once

#include <volk.h>

#include "Core/Types.hpp"
#include "Core/Logger.hpp"

namespace Renderer {

    #define VK_CHECK(fn) do { VkResult result_ = fn; if (result_ != VK_SUCCESS) { LOG_ERROR("VK_CHECK Failed: {}", #fn); } } while (false)

}
