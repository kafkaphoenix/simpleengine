#pragma once

namespace se::render {

struct RenderStats {
    unsigned int drawCalls = 0;
    unsigned int triangles = 0;

    void reset() noexcept { drawCalls = triangles = 0; }

    RenderStats& operator+=(const RenderStats& other) noexcept {
        drawCalls += other.drawCalls;
        triangles += other.triangles;
        return *this;
    }
};

}  // namespace se::render