#pragma once
#include "Core/Types.hpp"
#include <vector>
#include <string>

namespace Malazan {

enum class TileTerrain : uint8_t {
    OpenPaved,        // 平整石板路
    NarrowAlley,      // 狭窄阴暗小巷
    BuildingWall,     // 坚实建筑外墙（阻挡街面，支撑屋顶）
    SlateRooftop,     // 斜瓦屋顶（可行走，需敏捷判定）
    FlatTerrace,      // 房顶平整露台（稳定战场）
    EmptyAir,         // 空中落差（屋顶层无建筑的街面上空）
    UndergroundPipe,  // 地下高压蓝火主干管
    CatacombTunnel,   // 地下墓穴甬道（可行走）
    CatacombWall,     // 坚硬地下基岩
    AzathRootNode,    // 阿扎斯古木灵根节点
    SewerGrateStairs, // 街面与地下墓穴连接铁栅石阶
    GasMainPipe = UndergroundPipe // 兼容别名
};

struct Cell {
    TileTerrain terrain{TileTerrain::OpenPaved};
    bool blocks_sight{false};
    bool blocks_movement{false};
    bool has_gas_lamp{false};      // 是否立有蓝火煤气路灯
    bool roof_collapsed{false};    // 屋顶是否已被炸塌崩落
    bool is_gas_leaking{false};    // 管道或区域是否处于瓦斯泄漏状态
};

class DarujhistanMap {
public:
    DarujhistanMap(int32_t width, int32_t height)
        : m_width(width), m_height(height) {
        m_underground_layer.resize(width * height, Cell{.terrain = TileTerrain::CatacombWall, .blocks_sight = true, .blocks_movement = true});
        m_street_layer.resize(width * height, Cell{.terrain = TileTerrain::OpenPaved});
        m_rooftop_layer.resize(width * height, Cell{.terrain = TileTerrain::EmptyAir, .blocks_movement = true});

        // 默认开辟主城区地下墓穴与煤气主干管网 (Catacombs & Main Gas Line)
        carve_default_catacombs();
    }

    [[nodiscard]] int32_t width() const { return m_width; }
    [[nodiscard]] int32_t height() const { return m_height; }

    [[nodiscard]] bool in_bounds(Vec2i pos) const {
        return pos.x >= 0 && pos.x < m_width && pos.y >= 0 && pos.y < m_height;
    }

    [[nodiscard]] Cell& get_cell(Vec2i pos, ElevationLevel elev) {
        size_t idx = static_cast<size_t>(pos.y * m_width + pos.x);
        switch (elev) {
            case ElevationLevel::Underground: return m_underground_layer[idx];
            case ElevationLevel::Rooftop:     return m_rooftop_layer[idx];
            case ElevationLevel::Street:
            default:                          return m_street_layer[idx];
        }
    }

    [[nodiscard]] const Cell& get_cell(Vec2i pos, ElevationLevel elev) const {
        size_t idx = static_cast<size_t>(pos.y * m_width + pos.x);
        switch (elev) {
            case ElevationLevel::Underground: return m_underground_layer[idx];
            case ElevationLevel::Rooftop:     return m_rooftop_layer[idx];
            case ElevationLevel::Street:
            default:                          return m_street_layer[idx];
        }
    }

    // 建造一座建筑：在街面设置外墙，在屋顶层生成可攀爬瓦面
    void add_building(Vec2i top_left, int32_t w, int32_t h, TileTerrain roof_type = TileTerrain::SlateRooftop) {
        for (int32_t y = top_left.y; y < top_left.y + h; ++y) {
            for (int32_t x = top_left.x; x < top_left.x + w; ++x) {
                Vec2i p{x, y};
                if (!in_bounds(p)) continue;

                // 街面阻挡
                auto& street_cell = get_cell(p, ElevationLevel::Street);
                street_cell.terrain = TileTerrain::BuildingWall;
                street_cell.blocks_movement = true;
                street_cell.blocks_sight = true;

                // 屋顶可踏足
                auto& roof_cell = get_cell(p, ElevationLevel::Rooftop);
                roof_cell.terrain = roof_type;
                roof_cell.blocks_movement = false;
                roof_cell.blocks_sight = false;
            }
        }
    }

