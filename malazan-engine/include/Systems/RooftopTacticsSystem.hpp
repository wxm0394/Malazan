#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "World/DarujhistanMap.hpp"
#include "Systems/OtataralSystem.hpp"
#include <iostream>
#include <vector>
#include <string>

namespace Malazan {

class RooftopTacticsSystem {
public:
    // 屋顶飞跃移动（刺客/盗贼特长）
    static bool attempt_rooftop_leap(
        Entity actor_ent,
        Registry& reg,
        DarujhistanMap& map,
        Vec2i destination,
        std::vector<std::string>* out_logs = nullptr
    ) {
        if (!reg.has<PositionComponent>(actor_ent) || !reg.has<ActorComponent>(actor_ent)) {
            return false;
        }

        auto& pos = reg.get<PositionComponent>(actor_ent);
        auto& actor = reg.get<ActorComponent>(actor_ent);

        auto log_msg = [&](const std::string& s) {
            std::cout << s << "\n";
            if (out_logs) out_logs->push_back(s);
        };

        if (!pos.can_climb_roofs) {
            log_msg("[战术移动] " + actor.name + " 未受过轻功训练，无法在屋顶斜瓦间跳跃!");
            return false;
        }

        int32_t distance = pos.coord.manhattan_dist(destination);
        if (distance > pos.rooftop_jump_range) {
            log_msg("[战术移动] 目标距离 (" + std::to_string(distance) + " 格) 超出 " 
                    + actor.name + " 的飞跃极限 (" + std::to_string(pos.rooftop_jump_range) + " 格)!");
            return false;
        }

        // 检查目标地点在屋顶层的可行走性
        const auto& target_roof = map.get_cell(destination, ElevationLevel::Rooftop);
        if (target_roof.terrain == TileTerrain::EmptyAir || target_roof.roof_collapsed) {
            log_msg("[战术移动] 目标点没有坚实屋檐支撑（虚空或已坍塌），强行跳跃将跌落街面!");
            // 跌落街面并承受坠落伤害
            pos.coord = destination;
            pos.elevation = ElevationLevel::Street;
            actor.take_damage(25, DamageType::PhysicalSlash);
            log_msg("    * " + actor.name + " 摔落在石板路上，受到 25 点坠落钝击伤害! (HP: " + std::to_string(actor.hp) + ")");
            return true;
        }

        // 成功飞跃
        log_msg("[屋顶飞跃] " + actor.name + " 灵巧地跃过小巷上空，从 (" 
                + std::to_string(pos.coord.x) + ", " + std::to_string(pos.coord.y) + ") 落在 (" 
                + std::to_string(destination.x) + ", " + std::to_string(destination.y) + ") 的青石屋顶上!");
        pos.coord = destination;
        pos.elevation = ElevationLevel::Rooftop;

        return true;
    }

    // 从屋顶向街面目标发动高空俯冲突袭 (Plunging Ambush)
    static bool execute_plunging_ambush(
        Entity attacker_ent,
        Entity victim_ent,
        Registry& reg,
        DarujhistanMap& map,
        std::vector<std::string>* out_logs = nullptr
    ) {
        (void)map;
        auto& atk_pos = reg.get<PositionComponent>(attacker_ent);
        auto& atk_actor = reg.get<ActorComponent>(attacker_ent);
        auto& vic_pos = reg.get<PositionComponent>(victim_ent);
        auto& vic_actor = reg.get<ActorComponent>(victim_ent);

        auto log_msg = [&](const std::string& s) {
            std::cout << s << "\n";
            if (out_logs) out_logs->push_back(s);
        };

        if (atk_pos.elevation != ElevationLevel::Rooftop || vic_pos.elevation != ElevationLevel::Street) {
            log_msg("[暗杀无效] 俯冲暗杀要求攻击者在屋顶、受害者在街面!");
            return false;
        }

        int32_t dist = atk_pos.coord.manhattan_dist(vic_pos.coord);
        if (dist > 2) {
            log_msg("[暗杀无效] 目标距离过远，无法施展高空袭杀!");
            return false;
        }

        log_msg("\n>>> [高空暗杀] " + atk_actor.name + " 从屋顶阴影中无声滑翔俯冲，刺向街面的 " + vic_actor.name + "!");

        // 高度优势加成
        float elevation_mult = 1.35f;
        if (reg.has<TacticalElevationComponent>(attacker_ent)) {
            elevation_mult += reg.get<TacticalElevationComponent>(attacker_ent).high_ground_dmg_bonus;
        }

        int32_t base_damage = 50;
        int32_t final_damage = static_cast<int32_t>(base_damage * elevation_mult);

        // 奥塔拉尔破魔加成检定
        std::vector<std::string> ota_logs;
        OtataralSystem::apply_otataral_melee_bonus(attacker_ent, victim_ent, final_damage, reg, ota_logs);
        for (const auto& ol : ota_logs) log_msg("  " + ol);

        vic_actor.take_damage(final_damage, DamageType::PhysicalSlash);

        // 攻击者落至街面
        atk_pos.coord = vic_pos.coord;
        atk_pos.elevation = ElevationLevel::Street;

        log_msg("    * 破空暴击造成 " + std::to_string(final_damage) + " 点致命伤害!");
        log_msg("    * [" + vic_actor.name + "] 当前 HP: " + std::to_string(vic_actor.hp) + (vic_actor.is_alive ? "" : " 【当场毙命】"));

        return true;
    }
};

} // namespace Malazan
