#pragma once
#include "Core/Types.hpp"
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "Components/BossComponent.hpp"
#include "World/DarujhistanMap.hpp"

#include <string>
#include <vector>
#include <iostream>

namespace Malazan {

enum class StoryAct : uint8_t {
    Act1_Infiltration,     // 第一幕：暗影潜伏·凤凰客栈保卫战
    Act2_TyrantAwakening,  // 第二幕：太古严霜·贾格特暴君降临
    Act3_AzathConvergence, // 第三幕：万神汇聚·阿扎斯破土永恒封印
    Victory                // 主线通关：达鲁基斯坦和平与破桥军的新纪元
};

class StorylineSystem {
public:
    static inline StoryAct current_act = StoryAct::Act1_Infiltration;
    static inline int32_t claws_eliminated = 0;
    static inline int32_t claws_required = 1;
    static inline Entity boss_raest_entity = NULL_ENTITY;

    // 获取当前主线任务标题与目标
    static std::string get_act_title() {
        switch (current_act) {
            case StoryAct::Act1_Infiltration:
                return "第一幕：暗巷围猎·凤凰客栈的阴影";
            case StoryAct::Act2_TyrantAwakening:
                return "第二幕：太古坚冰·贾格特暴君降临";
            case StoryAct::Act3_AzathConvergence:
                return "第三幕：万神汇聚·阿扎斯破土封印";
            case StoryAct::Victory:
                return "终局：蓝火之城的破晓 (VICTORY)";
        }
        return "";
    }

    static std::string get_current_objective() {
        switch (current_act) {
            case StoryAct::Act1_Infiltration:
                return "消灭潜伏在圆形大区小巷的帝国爪牙特工 (" 
                       + std::to_string(claws_eliminated) + "/" + std::to_string(claws_required) + ")";
            case StoryAct::Act2_TyrantAwakening:
                return "诱导贾格特暴君雷斯特至煤气中心 (10, 6)，击碎其坚冰护甲";
            case StoryAct::Act3_AzathConvergence:
                return "唤醒阿扎斯地脉根须，将失去护甲的暴君永世拖入囚笼！";
            case StoryAct::Victory:
                return "暴君已被封入阿扎斯之屋。达鲁基斯坦守住了自由。";
        }
        return "";
    }

    // 检查并推进主线进度
    static void check_progression(
        Registry& reg, 
        DarujhistanMap& map,
        std::vector<std::string>& out_logs
    ) {
        if (current_act == StoryAct::Act1_Infiltration) {
            // 检查爪牙是否全部被击杀
            bool claw_alive = false;
            for (auto [ent, actor] : reg.view<ActorComponent>()) {
                if (actor.faction == Faction::ImperialClaw && actor.is_alive) {
                    claw_alive = true;
                    break;
                }
            }

            if (!claw_alive) {
                claws_eliminated = claws_required;
                current_act = StoryAct::Act2_TyrantAwakening;
                out_logs.push_back("=================================================");
                out_logs.push_back("【主线推进】帝国爪牙肃清！凤凰客栈暂时脱险。");
                out_logs.push_back("突然，北面加德罗比丘陵传来崩裂巨响！");
                out_logs.push_back("贾格特上古暴君雷斯特 (Raest) 破封出世！");
                out_logs.push_back("极度严寒的 Omtose Phellack 坚冰风暴正向达鲁基斯坦压来！");
                out_logs.push_back("=================================================");
                
                spawn_jaghut_tyrant(reg, map);
            }
        }
        else if (current_act == StoryAct::Act2_TyrantAwakening) {
            if (boss_raest_entity != NULL_ENTITY && reg.has<ActorComponent>(boss_raest_entity)) {
                auto& boss_pos = reg.get<PositionComponent>(boss_raest_entity);
                auto& boss_comp = reg.get<JaghutTyrantComponent>(boss_raest_entity);

                // 若坚冰护甲被打破或生命过半，且靠近阿扎斯地脉 (10, 6)
                if (boss_comp.frost_armor <= 0 && boss_pos.coord.manhattan_dist({10, 6}) <= 2) {
                    current_act = StoryAct::Act3_AzathConvergence;
                    out_logs.push_back("=================================================");
                    out_logs.push_back("【主线推进】暴君的远古坚冰护甲全面崩碎！");
                    out_logs.push_back("地底深处的阿扎斯古木感知到了毁灭性混沌引力！");
                    out_logs.push_back("石板路剧烈隆起！万神汇聚的终局时刻已至！");
                    out_logs.push_back("按 [5] 或 [SPACE] 触发阿扎斯破土永恒封印！");
                    out_logs.push_back("=================================================");
                }
            }
        }
    }

