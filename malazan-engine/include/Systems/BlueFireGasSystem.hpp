#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/BossComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include <iostream>
#include <vector>
#include <string>

namespace Malazan {

class BlueFireGasSystem {
public:
    // 引爆指定的煤气节点并引起连锁反应
    static void detonate_gas_node(
        Entity node_ent,
        Registry& reg,
        DarujhistanMap& map,
        std::vector<std::string>* out_logs = nullptr
    ) {
        if (!reg.has<BlueFireNodeComponent>(node_ent)) return;

        auto& node = reg.get<BlueFireNodeComponent>(node_ent);
        if (!node.is_lit && node.gas_pressure <= 0.0f) return;

        auto log_msg = [&](const std::string& s) {
            std::cout << s << "\n";
            if (out_logs) out_logs->push_back(s);
        };

        log_msg("\n>>> [环境殉爆] 达鲁基斯坦蓝火煤气管道节点 (" 
                + std::to_string(node.coord.x) + ", " + std::to_string(node.coord.y) + ") 被诱爆！");

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

                // 距爆心 2 格内的屋顶直接震塌
                if (center.manhattan_dist(target) <= 2) {
                    map.collapse_rooftop(target);
                }
            }
        }
        log_msg("    * 剧烈冲击波震碎附近建筑拱顶，瓦砾崩塌至小巷中！");

        // 2. 波及范围内的所有生物
        for (auto [ent, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
            if (!actor.is_alive) continue;

            int32_t dist = pos.coord.manhattan_dist(center);
            if (dist <= radius) {
                int32_t actual_dmg = blast_dmg - (dist * 12);
                if (actual_dmg > 0) {
                    log_msg("    * 幽蓝烈焰席卷 [" + actor.name + "]！造成 " 
                            + std::to_string(actual_dmg) + " 点莫兰斯/瓦斯爆炸混合伤害！");
                    actor.take_damage(actual_dmg, DamageType::MoranthExplosive);
                    log_msg("      [" + actor.name + "] 剩余生命: " + std::to_string(actor.hp));
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
        DarujhistanMap& map,
        bool is_cusser = false,
        std::vector<std::string>* out_logs = nullptr
    ) {
        if (!reg.has<ActorComponent>(thrower)) return false;

        auto& actor = reg.get<ActorComponent>(thrower);
        auto log_msg = [&](const std::string& s) {
            std::cout << s << "\n";
            if (out_logs) out_logs->push_back(s);
        };

        // 尝试从 Inventory 或 MoranthMunitionComponent 消耗弹药
        bool consumed = false;
        if (reg.has<InventoryComponent>(thrower)) {
            auto& inv = reg.get<InventoryComponent>(thrower);
            for (size_t i = 0; i < inv.backpack.size(); ++i) {
                if (inv.backpack[i].is_munition && 
                    (is_cusser ? (inv.backpack[i].id == "moranth_cusser") : (inv.backpack[i].munition_type == type))) {
                    inv.backpack[i].charges--;
                    if (inv.backpack[i].charges <= 0) {
                        inv.remove_item(i);
                    }
                    consumed = true;
                    break;
                }
            }
        }

        if (!consumed && reg.has<MoranthMunitionComponent>(thrower)) {
            consumed = reg.get<MoranthMunitionComponent>(thrower).use(type);
        }

        // 如果没有弹药组件（比如无头脚本测试或特权调用），允许默认投掷
        if (!consumed && !reg.has<MoranthMunitionComponent>(thrower) && !reg.has<InventoryComponent>(thrower)) {
            consumed = true;
        }

        if (!consumed) {
            log_msg("[战备不足] " + actor.name + " 装备中已无此类莫兰斯弹药!");
            return false;
        }

        std::string bomb_name = is_cusser ? "【莫兰斯·卡瑟毁灭巨弹 (CUSSER)】" : "莫兰斯炼金炸药";
        log_msg("\n>>> [破桥军战术] " + actor.name + " 投掷 " + bomb_name + " 直击坐标 (" 
                + std::to_string(target_coord.x) + ", " + std::to_string(target_coord.y) + ")!");

        // 检查落点是否有蓝火煤气节点
        bool detonated_gas = false;
        for (auto [node_ent, node_comp] : reg.view<BlueFireNodeComponent>()) {
            if (node_comp.coord.manhattan_dist(target_coord) <= 2) {
                log_msg("    * 炸药火舌触及了裸露的蓝火管线！");
                detonate_gas_node(node_ent, reg, map, out_logs);
                detonated_gas = true;
                break;
            }
        }

        // 若是卡瑟炸弹 (Cusser)，额外引发超大范围毁灭性冲击波
        if (is_cusser) {
            log_msg("    【卡瑟终极引爆!】天崩地裂！整座街区的石造建筑屋顶在暴风中轰然崩塌！");
            for (int32_t dy = -3; dy <= 3; ++dy) {
                for (int32_t dx = -3; dx <= 3; ++dx) {
                    Vec2i target{target_coord.x + dx, target_coord.y + dy};
                    if (map.in_bounds(target)) {
                        map.collapse_rooftop(target);
                    }
                }
            }
            for (auto [ent, pos, victim] : reg.view<PositionComponent, ActorComponent>()) {
                int32_t dist = pos.coord.manhattan_dist(target_coord);
                if (dist <= 4 && victim.is_alive) {
                    int32_t c_dmg = 160 - dist * 25;
                    if (c_dmg > 0) {
                        // 暴君雷斯特破盾
                        if (reg.has<JaghutTyrantComponent>(ent)) {
                            auto& tyr = reg.get<JaghutTyrantComponent>(ent);
                            tyr.frost_armor = std::max(0, tyr.frost_armor - 80);
                            log_msg("    * 卡瑟巨力轰碎贾格特暴君坚冰护甲 -80！剩余护甲: " + std::to_string(tyr.frost_armor));
                        }
                        victim.take_damage(c_dmg, DamageType::MoranthExplosive);
                        log_msg("    * [" + victim.name + "] 承受 " + std::to_string(c_dmg) + " 点毁灭爆破冲击! (HP: " + std::to_string(victim.hp) + ")");
                    }
                }
            }
            return true;
        }

        if (!detonated_gas) {
            log_msg("    * 莫兰斯炸药轰鸣炸裂，黑烟与炽烈破片扫荡四周！");
            for (auto [ent, pos, victim] : reg.view<PositionComponent, ActorComponent>()) {
                if (pos.coord.manhattan_dist(target_coord) <= 2 && victim.is_alive) {
                    victim.take_damage(45, DamageType::MoranthExplosive);
                    log_msg("      [" + victim.name + "] 被破片波及，受到 45 点伤害 (剩余 HP: " 
                            + std::to_string(victim.hp) + ")");
                }
            }
        }

        return true;
    }
};

} // namespace Malazan
