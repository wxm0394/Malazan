#pragma once
#include "Core/Types.hpp"
#include "World/DarujhistanMap.hpp"
#include <vector>
#include <cmath>

namespace Malazan {

enum class VisibilityState : uint8_t {
    Unexplored, // 未探索（完全黑雾，不显示地形与实体）
    Explored,   // 记忆区（已知地形，呈暗淡灰度，不显示敌对活动实体）
    Visible     // 视线内（当前完全可见，全彩高光与动态光影）
};

class FOVSystem {
public:
    static void compute_fov(
        Vec2i origin,
        ElevationLevel elevation,
        int32_t sight_radius,
        const DarujhistanMap& map,
        std::vector<VisibilityState>& out_fog_grid
    ) {
        int32_t w = map.width();
        int32_t h = map.height();

        if (static_cast<int32_t>(out_fog_grid.size()) != w * h) {
            out_fog_grid.assign(w * h, VisibilityState::Unexplored);
        }

        // 1. 将上次的 Visible 降级为 Explored 记忆区
        for (auto& state : out_fog_grid) {
            if (state == VisibilityState::Visible) {
                state = VisibilityState::Explored;
            }
        }

        // 起始点始终可见
        if (map.in_bounds(origin)) {
            out_fog_grid[origin.y * w + origin.x] = VisibilityState::Visible;
        }

        // 高度优势：屋顶视野半径提升 +4 格
        int32_t actual_radius = sight_radius + (elevation == ElevationLevel::Rooftop ? 4 : 0);

        // 2. 360 度圆周光线投射 (Raycasting FOV)
        constexpr int NUM_RAYS = 180;
        for (int i = 0; i < NUM_RAYS; ++i) {
            float angle = (i * 2.0f * 3.14159265f) / NUM_RAYS;
            float cos_a = std::cos(angle);
            float sin_a = std::sin(angle);

            float cx = static_cast<float>(origin.x) + 0.5f;
            float cy = static_cast<float>(origin.y) + 0.5f;

            for (int r = 1; r <= actual_radius; ++r) {
                cx += cos_a;
                cy += sin_a;

                int32_t gx = static_cast<int32_t>(cx);
                int32_t gy = static_cast<int32_t>(cy);
                Vec2i cur{gx, gy};

                if (!map.in_bounds(cur)) break;

                size_t idx = static_cast<size_t>(gy * w + gx);
                out_fog_grid[idx] = VisibilityState::Visible;

                // 检查障碍遮挡
                const auto& cell = map.get_cell(cur, ElevationLevel::Street);
                if (cell.blocks_sight) {
                    // 屋顶若高于障碍物，允许视线继续穿透延伸 1 格
                    if (elevation != ElevationLevel::Rooftop) {
                        break;
                    }
                }
            }
        }
    }
};

} // namespace Malazan
