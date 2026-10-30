#pragma once
#include "Core/Types.hpp"
#include <vector>
#include <string>

namespace Malazan {

enum class TileTerrain : uint8_t {
    OpenPaved,      // 平整石板路
    NarrowAlley,    // 狭窄阴暗小巷
    BuildingWall,   // 坚实建筑外墙（阻挡街面，支撑屋顶）
    SlateRooftop,   // 斜瓦屋顶（可行走，需敏捷判定）
    FlatTerrace,    // 房顶平整露台（稳定战场）
    EmptyAir,       // 空中落差（屋顶层无建筑的街面上空）
    UndergroundPipe // 地下主管道
};

struct Cell {
    TileTerrain terrain{TileTerrain::OpenPaved};
    bool blocks_sight{false};
    bool blocks_movement{false};
    bool has_gas_lamp{false};      // 是否立有蓝火煤气路灯
    bool roof_collapsed{false};    // 屋顶是否已被炸塌崩落
};

class DarujhistanMap {
public:
    DarujhistanMap(int32_t width, int32_t height)
        : m_width(width), m_height(height) {
        m_street_layer.resize(width * height, Cell{.terrain = TileTerrain::OpenPaved});
        m_rooftop_layer.resize(width * height, Cell{.terrain = TileTerrain::EmptyAir, .blocks_movement = true});
    }

    [[nodiscard]] int32_t width() const { return m_width; }
    [[nodiscard]] int32_t height() const { return m_height; }

    [[nodiscard]] bool in_bounds(Vec2i pos) const {
        return pos.x >= 0 && pos.x < m_width && pos.y >= 0 && pos.y < m_height;
    }

    [[nodiscard]] Cell& get_cell(Vec2i pos, ElevationLevel elev) {
        size_t idx = static_cast<size_t>(pos.y * m_width + pos.x);
        return elev == ElevationLevel::Rooftop ? m_rooftop_layer[idx] : m_street_layer[idx];
    }

    [[nodiscard]] const Cell& get_cell(Vec2i pos, ElevationLevel elev) const {
        size_t idx = static_cast<size_t>(pos.y * m_width + pos.x);
        return elev == ElevationLevel::Rooftop ? m_rooftop_layer[idx] : m_street_layer[idx];
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

private:
    int32_t m_width;
    int32_t m_height;
    std::vector<Cell> m_street_layer;
    std::vector<Cell> m_rooftop_layer;
};

} // namespace Malazan
