#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "World/DarujhistanMap.hpp"
#include <iostream>
#include <vector>

namespace Malazan {

class BlueFireGasSystem {
public:
    // 引爆指定的煤气节点并引起连锁反应
    static void detonate_gas_node(
        Entity node_ent,
        Registry& reg,
        DarujhistanMap& map
    ) {
        if (!reg.has<BlueFireNodeComponent>(node_ent)) return;

        auto& node = reg.get<BlueFireNodeComponent>(node_ent);
        if (!node.is_lit && node.gas_pressure <= 0.0f) return;

        std::cout << "\n>>> [环境殉爆] 达鲁基斯坦蓝火煤气管道节点 (" 
                  << node.coord.x << ", " << node.coord.y << ") 被诱爆！\n";

        Vec2i center = node.coord;
        int32_t radius = node.blast_radius;
        int32_t blast_dmg = node.blast_yield;

        // 管道泄压失效
        node.gas_pressure = 0.0f;
        node.is_lit = false;
        node.is_leaking = false;

        // 1. 冲击波使附近的屋顶坍塌
        for (int32_t dy = -radius; dy <= radius; ++dy) {
            for (int32_t dx = -radius; dx <= radius; ++dx) {
                Vec2i target{center.x + dx, center.y + dy};
                if (!map.in_bounds(target)) continue;

                // 距爆心 1 格内的屋顶直接震塌
                if (center.manhattan_dist(target) <= 2) {
                    map.collapse_rooftop(target);
                }
            }
        }
        std::cout << "    * 剧烈冲击波震碎附近建筑拱顶，瓦砾崩塌至小巷中！\n";

        // 2. 波及范围内的所有生物（无论街面还是屋顶）
        for (auto [ent, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
            if (!actor.is_alive) continue;

            int32_t dist = pos.coord.manhattan_dist(center);
            if (dist <= radius) {
                int32_t actual_dmg = blast_dmg - (dist * 12);
                if (actual_dmg > 0) {
                    std::cout << "    * 幽蓝烈焰席卷 [" << actor.name << "]！造成 " 
                              << actual_dmg << " 点莫兰斯/瓦斯爆炸混合伤害！\n";
                    actor.take_damage(actual_dmg, DamageType::MoranthExplosive);
                    std::cout << "      [" << actor.name << "] 剩余生命: " << actor.hp << "\n";
                }
            }
        }
    }

    // 投掷莫兰斯炸药（破桥军刺客/工兵招牌战术）
    static bool throw_moranth_munition(
        Entity thrower,
        MoranthMunitionType type,
        Vec2i target_coord,
        Registry& reg,
        DarujhistanMap& map
    ) {
        if (!reg.has<ActorComponent>(thrower) || !reg.has<MoranthMunitionComponent>(thrower)) {
            return false;
        }

        auto& actor = reg.get<ActorComponent>(thrower);
        auto& munitions = reg.get<MoranthMunitionComponent>(thrower);

        if (!munitions.use(type)) {
            std::cout << "[战备不足] " << actor.name << " 装备中已无此类莫兰斯弹药!\n";
            return false;
        }

        std::cout << "\n>>> [破桥军战术] " << actor.name << " 拔出莫兰斯炼金炸药掷向坐标 (" 
                  << target_coord.x << ", " << target_coord.y << ")!\n";

        // 检查落点是否有蓝火煤气节点
        bool detonated_gas = false;
        for (auto [node_ent, node_comp] : reg.view<BlueFireNodeComponent>()) {
            if (node_comp.coord.manhattan_dist(target_coord) <= 1) {
                std::cout << "    * 炸药火舌触及了裸露的蓝火管线！\n";
                detonate_gas_node(node_ent, reg, map);
                detonated_gas = true;
                break;
            }
        }

        if (!detonated_gas) {
            std::cout << "    * 莫兰斯炸药轰鸣炸裂，黑烟与炽烈破片扫荡四周！\n";
            for (auto [ent, pos, victim] : reg.view<PositionComponent, ActorComponent>()) {
                if (pos.coord.manhattan_dist(target_coord) <= 2 && victim.is_alive) {
                    victim.take_damage(45, DamageType::MoranthExplosive);
                    std::cout << "      [" << victim.name << "] 被破片波及，受到 45 点伤害 (剩余 HP: " 
                              << victim.hp << ")\n";
                }
            }
        }

        return true;
    }
};

} // namespace Malazan
