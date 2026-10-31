#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/InventoryComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include "World/HazardSystem.hpp"
#include "Render/ScreenEffects.hpp"

#include <vector>
#include <string>
#include <algorithm>

namespace Malazan {

struct SapperMine {
    Vec2i pos{0, 0};
    ElevationLevel elev{ElevationLevel::Street};
    Vec2i coord{0, 0};
    ElevationLevel elevation{ElevationLevel::Street};
    int32_t damage{125};
    int32_t trigger_dist{1};
    Faction placed_by{Faction::Bridgeburners};
    bool is_armed{true};
};

class SquadTacticsSystem {
public:
    static inline std::vector<SapperMine> deployed_mines;

    // 1. 工兵战术布雷 (Deploy Directional Mine)
    static bool deploy_mine(
        Entity sapper,
        Vec2i pos,
        ElevationLevel elev,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        // 检查工兵行囊中是否有地雷
        bool has_mine = false;
        if (reg.has<InventoryComponent>(sapper)) {
            auto& inv = reg.get<InventoryComponent>(sapper);
            for (size_t i = 0; i < inv.backpack.size(); ++i) {
                if (inv.backpack[i].id == "moranth_directional_mine") {
                    inv.backpack[i].charges--;
                    if (inv.backpack[i].charges <= 0) {
                        inv.remove_item(i);
                    }
                    has_mine = true;
                    break;
                }
            }
        }

        if (!has_mine) {
            out_logs.push_back("[工兵提示] 行囊中缺少【破桥军莫兰斯定向地雷】，可在工坊 [C] 组装！");
            return false;
        }

        Faction placer_faction = reg.has<ActorComponent>(sapper) ? reg.get<ActorComponent>(sapper).faction : Faction::Bridgeburners;
        deployed_mines.push_back(SapperMine{
            .pos = pos,
            .elev = elev,
            .coord = pos,
            .elevation = elev,
            .damage = 125,
            .trigger_dist = 1,
            .placed_by = placer_faction,
            .is_armed = true
        });

        out_logs.push_back(">>> 【工兵布设完成】卡拉姆在 (" + std::to_string(pos.x) + ", " 
                           + std::to_string(pos.y) + ") 埋设了莫兰斯定向死域雷！");
        return true;
    }

    // 2. 地雷触发行走检测 (Trigger Detection)
    static void check_mine_triggers(
        Registry& reg,
        DarujhistanMap& map,
        std::vector<std::string>& out_logs
    ) {
        for (auto it = deployed_mines.begin(); it != deployed_mines.end(); ) {
            auto& mine = *it;
            if (!mine.is_armed) {
                ++it;
                continue;
            }

            bool triggered = false;
            for (auto [ent, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
                if (!actor.is_alive) continue;
                if (pos.elevation != mine.elev) continue;
                if (actor.faction == mine.placed_by) continue; // 不误伤同盟

                if (pos.coord.manhattan_dist(mine.pos) <= mine.trigger_dist) {
                    triggered = true;
                    out_logs.push_back("=================================================================");
                    out_logs.push_back("【轰——! 定向破片雷引爆!】[" + actor.name + "] 踩中工兵死域地雷！");

                    // 造成破片割裂巨创
                    actor.take_damage(mine.damage, DamageType::MoranthExplosive);
                    out_logs.push_back("    * 几丁质钢片撕裂血肉！承受 " + std::to_string(mine.damage) 
                                       + " 点毁灭伤害！(剩余 HP: " + std::to_string(actor.hp) + ")");

                    // 震屏与瓦斯云爆联动
                    ScreenEffects::add_trauma(0.75f);
                    HazardSystem::check_and_detonate_gas(mine.pos, mine.elev, reg, map, out_logs);

                    // 若在地表或屋顶，可能震塌瓦片
                    if (mine.elev == ElevationLevel::Rooftop) {
                        map.collapse_rooftop(mine.pos);
                    }
                    out_logs.push_back("=================================================================");
                    break;
                }
            }

            if (triggered) {
                it = deployed_mines.erase(it);
            } else {
                ++it;
            }
        }
    }

    // 3. 破桥军小队战术协同：快班远程火力掩护 (Quick Ben Covering Fire)
    static bool trigger_covering_fire(
        Entity kalam,
        Entity target,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<ActorComponent>(target) || !reg.get<ActorComponent>(target).is_alive) {
            return false;
        }

        auto& target_act = reg.get<ActorComponent>(target);
        int32_t spell_dmg = 45;
        target_act.take_damage(spell_dmg, DamageType::WarrenFire);

        ScreenEffects::add_trauma(0.35f);
        out_logs.push_back("=================================================");
        out_logs.push_back("【小队战术联动·快班火力压制!】");
        out_logs.push_back("屋顶阴影中的快班默念咒文，特拉斯（Telas）炽焰飞弹凌空轰击 [" 
                           + target_act.name + "]！");
        out_logs.push_back("造成 45 点烈焰灼伤与护甲融毁！(目标 HP: " + std::to_string(target_act.hp) + ")");
        out_logs.push_back("=================================================");
        (void)kalam;
        return true;
    }

