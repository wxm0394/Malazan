#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "Components/AIComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include "Systems/DeckOfDragonsSystem.hpp"
#include <iostream>
#include <cmath>

namespace Malazan {

class AISystem {
public:
    // 执行所有 AI 实体的自主战术决策回合
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
                // 听到动静，进入怀疑调查状态
                ai_comp.state = AIBehaviorState::Suspicious;
                ai_comp.target_interest_pos = player_pos.coord;
            }

            // 2. 行为执行
            if (ai_comp.state == AIBehaviorState::Combat) {
                // 若已处于攻击射程内
                if (dist_to_player <= ai_comp.attack_range) {
                    execute_ai_attack(ai_ent, player_ent, reg, out_logs);
                    energy.consume(1000);
                    continue;
                }

                // 否则全速向目标寻路靠近
                step_towards(pos, ai_comp.target_interest_pos, map, pos.elevation);
                out_logs.push_back("[" + actor.name + "] 握紧浸毒刃，向目标逼近!");
                energy.consume(1000);

            } else if (ai_comp.state == AIBehaviorState::Suspicious) {
                if (pos.coord == ai_comp.target_interest_pos) {
                    // 到达怀疑点无果，恢复巡逻
                    ai_comp.state = AIBehaviorState::Patrol;
                    ai_comp.alert_meter = 0.0f;
                } else {
                    step_towards(pos, ai_comp.target_interest_pos, map, pos.elevation);
                    energy.consume(1000);
                }
            } else if (ai_comp.state == AIBehaviorState::Patrol) {
                // 沿巡逻路标移动
                if (!ai_comp.patrol_waypoints.empty()) {
                    Vec2i target_wp = ai_comp.patrol_waypoints[ai_comp.current_waypoint_idx];
                    if (pos.coord == target_wp) {
                        ai_comp.current_waypoint_idx = (ai_comp.current_waypoint_idx + 1) % ai_comp.patrol_waypoints.size();
                    } else {
                        step_towards(pos, target_wp, map, pos.elevation);
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
                           + vic_actor.name + "]！造成 " + std::to_string(dmg) + " 点破甲伤害！");
    }

    static void step_towards(PositionComponent& pos, Vec2i target, DarujhistanMap& map, ElevationLevel elev) {
        int32_t dx = target.x - pos.coord.x;
        int32_t dy = target.y - pos.coord.y;

        Vec2i next_step = pos.coord;

        // 优先在距离更大的轴上走一格
        if (std::abs(dx) > std::abs(dy)) {
            next_step.x += (dx > 0 ? 1 : -1);
        } else if (dy != 0) {
            next_step.y += (dy > 0 ? 1 : -1);
        } else if (dx != 0) {
            next_step.x += (dx > 0 ? 1 : -1);
        }

        if (map.in_bounds(next_step)) {
            const auto& cell = map.get_cell(next_step, elev);
            if (!cell.blocks_movement) {
                pos.coord = next_step;
            }
        }
    }
};

} // namespace Malazan
