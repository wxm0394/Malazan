#pragma once
#include "Core/Types.hpp"
#include "World/DarujhistanMap.hpp"
#include <string>
#include <unordered_map>
#include <memory>

namespace Malazan {

enum class SectorId : uint8_t {
    PhoenixInnQuarter,      // 凤凰客栈城区 (青石瓦顶、繁华集市、公会暗巷)
    DarujhistanCatacombs,   // 远古地底墓穴与煤气主干管网 (阴暗甬道、易爆气云、阿扎斯根须)
    GadrobiHillsBarrow      // 加德罗比丘陵上古墓冢 (暴君雷斯特破封荒原、石阵巨石)
};

struct SectorPortal {
    Vec2i trigger_pos;
    ElevationLevel trigger_elev{ElevationLevel::Street};
    SectorId dest_sector{SectorId::PhoenixInnQuarter};
    Vec2i dest_pos{0, 0};
    ElevationLevel dest_elev{ElevationLevel::Street};
    std::string portal_name;
};

class WorldSector {
public:
    WorldSector(SectorId id, std::string name, std::string description, int32_t width = 20, int32_t height = 20)
        : m_id(id), m_name(std::move(name)), m_description(std::move(description)), m_map(width, height) {}

    [[nodiscard]] SectorId id() const { return m_id; }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] const std::string& description() const { return m_description; }
    [[nodiscard]] DarujhistanMap& map() { return m_map; }
    [[nodiscard]] const DarujhistanMap& map() const { return m_map; }

    void add_portal(SectorPortal portal) {
        m_portals.push_back(std::move(portal));
    }

    [[nodiscard]] const std::vector<SectorPortal>& get_portals() const {
        return m_portals;
    }

private:
    SectorId m_id{SectorId::PhoenixInnQuarter};
    std::string m_name;
    std::string m_description;
    DarujhistanMap m_map;
    std::vector<SectorPortal> m_portals;
};

} // namespace Malazan
