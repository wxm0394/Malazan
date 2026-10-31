#pragma once
#include "Core/Types.hpp"
#include "World/DarujhistanMap.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace Malazan {

struct PathNode {
    Vec2i pos{0, 0};
    ElevationLevel elev{ElevationLevel::Street};
    Vec2i coord{0, 0};
    ElevationLevel elevation{ElevationLevel::Street};

    PathNode() = default;
    PathNode(Vec2i p, ElevationLevel e)
        : pos(p), elev(e), coord(p), elevation(e) {}

    bool operator==(const PathNode& o) const {
        return pos == o.pos && elev == o.elev;
    }
};

struct PathNodeHash {
    std::size_t operator()(const PathNode& n) const {
        return (static_cast<size_t>(n.pos.x) * 73856093) ^
               (static_cast<size_t>(n.pos.y) * 19349663) ^
               (static_cast<size_t>(n.elev) * 83492791);
    }
};

class PathfindingSystem {
public:
    static std::vector<PathNode> find_path_3d(
        const DarujhistanMap& map,
        Vec2i start_pos,
        ElevationLevel start_elev,
        Vec2i goal_pos,
        ElevationLevel goal_elev,
        bool can_climb_roofs = true
    ) {
        return find_path(PathNode{start_pos, start_elev}, PathNode{goal_pos, goal_elev}, map, can_climb_roofs);
    }

