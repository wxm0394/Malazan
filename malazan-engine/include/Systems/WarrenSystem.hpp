#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "Components/BossComponent.hpp"
#include "Systems/OtataralSystem.hpp"
#include <iostream>
#include <format>
#include <random>
#include <vector>
#include <string>

namespace Malazan {

struct SpellWeaveNode {
    WarrenType warren;
    float intensity; // 抽取能量强度
};

class WarrenSystem {
public:
    static inline float azath_house_alertness{0.0f}; // 达鲁基斯坦地底阿扎斯之屋沉睡/苏醒指数 (0~100)

    // 复合沃伦编织施法（快班 Quick Ben 的招牌绝技）
    static bool cast_multi_warren_spell(
        Entity caster,
        Registry& reg,
        const std::string& spell_name,
        const std::vector<SpellWeaveNode>& weave,
        Vec2i target_coord,
        std::vector<std::string>* out_logs = nullptr
    ) {
        if (!reg.has<WarrenChannelerComponent>(caster) || !reg.has<ActorComponent>(caster)) {
            return false;
        }

        auto& channeler = reg.get<WarrenChannelerComponent>(caster);
        auto& actor = reg.get<ActorComponent>(caster);

        auto log_msg = [&](const std::string& s) {
            std::cout << s << "\n";
            if (out_logs) out_logs->push_back(s);
        };

        log_msg(">>> [沃伦施法] " + actor.name + " 开始引导复合术式: 【" + spell_name + "】");

        // 0. 奥塔拉尔反魔力场压制与扼杀检定
        std::vector<std::string> ota_logs;
        if (OtataralSystem::check_and_suppress_spell(caster, target_coord, reg, ota_logs)) {
            for (const auto& l : ota_logs) log_msg("  " + l);
            return false; // 法门通道被完全扑灭
        }

        float total_stress = 0.0f;

        // 验证各沃伦亲和度并抽取
        for (const auto& node : weave) {
            float affinity = channeler.get_affinity(node.warren);
            if (affinity <= 0.0f) {
                log_msg("    [失败] " + actor.name + " 无法感知或触碰未契约的沃伦通道!");
                return false;
            }

            // 亲和度越高，转化的自身压力越低
            float efficiency = std::clamp(affinity / 100.0f, 0.2f, 1.0f);
            float stress_cost = node.intensity * (1.2f - efficiency * 0.5f);
            total_stress += stress_cost;

            log_msg("    * 抽取沃伦通道强度: " + std::to_string(node.intensity).substr(0, 4) 
                    + " (亲和度: " + std::to_string(affinity).substr(0, 4) + "%)");
        }

        channeler.current_stress += total_stress;
        
        // 全局阿扎斯监测累加（剧烈的法术回响向地脉深处蔓延）
        azath_house_alertness += total_stress * 0.12f;
        log_msg("    * 施法压力 +" + std::to_string(total_stress).substr(0, 4) 
                + " (当前负荷: " + std::to_string(channeler.current_stress).substr(0, 4) + "/" 
                + std::to_string(channeler.max_safe_stress).substr(0, 4) + ")");
        log_msg("    * 达鲁基斯坦地脉异动: 阿扎斯警觉度上升至 " + std::to_string(azath_house_alertness).substr(0, 4) + "%");

        // 反噬过载检定
        if (channeler.current_stress > channeler.max_safe_stress) {
            trigger_overdraw_backfire(caster, actor, channeler, out_logs);
        }

        // 命中与伤害结算
        resolve_spell_impact(reg, spell_name, weave, target_coord, out_logs);

        return true;
    }

private:
    static void trigger_overdraw_backfire(
        Entity ent, 
        ActorComponent& actor, 
        WarrenChannelerComponent& channeler,
        std::vector<std::string>* out_logs
    ) {
        (void)ent;
        float overflow = channeler.current_stress - channeler.max_safe_stress;
        auto log_msg = [&](const std::string& s) {
            std::cout << s << "\n";
            if (out_logs) out_logs->push_back(s);
        };

        log_msg("    [警告!] 沃伦通道过载反噬! 溢出量: " + std::to_string(overflow).substr(0, 4));

        static std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        if (dist(rng) < (overflow * 0.03f)) {
            int32_t chaos_dmg = static_cast<int32_t>(overflow * 1.5f);
            log_msg("    [反噬灾难!] 现实空间被撕裂，混沌能量灼烧法师神经! " 
                    + actor.name + " 承受 " + std::to_string(chaos_dmg) + " 点真实混沌伤害!");
            actor.take_damage(chaos_dmg, DamageType::ChaosDisruption);
            channeler.chaos_corruption += 15.0f;
        } else {
            log_msg("    [微度震颤] 脉轮过热，法门暂时出现啸鸣。");
        }
    }

    static void resolve_spell_impact(
        Registry& reg, 
        const std::string& spell_name,
        const std::vector<SpellWeaveNode>& weave,
        Vec2i target_coord,
        std::vector<std::string>* out_logs
    ) {
        (void)spell_name;
        auto log_msg = [&](const std::string& s) {
            std::cout << s << "\n";
            if (out_logs) out_logs->push_back(s);
        };

        // 查找落在目标坐标处的受害者
        for (auto [target_ent, pos, target_actor] : reg.view<PositionComponent, ActorComponent>()) {
            if (pos.coord == target_coord && target_actor.is_alive) {
                // 根据编织的各沃伦造成对应伤害
                int32_t total_dmg = 0;
                for (const auto& node : weave) {
                    total_dmg += static_cast<int32_t>(node.intensity * 1.2f);
                }

                log_msg("    => 命中目标 [" + target_actor.name + "] 于坐标 (" 
                        + std::to_string(target_coord.x) + ", " + std::to_string(target_coord.y) + ")! 造成 " 
                        + std::to_string(total_dmg) + " 点综合沃伦杀伤!");
                
                if (reg.has<JaghutTyrantComponent>(target_ent)) {
                    auto& tyrant = reg.get<JaghutTyrantComponent>(target_ent);
                    if (tyrant.frost_armor > 0) {
                        tyrant.frost_armor -= total_dmg;
                        log_msg("       [坚冰护甲吸收] 暴君的 Omtose 冰晶护甲吸收伤害! 护甲剩余: " 
                                + std::to_string(std::max(0, tyrant.frost_armor)));
                        if (tyrant.frost_armor <= 0) {
                            log_msg("       【暴君护甲爆碎!】雷斯特发出惊天怒吼，本体暴露在炽热蓝火之下!");
                        }
                    } else {
                        target_actor.take_damage(total_dmg, DamageType::WarrenFire);
                    }
                } else {
                    target_actor.take_damage(total_dmg, DamageType::WarrenFire);
                }
                log_msg("       [" + target_actor.name + "] 剩余生命值: " + std::to_string(target_actor.hp));
            }
        }
    }
};

} // namespace Malazan
