#pragma once

#include <cstdint>

// Central registry of UBO binding points.
// Must stay in sync with `layout(std140, binding = N)` in GLSL shaders.
namespace se::render {

enum class UboBinding : uint8_t {
    Frame = 0,
    Bones = 1,
};

}  // namespace se::render
