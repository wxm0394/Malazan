#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/AIComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include "World/HazardSystem.hpp"
#include "Render/ScreenEffects.hpp"
#include "Core/DataLoader.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace Malazan {

enum class RiftType : uint8_t {
    ChaosDistortion,   // 混沌裂隙：空间引力撕裂，扭曲现实，释放深渊恶魔
    KuraldGalainDark,  // 库拉尔德·加兰太古纯暗：漆黑如夜，压制非暗夜阵营视野与心智
    TellannElderFire   // 提兰太古荒火：金色烈焰翻涌，引燃地脉蓝火煤气，重塑不朽白骨
};

using WarrenRiftType = RiftType;

struct WarrenRift {
    Vec2i pos{0, 0};
    ElevationLevel elev{ElevationLevel::Street};
    Vec2i coord{0, 0};
    ElevationLevel elevation{ElevationLevel::Street};
    RiftType type{RiftType::ChaosDistortion};
    float instability{100.0f};  // 裂隙能量稳定性
    int32_t danger_radius{2};
    bool demon_spawned{false};
    bool is_open{true};
};

class WarrenRiftSystem {
public:
    static inline std::vector<WarrenRift> active_rifts;
    static inline float local_planar_stress{0.0f}; // 局部空间应力累积槽

    // 累积施法魔力通量：过度施法撕裂维度帷幕
    static void accumulate_magic_flux(
        Vec2i pos,
        ElevationLevel elev,
        float flux,
        Registry& reg,
        DarujhistanMap& map,
        std::vector<std::string>& out_logs
    ) {
        local_planar_stress += flux;
        if (local_planar_stress >= 70.0f) {
            // 空间承受极限突破！强行撕开一道混沌沃伦裂隙
            local_planar_stress = 0.0f;
            open_rift(pos, elev, RiftType::ChaosDistortion, out_logs);
        }
        (void)reg;
        (void)map;
    }

    static void open_rift(
        Vec2i pos,
        ElevationLevel elev,
        RiftType type,
        float instability = 100.0f
    ) {
        std::vector<std::string> dummy_logs;
        open_rift(pos, elev, type, dummy_logs);
        if (!active_rifts.empty()) {
            active_rifts.back().instability = instability;
        }
    }

    // 主动打开指定类型的维度裂隙
    static void open_rift(
        Vec2i pos,
        ElevationLevel elev,
        RiftType type,
        std::vector<std::string>& out_logs
    ) {
        WarrenRift rift{
            .pos = pos,
            .elev = elev,
            .coord = pos,
            .elevation = elev,
            .type = type,
            .instability = 100.0f,
            .danger_radius = 2,
            .demon_spawned = false,
            .is_open = true
        };
        active_rifts.push_back(rift);

        ScreenEffects::add_trauma(0.55f);
        ScreenEffects::trigger_distortion_wave(0.9f);

        out_logs.push_back("=================================================================");
        if (type == RiftType::ChaosDistortion) {
            out_logs.push_back("【空间裂隙骤现!】狂暴的混沌维度帷幕在 (" + std::to_string(pos.x) + ", " 
                               + std::to_string(pos.y) + ") 被硬生生撕裂！");
            out_logs.push_back("紫黑色的空间乱流激荡，空气中弥漫着纯粹的现实崩解尖啸！");
        } else if (type == RiftType::KuraldGalainDark) {
            out_logs.push_back("【太古纯暗之渊!】Kurald Galain 母界裂隙于 (" + std::to_string(pos.x) + ", " 
                               + std::to_string(pos.y) + ") 洞开！");
            out_logs.push_back("至暗吞噬了达鲁基斯坦的所有微光，暗夜之主的威能在此得到无尽增幅！");
        } else {
            out_logs.push_back("【提兰太古荒火!】Tellann 炽烈火柱破土而出，古老的骨骸仪式再临人间！");
        }
        out_logs.push_back("=================================================================");
    }

