#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/DragnipurComponent.hpp"
#include "Components/AIComponent.hpp"
#include <iostream>
#include <vector>
#include <string>

namespace Malazan {

class DragnipurSystem {
public:
    // 德拉格尼普尔出鞘斩杀与摄魂拉车
    static bool slay_and_trap(
        Entity killer,
        Entity victim,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<DragnipurComponent>(killer)) return false;
        if (!reg.has<ActorComponent>(victim)) return false;

        auto& dragnipur = reg.get<DragnipurComponent>(killer);
        auto& vic_actor = reg.get<ActorComponent>(victim);
        auto& vic_pos = reg.get<PositionComponent>(victim);

        vic_actor.hp = 0;
        vic_actor.is_alive = false;

        float pull_str = 2.0f;
        if (reg.has<AIComponent>(victim)) {
            pull_str = static_cast<float>(reg.get<AIComponent>(victim).attack_power) * 0.1f;
        }

        dragnipur.trapped_souls.push_back(TrappedSoul{
            .name = vic_actor.name,
            .former_faction = vic_actor.faction,
            .combat_power = vic_actor.max_hp,
            .pull_effort = pull_str
        });

        // 巨车远离混沌推进
        dragnipur.chaos_pursuit_distance = std::min(100.0f, dragnipur.chaos_pursuit_distance + 3.5f);

        out_logs.push_back("=================================================================");
        out_logs.push_back("【德拉格尼普尔·万魂锁链!】黑剑出鞘，达鲁基斯坦夜空星辰失色!");
        out_logs.push_back("数丈粗的沉重铁链自虚空深处激射而出，穿透 [" + vic_actor.name + "] 的魂魄!");
        out_logs.push_back("灵魂被狂暴拖拽入黑剑内部的无垠灰烬平原，套上轭具拉动上古纯暗大车!");
        out_logs.push_back("大车拉力提升至: " + std::to_string(dragnipur.get_total_pull_power()).substr(0, 5) 
                           + " | 逃离混沌安全余量: " + std::to_string(dragnipur.chaos_pursuit_distance).substr(0, 4) + "%");
        out_logs.push_back("=================================================================");

        (void)vic_pos;
        return true;
    }

    // 太古纯暗威压光环（震慑全图心智弱小者）
    static void apply_dread_aura(Entity rake, Registry& reg, std::vector<std::string>& out_logs) {
        if (!reg.has<DragnipurComponent>(rake) || !reg.has<PositionComponent>(rake)) return;
        auto& rake_pos = reg.get<PositionComponent>(rake);
        auto& dragnipur = reg.get<DragnipurComponent>(rake);

        for (auto [ent, pos, ai, actor] : reg.view<PositionComponent, AIComponent, ActorComponent>()) {
            if (ent == rake || !actor.is_alive) continue;
            int32_t dist = pos.coord.manhattan_dist(rake_pos.coord);
            if (dist <= static_cast<int32_t>(dragnipur.terror_aura_radius)) {
                if (ai.state == AIBehaviorState::Combat) {
                    ai.state = AIBehaviorState::Suspicious; // 恐惧退缩
                    out_logs.push_back("【纯暗恐惧压制】[" + actor.name + "] 感知到德拉格尼普尔的锁链悲鸣，战意瓦解，惊恐后退!");
                }
            }
        }
    }
};

} // namespace Malazan
