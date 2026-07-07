#pragma once

#include <glad/glad.h>

#include <array>
#include <string>

#include "Asset.h"

namespace se::assets {

class Cubemap : public Asset {
public:
    explicit Cubemap(const std::array<std::string, 6>& facePaths);
    ~Cubemap() override;

    Cubemap(const Cubemap&) = delete;
    Cubemap& operator=(const Cubemap&) = delete;
    Cubemap(Cubemap&&) = delete;
    Cubemap& operator=(Cubemap&&) = delete;

    void bind(unsigned int slot = 0) const;

    [[nodiscard]] unsigned int id() const { return m_Id; }
    [[nodiscard]] int width() const { return m_Width; }
    [[nodiscard]] int height() const { return m_Height; }

private:
    unsigned int m_Id = 0;
    int m_Width = 0;
    int m_Height = 0;
};

}  // namespace se::assets