    // 轰塌屋顶：屋顶变为虚空坠落区，街面受瓦砾掩埋
    void collapse_rooftop(Vec2i pos) {
        if (!in_bounds(pos)) return;
        auto& roof = get_cell(pos, ElevationLevel::Rooftop);
        roof.roof_collapsed = true;
        roof.terrain = TileTerrain::EmptyAir;
        roof.blocks_movement = true;

        auto& street = get_cell(pos, ElevationLevel::Street);
        street.terrain = TileTerrain::NarrowAlley; // 瓦砾堆，减速
    }

    // 挖掘地下墓道
    void carve_tunnel(Vec2i from, Vec2i to) {
        int32_t x0 = std::min(from.x, to.x);
        int32_t x1 = std::max(from.x, to.x);
        int32_t y0 = std::min(from.y, to.y);
        int32_t y1 = std::max(from.y, to.y);

        for (int32_t x = x0; x <= x1; ++x) {
            Vec2i p{x, from.y};
            if (in_bounds(p)) {
                auto& c = get_cell(p, ElevationLevel::Underground);
                c.terrain = TileTerrain::CatacombTunnel;
                c.blocks_movement = false;
                c.blocks_sight = false;
            }
        }
        for (int32_t y = y0; y <= y1; ++y) {
            Vec2i p{to.x, y};
            if (in_bounds(p)) {
                auto& c = get_cell(p, ElevationLevel::Underground);
                c.terrain = TileTerrain::CatacombTunnel;
                c.blocks_movement = false;
                c.blocks_sight = false;
            }
        }
    }

    // 序列化与反序列化访问接口
    [[nodiscard]] const std::vector<Cell>& get_underground_layer() const { return m_underground_layer; }
    [[nodiscard]] const std::vector<Cell>& get_street_layer() const { return m_street_layer; }
    [[nodiscard]] const std::vector<Cell>& get_rooftop_layer() const { return m_rooftop_layer; }
    std::vector<Cell>& get_underground_layer() { return m_underground_layer; }
    std::vector<Cell>& get_street_layer() { return m_street_layer; }
    std::vector<Cell>& get_rooftop_layer() { return m_rooftop_layer; }

private:
    void carve_default_catacombs() {
        // 主干墓道贯通
        carve_tunnel({2, 4}, {10, 4});
        carve_tunnel({2, 4}, {2, 10});
        carve_tunnel({4, 6}, {16, 6});
        carve_tunnel({10, 4}, {10, 14});
        carve_tunnel({2, 10}, {16, 10});

        // 设置街面与地下贯通的石阶铁栅 (位于未被建筑遮挡的开放街道)
        Vec2i grate1{2, 4};   // 西区城防守卫街道下水道口
        Vec2i grate2{10, 14}; // 南区大道下水道口
        Vec2i grate3{10, 6};  // 中央街区下水道口
        for (const auto& g : {grate1, grate2, grate3}) {
            auto& sg = get_cell(g, ElevationLevel::Street);
            sg.terrain = TileTerrain::SewerGrateStairs;
            sg.blocks_movement = false;
            sg.blocks_sight = false;

            auto& ug = get_cell(g, ElevationLevel::Underground);
            ug.terrain = TileTerrain::SewerGrateStairs;
            ug.blocks_movement = false;
            ug.blocks_sight = false;
        }

        // 地下高压蓝火主干管道走线
        for (int32_t x = 4; x <= 16; ++x) {
            Vec2i pipe_p{x, 7};
            if (in_bounds(pipe_p)) {
                auto& c = get_cell(pipe_p, ElevationLevel::Underground);
                c.terrain = TileTerrain::UndergroundPipe;
                c.blocks_movement = false;
            }
        }

        // 阿扎斯灵根沉眠节点
        Vec2i azath_root{10, 10};
        if (in_bounds(azath_root)) {
            auto& c = get_cell(azath_root, ElevationLevel::Underground);
            c.terrain = TileTerrain::AzathRootNode;
            c.blocks_movement = false;
        }
    }

    int32_t m_width;
    int32_t m_height;
    std::vector<Cell> m_underground_layer;
    std::vector<Cell> m_street_layer;
    std::vector<Cell> m_rooftop_layer;
};

} // namespace Malazan
