#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "World/DarujhistanMap.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>

namespace Malazan {

struct GasCloud {
    Vec2i pos;
    ElevationLevel elev{ElevationLevel::Underground};
    float concentration{80.0f}; // 瓦斯浓度 (超过 30.0f 遇火即爆)
    bool is_combustible{true};
};

class HazardSystem {
public:
    static inline std::vector<GasCloud> active_gas_clouds;

    static void spawn_gas_leak(Vec2i pos, ElevationLevel elev, float conc = 85.0f) {
        active_gas_clouds.push_back(GasCloud{
            .pos = pos,
            .elev = elev,
            .concentration = conc,
            .is_combustible = true
        });
    }

    // 明火诱爆判定：当火之沃伦 (Telas)、莫兰斯燃烧弹或蓝火火花触及瓦斯气云时，引发全区域连环云爆
    static bool check_and_detonate_gas(
        Vec2i spark_pos,
        ElevationLevel elev,
        Registry& reg,
        DarujhistanMap& map,
        std::vector<std::string>& out_logs
    ) {
        std::vector<size_t> ignited_indices;
        for (size_t i = 0; i < active_gas_clouds.size(); ++i) {
            auto& cloud = active_gas_clouds[i];
            if (cloud.elev == elev && cloud.pos.manhattan_dist(spark_pos) <= 2 && cloud.concentration >= 30.0f) {
                ignited_indices.push_back(i);
            }
        }

        if (ignited_indices.empty()) return false;

        out_logs.push_back("=================================================================");
        out_logs.push_back("【高压瓦斯云爆轰鸣!】空气中的悬浮蓝火煤气被明火引爆！");
        out_logs.push_back("高压冲击波瞬间充斥整个地下墓穴与管网甬道！");

        // 引爆所有被点燃的瓦斯云
        for (size_t idx : ignited_indices) {
            Vec2i blast_p = active_gas_clouds[idx].pos;
            map.collapse_rooftop(blast_p);

            // 震伤波及范围内的生物
            for (auto [ent, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
                if (!actor.is_alive) continue;
                if (pos.elevation == elev && pos.coord.manhattan_dist(blast_p) <= 2) {
                    actor.take_damage(65, DamageType::MoranthExplosive);
                    out_logs.push_back("    * [" + actor.name + "] 遭受瓦斯云爆冲击！承受 65 点剧烈爆震伤害！(HP: " 
                                       + std::to_string(actor.hp) + ")");
                }
            }
        }

        active_gas_clouds.clear();
        out_logs.push_back("=================================================================");
        return true;
    }
};

} // namespace Malazan