    // 每回合驱动裂隙辐射与异化逻辑
    static void update_rifts(
        Registry& reg,
        DarujhistanMap& map,
        std::vector<std::string>& out_logs
    ) {
        for (auto it = active_rifts.begin(); it != active_rifts.end(); ) {
            auto& rift = *it;

            // 1. 裂隙引力对周围实体的波及
            for (auto [ent, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
                if (!actor.is_alive) continue;
                if (pos.elevation != rift.elev) continue;

                int dist = pos.coord.manhattan_dist(rift.pos);
                if (dist <= rift.danger_radius) {
                    if (rift.type == RiftType::ChaosDistortion) {
                        // 空间畸变撕裂：承受混沌虚空伤害
                        int32_t chaos_dmg = 20;
                        actor.take_damage(chaos_dmg, DamageType::ChaosDisruption);
                        out_logs.push_back("    * 空间乱流撕扯了 [" + actor.name + "]！承受 20 点混沌真伤！");
                    } else if (rift.type == RiftType::TellannElderFire) {
                        // 若是泰兰·伊玛斯，荒火为其修复白骨；其他生物受灼烧
                        if (actor.faction == Faction::TlanImass) {
                            actor.hp = std::min(actor.max_hp, actor.hp + 25);
                            out_logs.push_back("    * 提兰荒火重塑了太古骨骸战士的燧石铠甲！(+25 HP)");
                        } else {
                            actor.take_damage(15, DamageType::WarrenFire);
                        }
                    }
                }
            }

            // 2. 提兰荒火引爆周边瓦斯气云
            if (rift.type == RiftType::TellannElderFire) {
                HazardSystem::check_and_detonate_gas(rift.pos, rift.elev, reg, map, out_logs);
            }

            // 3. 混沌裂隙恶魔入侵 (Demon Incursion)
            if (rift.type == RiftType::ChaosDistortion && !rift.demon_spawned && rift.instability >= 50.0f) {
                rift.demon_spawned = true;
                Entity demon = reg.create_entity();
                reg.emplace<PositionComponent>(demon, PositionComponent{
                    .coord = rift.pos,
                    .elevation = rift.elev,
                    .can_climb_roofs = true,
                    .rooftop_jump_range = 2
                });
                reg.emplace<ActorComponent>(demon, ActorComponent{
                    .name = "混沌裂隙恶魔 (Chaos Rift Demon)",
                    .faction = Faction::ChaosVoidDemons,
                    .hp = 140,
                    .max_hp = 140,
                    .is_alive = true
                });
                reg.emplace<TurnEnergyComponent>(demon, TurnEnergyComponent{
                    .energy = 0,
                    .speed = 1200
                });
                reg.emplace<AIComponent>(demon, AIComponent{
                    .state = AIBehaviorState::Combat,
                    .target_interest_pos = rift.pos,
                    .sight_range = 8,
                    .hearing_range = 10,
                    .attack_range = 1,
                    .attack_power = 38,
                    .alert_meter = 100.0f,
                    .patrol_waypoints = {},
                    .current_waypoint_idx = 0
                });

                out_logs.push_back(">>> 【深渊入侵!】一头长满倒刺重鳞的【混沌裂隙恶魔】自裂隙深处挣脱爬出！");
                ScreenEffects::add_trauma(0.6f);
            }

            // 4. 裂隙能量自然衰退
            rift.instability -= 25.0f;
            if (rift.instability <= 0.0f) {
                out_logs.push_back(">>> (" + std::to_string(rift.pos.x) + ", " 
                                   + std::to_string(rift.pos.y) + ") 处的维度裂隙能量耗尽，空间闭合修复。");
                it = active_rifts.erase(it);
            } else {
                ++it;
            }
        }
    }

    [[nodiscard]] static const std::vector<WarrenRift>& get_active_rifts() {
        return active_rifts;
    }

    static void clear() {
        active_rifts.clear();
        local_planar_stress = 0.0f;
    }
};

} // namespace Malazan
