#pragma once
#include <random>

class UUID {
   public:
    UUID() : m_uuid(dis(gen)) {}
    UUID(uint64_t uuid) : m_uuid(uuid) {}
    operator uint64_t() const { return m_uuid; }
    bool operator==(uint64_t value) const { return m_uuid == value; }
    bool operator!=(uint64_t value) const { return m_uuid != value; }

   private:
    static inline std::random_device rd{};
    static inline std::mt19937_64 gen{rd()};
    static inline std::uniform_int_distribution<uint64_t> dis;

    uint64_t m_uuid{};
};

namespace std {
template <>
struct hash<UUID> {
    std::size_t operator()(const UUID& id) const noexcept {
        return std::hash<uint64_t>{}(static_cast<uint64_t>(id));
    }
};
}