    static std::vector<PathNode> find_path(
        PathNode start,
        PathNode goal,
        const DarujhistanMap& map,
        bool can_climb_roofs = true
    ) {
        if (start == goal) return {start};
        if (!map.in_bounds(start.pos) || !map.in_bounds(goal.pos)) return {};

        struct PriorityItem {
            PathNode node;
            float f_score;
            bool operator>(const PriorityItem& o) const { return f_score > o.f_score; }
        };

        std::priority_queue<PriorityItem, std::vector<PriorityItem>, std::greater<PriorityItem>> open_set;
        std::unordered_map<PathNode, PathNode, PathNodeHash> came_from;
        std::unordered_map<PathNode, float, PathNodeHash> g_score;

        g_score[start] = 0.0f;
        open_set.push({start, heuristic(start, goal)});

        constexpr int dx[] = {0, 0, -1, 1};
        constexpr int dy[] = {-1, 1, 0, 0};

        while (!open_set.empty()) {
            PathNode current = open_set.top().node;
            open_set.pop();

            if (current == goal) {
                // 回溯重构最优路径
                std::vector<PathNode> total_path;
                PathNode curr_trace = current;
                while (!(curr_trace == start)) {
                    total_path.push_back(curr_trace);
                    curr_trace = came_from[curr_trace];
                }
                total_path.push_back(start);
                std::reverse(total_path.begin(), total_path.end());
                return total_path;
            }

            float current_g = g_score[current];

            // 1. 同层水平四向移动
            for (int i = 0; i < 4; ++i) {
                Vec2i next_pos{current.pos.x + dx[i], current.pos.y + dy[i]};
                if (!map.in_bounds(next_pos)) continue;

                PathNode next_node{next_pos, current.elev};
                float step_cost = 1.0f;
                bool passable = false;

                if (current.elev == ElevationLevel::Street) {
                    const auto& cell = map.get_cell(next_pos, ElevationLevel::Street);
                    if (!cell.blocks_movement && cell.terrain != TileTerrain::BuildingWall) {
                        passable = true;
                    }
                } else if (current.elev == ElevationLevel::Underground) {
                    const auto& cell = map.get_cell(next_pos, ElevationLevel::Underground);
                    if (!cell.blocks_movement && cell.terrain != TileTerrain::CatacombWall) {
                        passable = true;
                    }
                } else if (current.elev == ElevationLevel::Rooftop) {
                    const auto& roof_cell = map.get_cell(next_pos, ElevationLevel::Rooftop);
                    const auto& street_cell = map.get_cell(next_pos, ElevationLevel::Street);

                    // 正常的屋顶
                    if (street_cell.terrain == TileTerrain::BuildingWall && !roof_cell.roof_collapsed) {
                        passable = true;
                    } else if (can_climb_roofs) {
                        // 刺客屋顶飞跃：两座建筑间若隔着 1 格街面小巷，允许直接跨越
                        Vec2i leap_pos{next_pos.x + dx[i], next_pos.y + dy[i]};
                        if (map.in_bounds(leap_pos)) {
                            const auto& far_street = map.get_cell(leap_pos, ElevationLevel::Street);
                            const auto& far_roof = map.get_cell(leap_pos, ElevationLevel::Rooftop);
                            if (far_street.terrain == TileTerrain::BuildingWall && !far_roof.roof_collapsed) {
                                next_node = PathNode{leap_pos, ElevationLevel::Rooftop};
                                step_cost = 2.0f; // 飞跃消耗稍多
                                passable = true;
                            }
                        }
                    }
                }

                if (passable) {
                    float tentative_g = current_g + step_cost;
                    if (!g_score.contains(next_node) || tentative_g < g_score[next_node]) {
                        came_from[next_node] = current;
                        g_score[next_node] = tentative_g;
                        open_set.push({next_node, tentative_g + heuristic(next_node, goal)});
                    }
                }
            }

            // 2. 空间高程垂直跃迁 (Vertical Transitions)
            // A. 地下墓道石阶铁栅门扉与街面互通 (Sewer Grate / Catacomb Stairs)
            if (current.elev == ElevationLevel::Street) {
                const auto& cur_st = map.get_cell(current.pos, ElevationLevel::Street);
                if (cur_st.terrain == TileTerrain::SewerGrateStairs) {
                    PathNode down_node{current.pos, ElevationLevel::Underground};
                    float tentative_g = current_g + 1.2f;
                    if (!g_score.contains(down_node) || tentative_g < g_score[down_node]) {
                        came_from[down_node] = current;
                        g_score[down_node] = tentative_g;
                        open_set.push({down_node, tentative_g + heuristic(down_node, goal)});
                    }
                }
            } else if (current.elev == ElevationLevel::Underground) {
                const auto& cur_ug = map.get_cell(current.pos, ElevationLevel::Underground);
                if (cur_ug.terrain == TileTerrain::SewerGrateStairs) {
                    PathNode up_node{current.pos, ElevationLevel::Street};
                    float tentative_g = current_g + 1.2f;
                    if (!g_score.contains(up_node) || tentative_g < g_score[up_node]) {
                        came_from[up_node] = current;
                        g_score[up_node] = tentative_g;
                        open_set.push({up_node, tentative_g + heuristic(up_node, goal)});
                    }
                }
            }

            // B. 屋顶攀爬与坠落 (Rooftop climb / plunge)
            if (can_climb_roofs) {
                if (current.elev == ElevationLevel::Street) {
                    // 若当前街面位置紧邻建筑外墙，且该建筑上方是屋顶，可攀爬上房
                    for (int i = 0; i < 4; ++i) {
                        Vec2i adj{current.pos.x + dx[i], current.pos.y + dy[i]};
                        if (!map.in_bounds(adj)) continue;
                        const auto& st = map.get_cell(adj, ElevationLevel::Street);
                        const auto& rf = map.get_cell(adj, ElevationLevel::Rooftop);
                        if (st.terrain == TileTerrain::BuildingWall && !rf.roof_collapsed) {
                            PathNode up_node{adj, ElevationLevel::Rooftop};
                            float tentative_g = current_g + 2.5f; // 攀爬动作耗时
                            if (!g_score.contains(up_node) || tentative_g < g_score[up_node]) {
                                came_from[up_node] = current;
                                g_score[up_node] = tentative_g;
                                open_set.push({up_node, tentative_g + heuristic(up_node, goal)});
                            }
                        }
                    }
                } else if (current.elev == ElevationLevel::Rooftop) {
                    // 从屋顶跳跃/绳降回街面
                    for (int i = 0; i < 4; ++i) {
                        Vec2i drop_pos{current.pos.x + dx[i], current.pos.y + dy[i]};
                        if (!map.in_bounds(drop_pos)) continue;
                        const auto& st = map.get_cell(drop_pos, ElevationLevel::Street);
                        if (st.terrain != TileTerrain::BuildingWall && !st.blocks_movement) {
                            PathNode down_node{drop_pos, ElevationLevel::Street};
                            float tentative_g = current_g + 1.5f;
                            if (!g_score.contains(down_node) || tentative_g < g_score[down_node]) {
                                came_from[down_node] = current;
                                g_score[down_node] = tentative_g;
                                open_set.push({down_node, tentative_g + heuristic(down_node, goal)});
                            }
                        }
                    }
                }
            }
        }

        return {}; // 路径不可达
    }

private:
    static float heuristic(const PathNode& a, const PathNode& b) {
        float manhattan = static_cast<float>(std::abs(a.pos.x - b.pos.x) + std::abs(a.pos.y - b.pos.y));
        float vertical_penalty = (a.elev != b.elev) ? 3.0f : 0.0f;
        return manhattan + vertical_penalty;
    }
};

} // namespace Malazan
