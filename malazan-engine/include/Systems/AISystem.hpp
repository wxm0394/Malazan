#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "Components/AIComponent.hpp"
#include "Components/OtataralComponent.hpp"
#include "Components/InventoryComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include "Systems/DeckOfDragonsSystem.hpp"
#include "Systems/PathfindingSystem.hpp"
#include "Systems/OtataralSystem.hpp"
#include <iostream>
#include <cmath>
#include <random>

namespace Malazan {

class AISystem {
public:
    // 执行所有 AI 实体的自主战术决策回合（基于 3D A* 空间寻路与感知状态机）
    static void update_ai_turns(
        Registry& reg,
        DarujhistanMap& map,
        Entity player_ent,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<PositionComponent>(player_ent) || !reg.has<ActorComponent>(player_ent)) return;

        auto& player_pos = reg.get<PositionComponent>(player_ent);
        auto& player_actor = reg.get<ActorComponent>(player_ent);
        bool player_stealthed = false;
        float player_stealth_val = 0.0f;

        if (reg.has<StealthComponent>(player_ent)) {
            auto& st = reg.get<StealthComponent>(player_ent);
            player_stealthed = st.in_stealth;
            player_stealth_val = st.stealth_power * DeckOfDragonsSystem::active_card.stealth_mult;
        }

        for (auto [ai_ent, ai_comp, actor, pos, energy] : 
             reg.view<AIComponent, ActorComponent, PositionComponent, TurnEnergyComponent>()) {
            
            if (!actor.is_alive) continue;
            if (actor.faction == player_actor.faction) continue; // 同阵营不攻击

            // 1. 感知与警觉检定 (Perception Check)
            int32_t dist_to_player = pos.coord.manhattan_dist(player_pos.coord);

            // 视线检定：同一层级且在视距内，或高处俯视
            bool can_see_player = false;
            if (dist_to_player <= ai_comp.sight_range) {
                if (pos.elevation == player_pos.elevation || pos.elevation == ElevationLevel::Rooftop) {
                    if (!player_stealthed || player_stealth_val < 50.0f) {
                        can_see_player = true;
                    }
                }
            }

            // 状态机流转
            if (can_see_player) {
                ai_comp.state = AIBehaviorState::Combat;
                ai_comp.target_interest_pos = player_pos.coord;
                ai_comp.alert_meter = 100.0f;
            } else if (dist_to_player <= ai_comp.hearing_range && ai_comp.state == AIBehaviorState::Patrol) {
                // 听到刺杀动静或爆炸声，进入怀疑调查状态
                ai_comp.state = AIBehaviorState::Suspicious;
                ai_comp.target_interest_pos = player_pos.coord;
            }

            // 2. 行为执行 (结合 3D A* 避障与立体穿插)
            if (ai_comp.state == AIBehaviorState::Combat) {
                // 若已处于攻击射程内
                if (dist_to_player <= ai_comp.attack_range) {
                    execute_ai_attack(ai_ent, player_ent, reg, out_logs);
                    energy.consume(1000);
                    continue;
                }

                // 否则调用 3D A* 寻路避开建筑物死角与障碍，全速逼近
                PathNode start{pos.coord, pos.elevation};
                PathNode goal{player_pos.coord, player_pos.elevation};
                auto path = PathfindingSystem::find_path(start, goal, map, pos.can_climb_roofs);

                if (path.size() >= 2) {
                    pos.coord = path[1].pos;
                    pos.elevation = path[1].elev;
                    out_logs.push_back("[" + actor.name + "] 施展暗影步沿巷道绕行逼近 (" 
                                       + std::to_string(pos.coord.x) + "," + std::to_string(pos.coord.y) + ")");
                }
                energy.consume(1000);

            } else if (ai_comp.state == AIBehaviorState::Suspicious) {
                if (pos.coord == ai_comp.target_interest_pos) {
                    // 到达怀疑点无果，恢复巡逻
                    ai_comp.state = AIBehaviorState::Patrol;
                    ai_comp.alert_meter = 0.0f;
                } else {
                    PathNode start{pos.coord, pos.elevation};
                    PathNode goal{ai_comp.target_interest_pos, pos.elevation};
                    auto path = PathfindingSystem::find_path(start, goal, map, pos.can_climb_roofs);
                    if (path.size() >= 2) {
                        pos.coord = path[1].pos;
                        pos.elevation = path[1].elev;
                    }
                    energy.consume(1000);
                }
            } else if (ai_comp.state == AIBehaviorState::Patrol) {
                // 沿巡逻路标移动
                if (!ai_comp.patrol_waypoints.empty()) {
                    Vec2i target_wp = ai_comp.patrol_waypoints[ai_comp.current_waypoint_idx];
                    if (pos.coord == target_wp) {
                        ai_comp.current_waypoint_idx = (ai_comp.current_waypoint_idx + 1) % ai_comp.patrol_waypoints.size();
                    } else {
                        PathNode start{pos.coord, pos.elevation};
                        PathNode goal{target_wp, pos.elevation};
                        auto path = PathfindingSystem::find_path(start, goal, map, pos.can_climb_roofs);
                        if (path.size() >= 2) {
                            pos.coord = path[1].pos;
                            pos.elevation = path[1].elev;
                        }
                    }
                }
                energy.consume(1000);
            }
        }
    }

private:
    static void execute_ai_attack(
        Entity attacker,
        Entity target,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        auto& atk_actor = reg.get<ActorComponent>(attacker);
        auto& atk_comp = reg.get<AIComponent>(attacker);
        auto& vic_actor = reg.get<ActorComponent>(target);

        int32_t dmg = atk_comp.attack_power;

        // 奥塔拉尔力场对暗影法术附魔攻击的压制检定
        float intensity = 0.0f;
        auto& target_pos = reg.get<PositionComponent>(target);
        if (OtataralSystem::is_suppressed_at(target_pos.coord, target_pos.elevation, reg, &intensity)) {
            dmg = static_cast<int32_t>(dmg * (1.0f - 0.4f * intensity));
            out_logs.push_back("【奥塔拉尔压制!】目标的生锈死域弱化了 [" + atk_actor.name + "] 的暗影淬毒效果!");
        }

        // 奥彭命运判定 (Oponn Gamble)
        if (DeckOfDragonsSystem::active_card.oponn_gamble_chance > 0.0f) {
            static std::mt19937 rng(999);
            std::uniform_real_distribution<float> d(0.0f, 1.0f);
            if (d(rng) < DeckOfDragonsSystem::active_card.oponn_gamble_chance) {
                dmg *= 2;
                out_logs.push_back("【奥彭硬币反转!】爪牙攻势受命运眷顾，暴击翻倍!");
            }
        }

        vic_actor.take_damage(dmg, DamageType::PhysicalPierce);
        out_logs.push_back(">>> [" + atk_actor.name + "] 投射淬毒暗影刃，刺伤 [" 
                           + vic_actor.name + "]！造成 " + std::to_string(dmg) + " 点伤害！");
    }
};

} // namespace Malazan
