#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "World/DarujhistanMap.hpp"
#include <iostream>

namespace Malazan {

class RooftopTacticsSystem {
public:
    // 屋顶飞跃移动（刺客/盗贼特长）
    static bool attempt_rooftop_leap(
        Entity actor_ent,
        Registry& reg,
        DarujhistanMap& map,
        Vec2i destination
    ) {
        if (!reg.has<PositionComponent>(actor_ent) || !reg.has<ActorComponent>(actor_ent)) {
            return false;
        }

        auto& pos = reg.get<PositionComponent>(actor_ent);
        auto& actor = reg.get<ActorComponent>(actor_ent);

        if (!pos.can_climb_roofs) {
            std::cout << "[战术移动] " << actor.name << " 未受过轻功训练，无法在屋顶斜瓦间跳跃!\n";
            return false;
        }

        int32_t distance = pos.coord.manhattan_dist(destination);
        if (distance > pos.rooftop_jump_range) {
            std::cout << "[战术移动] 目标距离 (" << distance << " 格) 超出 " 
                      << actor.name << " 的飞跃极限 (" << pos.rooftop_jump_range << " 格)!\n";
            return false;
        }

        // 检查目标地点在屋顶层的可行走性
        const auto& target_roof = map.get_cell(destination, ElevationLevel::Rooftop);
        if (target_roof.terrain == TileTerrain::EmptyAir || target_roof.roof_collapsed) {
            std::cout << "[战术移动] 目标点没有坚实屋檐支撑（虚空或已坍塌），强行跳跃将跌落街面!\n";
            // 跌落街面并承受坠落伤害
            pos.coord = destination;
            pos.elevation = ElevationLevel::Street;
            actor.take_damage(25, DamageType::PhysicalSlash);
            std::cout << "    * " << actor.name << " 摔落在石板路上，受到 25 点坠落钝击伤害! (HP: " << actor.hp << ")\n";
            return true;
        }

        // 成功飞跃
        std::cout << "[屋顶飞跃] " << actor.name << " 灵巧地跃过小巷上空，从 (" 
                  << pos.coord.x << ", " << pos.coord.y << ") 落在 (" 
                  << destination.x << ", " << destination.y << ") 的青石屋顶上!\n";
        pos.coord = destination;
        pos.elevation = ElevationLevel::Rooftop;

        return true;
    }

    // 从屋顶向街面目标发动高空俯冲突袭 (Plunging Ambush)
    static bool execute_plunging_ambush(
        Entity attacker_ent,
        Entity victim_ent,
        Registry& reg,
        DarujhistanMap& map
    ) {
        (void)map;
        auto& atk_pos = reg.get<PositionComponent>(attacker_ent);
        auto& atk_actor = reg.get<ActorComponent>(attacker_ent);
        auto& vic_pos = reg.get<PositionComponent>(victim_ent);
        auto& vic_actor = reg.get<ActorComponent>(victim_ent);

        if (atk_pos.elevation != ElevationLevel::Rooftop || vic_pos.elevation != ElevationLevel::Street) {
            std::cout << "[暗杀无效] 俯冲暗杀要求攻击者在屋顶、受害者在街面!\n";
            return false;
        }

        int32_t dist = atk_pos.coord.manhattan_dist(vic_pos.coord);
        if (dist > 2) {
            std::cout << "[暗杀无效] 目标距离过远，无法施展高空袭杀!\n";
            return false;
        }

        std::cout << "\n>>> [高空暗杀] " << atk_actor.name << " 从屋顶阴影中无声滑翔俯冲，刺向街面的 " 
                  << vic_actor.name << "!\n";

        // 高度优势加成
        float elevation_mult = 1.35f;
        if (reg.has<TacticalElevationComponent>(attacker_ent)) {
            elevation_mult += reg.get<TacticalElevationComponent>(attacker_ent).high_ground_dmg_bonus;
        }

        int32_t base_damage = 50;
        int32_t final_damage = static_cast<int32_t>(base_damage * elevation_mult);

        vic_actor.take_damage(final_damage, DamageType::PhysicalSlash);

        // 攻击者落至街面
        atk_pos.coord = vic_pos.coord;
        atk_pos.elevation = ElevationLevel::Street;

        std::cout << "    * 破空暴击造成 " << final_damage << " 点致命撕裂伤害!\n";
        std::cout << "    * [" << vic_actor.name << "] 当前 HP: " << vic_actor.hp;
        if (!vic_actor.is_alive) {
            std::cout << " 【当场毙命】\n";
        } else {
            std::cout << "\n";
        }

        return true;
    }
};

} // namespace Malazan