    static bool trigger_covering_fire(
        Entity source,
        Entity kalam,
        Vec2i target_coord,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        Entity target_ent = NULL_ENTITY;
        for (auto [e, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
            if (actor.is_alive && pos.coord == target_coord) {
                target_ent = e;
                break;
            }
        }
        if (target_ent != NULL_ENTITY) {
            return trigger_covering_fire(kalam, target_ent, reg, out_logs);
        }

        ScreenEffects::add_trauma(0.25f);
        out_logs.push_back("=================================================");
        out_logs.push_back("【小队战术联动·快班火力压制!】");
        out_logs.push_back("快班向目标区域 (" + std::to_string(target_coord.x) + ", " 
                           + std::to_string(target_coord.y) + ") 轰出一道特拉斯炽焰弹！");
        out_logs.push_back("=================================================");
        (void)source;
        return true;
    }

    // 4. 破桥军小队紧急救援：米纳斯（Meanas）幻象脱离
    static bool trigger_meanas_veil(
        Entity kalam,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<ActorComponent>(kalam)) return false;
        auto& kalam_act = reg.get<ActorComponent>(kalam);

        kalam_act.hp = std::min(kalam_act.max_hp, kalam_act.hp + 40);
        ScreenEffects::trigger_distortion_wave(0.6f);

        out_logs.push_back("=================================================");
        out_logs.push_back("【小队战术联动·米纳斯幻象帷幕!】");
        out_logs.push_back("快班撕开幻术沃伦，在卡拉姆周身投下重重镜面虚影！");
        out_logs.push_back("迷惑敌方视线，卡拉姆生命恢复 40 点并进入战术隐蔽状态！");
        out_logs.push_back("=================================================");
        return true;
    }

    static bool trigger_meanas_veil(
        Entity source,
        Entity kalam,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        (void)source;
        return trigger_meanas_veil(kalam, reg, out_logs);
    }

    // 5. 城市多阵营混战生态演化 (Cross-Faction Skirmishes)
    static void update_cross_faction_skirmishes(
        Registry& reg,
        DarujhistanMap& map,
        std::vector<std::string>& out_logs
    ) {
        (void)map;
        // 遍历所有活着的战斗实体，若两个敌对阵营相邻即爆发内战交火
        for (auto [e1, p1, a1] : reg.view<PositionComponent, ActorComponent>()) {
            if (!a1.is_alive) continue;
            for (auto [e2, p2, a2] : reg.view<PositionComponent, ActorComponent>()) {
                if (e1 >= e2) continue; // 避免重复处理
                if (!a2.is_alive) continue;
                if (p1.elevation != p2.elevation) continue;

                // 判断是否是敌对阵营相遇
                bool is_hostile = false;
                if ((a1.faction == Faction::ImperialClaw && a2.faction == Faction::CityAssassinsGuild) ||
                    (a1.faction == Faction::CityAssassinsGuild && a2.faction == Faction::ImperialClaw)) {
                    is_hostile = true; // 帝国爪牙 vs 刺客公会 屋顶暗战
                } else if ((a1.faction == Faction::ChaosVoidDemons) || (a2.faction == Faction::ChaosVoidDemons)) {
                    is_hostile = true; // 裂隙恶魔与凡间万物皆为死敌
                } else if ((a1.faction == Faction::NeutralCivilians && a2.faction == Faction::ImperialClaw) ||
                           (a1.faction == Faction::ImperialClaw && a2.faction == Faction::NeutralCivilians)) {
                    is_hostile = true; // 城防军 vs 潜伏特工
                }

                if (is_hostile && p1.coord.manhattan_dist(p2.coord) <= 1) {
                    // 爆发阵营死斗
                    int32_t dmg1 = 28;
                    int32_t dmg2 = 32;
                    a2.take_damage(dmg1, DamageType::PhysicalSlash);
                    a1.take_damage(dmg2, DamageType::PhysicalSlash);

                    out_logs.push_back(">>> 【街市阵营混战!】[" + a1.name + "] 与 [" 
                                       + a2.name + "] 拔刃血战相撞！");
                    if (!a1.is_alive) out_logs.push_back("    * [" + a1.name + "] 喋血阵亡！");
                    if (!a2.is_alive) out_logs.push_back("    * [" + a2.name + "] 喋血阵亡！");
                }
            }
        }
    }

    [[nodiscard]] static const std::vector<SapperMine>& get_deployed_mines() {
        return deployed_mines;
    }

    static void clear() {
        deployed_mines.clear();
    }
};

} // namespace Malazan