    // 触发第三幕阿扎斯破土封印大结局
    static void trigger_azath_sealing(
        Registry& reg, 
        DarujhistanMap& map,
        std::vector<std::string>& out_logs
    ) {
        if (current_act != StoryAct::Act3_AzathConvergence) return;

        if (boss_raest_entity != NULL_ENTITY && reg.has<ActorComponent>(boss_raest_entity)) {
            auto& boss_actor = reg.get<ActorComponent>(boss_raest_entity);
            auto& boss_comp = reg.get<JaghutTyrantComponent>(boss_raest_entity);

            boss_comp.is_sealed_in_azath = true;
            boss_actor.is_alive = false; // 永恒囚禁等同于战术歼灭

            // 将爆心及其周围转换为“阿扎斯树屋 (Azath Barrow Tree)”地貌
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    Vec2i p{10 + dx, 6 + dy};
                    if (map.in_bounds(p)) {
                        auto& street = map.get_cell(p, ElevationLevel::Street);
                        street.terrain = TileTerrain::BuildingWall;
                        street.blocks_movement = true;
                    }
                }
            }

            current_act = StoryAct::Victory;
            out_logs.push_back("*************************************************");
            out_logs.push_back("【神迹封印】苍白的树木与古老石屋自石板下悍然破土！");
            out_logs.push_back("漆黑的树根如巨蟒般死死绞住了贾格特暴君的身躯！");
            out_logs.push_back("雷斯特咆哮着被拖入阿扎斯之屋幽暗的虚空回廊！");
            out_logs.push_back("天空中漂浮城 (Moon's Spawn) 的巨影悄然掠过，黑剑归鞘。");
            out_logs.push_back("达鲁基斯坦仲夏夜狂欢节的蓝火重新绽放。破桥军赢得了黎明！");
            out_logs.push_back("*************************************************");
        }
    }

private:
    static void spawn_jaghut_tyrant(Registry& reg, DarujhistanMap& map) {
        (void)map;
        boss_raest_entity = reg.create_entity();

        reg.emplace<ActorComponent>(boss_raest_entity, ActorComponent{
            .name = "贾格特暴君·雷斯特 (Raest)",
            .faction = Faction::JaghutTyrantRaest,
            .hp = 350,
            .max_hp = 350,
            .is_alive = true
        });

        reg.emplace<PositionComponent>(boss_raest_entity, PositionComponent{
            .coord = {13, 7}, // 从东侧小巷强行突入战场
            .elevation = ElevationLevel::Street,
            .can_climb_roofs = false
        });

        reg.emplace<TurnEnergyComponent>(boss_raest_entity, TurnEnergyComponent{
            .energy = 500,
            .speed = 900 // 巨大体态，沉重而毁灭性
        });

        reg.emplace<JaghutTyrantComponent>(boss_raest_entity, JaghutTyrantComponent{
            .is_empowered = true,
            .omtose_blizzard_timer = 0.0f,
            .frost_armor = 100,
            .aura_radius = 4,
            .is_sealed_in_azath = false
        });

        // 暴君具备远古暴虐感知与极强破坏攻击
        reg.emplace<AIComponent>(boss_raest_entity, AIComponent{
            .state = AIBehaviorState::Combat,
            .target_interest_pos = {10, 6}, // 直奔精力果与地脉中心
            .sight_range = 10,
            .hearing_range = 12,
            .attack_range = 3,
            .attack_power = 48,
            .alert_meter = 100.0f,
            .patrol_waypoints = {{10, 6}}
        });
    }
};

} // namespace Malazan
