#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include <vector>
#include <iostream>

namespace Malazan {

class TurnSystem {
public:
    static inline uint64_t global_tick_count{0};

    // 寻找当前拥有行动权的实体（能量 >= 1000）
    // 若没有，则步进推进世界时间片（所有实体根据自身 speed 累加能量）
    static Entity advance_until_actor_ready(Registry& registry) {
        while (true) {
            // 扫描是否有已准备好的实体
            Entity ready_entity = NULL_ENTITY;
            int32_t highest_energy = -1;

            for (auto [ent, energy, actor] : registry.view<TurnEnergyComponent, ActorComponent>()) {
                if (!actor.is_alive) continue;

                if (energy.can_act() && energy.energy > highest_energy) {
                    highest_energy = energy.energy;
                    ready_entity = ent;
                }
            }

            if (ready_entity != NULL_ENTITY) {
                return ready_entity;
            }

            // 没有实体行动就绪，推进一个微时间步 (Tick)
            global_tick_count++;
            for (auto [ent, energy, actor] : registry.view<TurnEnergyComponent, ActorComponent>()) {
                if (actor.is_alive) {
                    energy.energy += energy.speed;
                }
            }
        }
    }
};

} // namespace Malazan
