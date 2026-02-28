#pragma once
#include <cstdint>
#include <random>

class UUID {
   public:
    UUID() : m_uuid(dis(gen)) {}
    UUID(uint64_t uuid) : m_uuid(uuid) {}

    explicit operator uint64_t() const { return m_uuid; }

    bool operator==(const UUID& other) const { return m_uuid == other.m_uuid; }
    bool operator!=(const UUID& other) const { return m_uuid != other.m_uuid; }

   private:
    static inline std::random_device rd{};
    // Use thread_local to ensure different sequences in different threads.
    static inline thread_local std::mt19937_64 gen{rd()};
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