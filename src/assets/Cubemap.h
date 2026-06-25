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

    [[nodiscard]] unsigned int id() const { return m_Id; }

private:
    unsigned int m_Id = 0;
};

}  // namespace se::assets
