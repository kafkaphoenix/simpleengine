#pragma once
#include <string>

#include "Asset.h"

class Texture : public Asset {
   public:
    Texture(const std::string& path, bool flipVertically = true);
    ~Texture();
    void bind(unsigned int slot = 0) const;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = delete;
    Texture& operator=(Texture&&) = delete;

    const std::string& getPath() const override { return m_Path; }

   private:
    unsigned int m_ID;
};
