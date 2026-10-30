#pragma once
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include <iostream>
#include <format>
#include <random>

#include "Components/BossComponent.hpp"

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
        Vec2i target_coord
    ) {
        if (!reg.has<WarrenChannelerComponent>(caster) || !reg.has<ActorComponent>(caster)) {
            return false;
        }

        auto& channeler = reg.get<WarrenChannelerComponent>(caster);
        auto& actor = reg.get<ActorComponent>(caster);

        std::cout << "\n>>> [沃伦施法] " << actor.name << " 开始引导复合术式: 【" << spell_name << "】\n";

        float total_stress = 0.0f;

        // 验证各沃伦亲和度并抽取
        for (const auto& node : weave) {
            float affinity = channeler.get_affinity(node.warren);
            if (affinity <= 0.0f) {
                std::cout << "    [失败] " << actor.name << " 无法感知或触碰未契约的沃伦通道!\n";
                return false;
            }

            // 亲和度越高，转化的自身压力越低
            float efficiency = std::clamp(affinity / 100.0f, 0.2f, 1.0f);
            float stress_cost = node.intensity * (1.2f - efficiency * 0.5f);
            total_stress += stress_cost;

            std::cout << "    * 抽取沃伦 [" << static_cast<int>(node.warren) 
                      << "] 强度: " << node.intensity 
                      << " (亲和度: " << affinity << "%)\n";
        }

        channeler.current_stress += total_stress;
        
        // 全局阿扎斯监测累加（剧烈的法术回响向地脉深处蔓延）
        azath_house_alertness += total_stress * 0.12f;
        std::cout << "    * 施法压力 +" << total_stress 
                  << " (当前身体负荷: " << channeler.current_stress << "/" << channeler.max_safe_stress << ")\n"
                  << "    * 达鲁基斯坦地脉异动: 阿扎斯警觉度上升至 " << azath_house_alertness << "%\n";

        // 反噬过载检定
        if (channeler.current_stress > channeler.max_safe_stress) {
            trigger_overdraw_backfire(caster, actor, channeler);
        }

        // 命中与伤害结算
        resolve_spell_impact(reg, spell_name, weave, target_coord);

        return true;
    }

private:
    static void trigger_overdraw_backfire(Entity ent, ActorComponent& actor, WarrenChannelerComponent& channeler) {
        (void)ent;
        float overflow = channeler.current_stress - channeler.max_safe_stress;
        std::cout << "    [警告!] 沃伦通道过载反噬! 溢出量: " << overflow << "\n";

        static std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        if (dist(rng) < (overflow * 0.03f)) {
            int32_t chaos_dmg = static_cast<int32_t>(overflow * 1.5f);
            std::cout << "    [反噬灾难!] 现实空间被撕裂，混沌能量灼烧法师神经! " 
                      << actor.name << " 承受 " << chaos_dmg << " 点真实混沌伤害!\n";
            actor.take_damage(chaos_dmg, DamageType::ChaosDisruption);
            channeler.chaos_corruption += 15.0f;
        } else {
            std::cout << "    [微度震颤] 脉轮过热，法门暂时出现啸鸣。\n";
        }
    }

    static void resolve_spell_impact(
        Registry& reg, 
        const std::string& spell_name,
        const std::vector<SpellWeaveNode>& weave,
        Vec2i target_coord
    ) {
        (void)spell_name;
        // 查找落在目标坐标处的受害者
        for (auto [target_ent, pos, target_actor] : reg.view<PositionComponent, ActorComponent>()) {
            if (pos.coord == target_coord && target_actor.is_alive) {
                // 根据编织的各沃伦造成对应伤害
                int32_t total_dmg = 0;
                for (const auto& node : weave) {
                    total_dmg += static_cast<int32_t>(node.intensity * 1.2f);
                }

                std::cout << "    => 命中目标 [" << target_actor.name << "] 于坐标 (" 
                          << target_coord.x << ", " << target_coord.y << ")! 造成 " 
                          << total_dmg << " 点综合沃伦杀伤!\n";
                
                if (reg.has<JaghutTyrantComponent>(target_ent)) {
                    auto& tyrant = reg.get<JaghutTyrantComponent>(target_ent);
                    if (tyrant.frost_armor > 0) {
                        tyrant.frost_armor -= total_dmg;
                        std::cout << "       [坚冰护甲吸收] 暴君的 Omtose 冰晶护甲吸收伤害! 护甲剩余: " 
                                  << std::max(0, tyrant.frost_armor) << "\n";
                        if (tyrant.frost_armor <= 0) {
                            std::cout << "       【暴君护甲爆碎!】雷斯特发出惊天怒吼，本体暴露在炽热蓝火之下!\n";
                        }
                    } else {
                        target_actor.take_damage(total_dmg, DamageType::WarrenFire);
                    }
                } else {
                    target_actor.take_damage(total_dmg, DamageType::WarrenFire);
                }
                std::cout << "       [" << target_actor.name << "] 剩余生命值: " << target_actor.hp << "\n";
            }
        }
    }
};

} // namespace Malazan
