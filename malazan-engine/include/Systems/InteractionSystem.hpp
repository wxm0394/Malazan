#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include <algorithm>

namespace Malazan {

class InteractionSystem {
public:
    // 寻找玩家周边可交互的目标（NPC、商人、盟友）
    static Entity find_nearby_interactable(Entity player, Registry& reg, int32_t max_dist = 2) {
        if (!reg.has<PositionComponent>(player)) return NULL_ENTITY;
        auto& p_pos = reg.get<PositionComponent>(player);

        Entity best_entity = NULL_ENTITY;
        int32_t best_dist = 9999;

        for (auto [ent, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
            if (ent == player || !actor.is_alive) continue;
            // 敌对爪牙不可进行日常交谈
            if (actor.faction == Faction::ImperialClaw || actor.faction == Faction::JaghutTyrantRaest) continue;

            // 同层级或近距离高程
            int32_t dist = pos.coord.manhattan_dist(p_pos.coord);
            if (dist <= max_dist && dist < best_dist) {
                best_dist = dist;
                best_entity = ent;
            }
        }

        return best_entity;
    }
};

} // namespace Malazan
