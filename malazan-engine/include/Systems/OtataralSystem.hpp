#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/OtataralComponent.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/BossComponent.hpp"
#include <vector>
#include <string>
#include <cmath>

namespace Malazan {

struct ActiveOtataralField {
    Vec2i center;
    ElevationLevel elevation;
    float radius;
    float intensity;
    Entity source_entity;
};

class OtataralSystem {
public:
    // 获取当前世界中所有激活的奥塔拉尔反魔力场
    static std::vector<ActiveOtataralField> get_active_fields(Registry& reg) {
        std::vector<ActiveOtataralField> fields;

        // 1. 检查拥有 OtataralComponent 的实体
        for (auto [ent, pos, ota] : reg.view<PositionComponent, OtataralComponent>()) {
            if (ota.is_active) {
                fields.push_back(ActiveOtataralField{
                    .center = pos.coord,
                    .elevation = pos.elevation,
                    .radius = ota.radius,
                    .intensity = ota.deaden_intensity,
                    .source_entity = ent
                });
            }
        }

        // 2. 检查装备了奥塔拉尔武器的实体（如卡拉姆装备了奥塔拉尔匕首）
        for (auto [ent, pos, inv] : reg.view<PositionComponent, InventoryComponent>()) {
            if (inv.has_otataral_equipped()) {
                float r = inv.get_max_otataral_radius();
                fields.push_back(ActiveOtataralField{
                    .center = pos.coord,
                    .elevation = pos.elevation,
                    .radius = r,
                    .intensity = 1.0f,
                    .source_entity = ent
                });
            }
        }

        return fields;
    }

    // 判定特定坐标是否处于任何奥塔拉尔力场的压制范围内
    static bool is_suppressed_at(
        Vec2i coord,
        ElevationLevel elev,
        Registry& reg,
        float* out_intensity = nullptr
    ) {
        auto fields = get_active_fields(reg);
        for (const auto& f : fields) {
            float dx = static_cast<float>(coord.x - f.center.x);
            float dy = static_cast<float>(coord.y - f.center.y);
            float dist = std::sqrt(dx * dx + dy * dy);

            // 高度差衰减：同层级完全覆盖，跨层级压制半衰
            float effective_radius = f.radius;
            if (elev != f.elevation) {
                effective_radius *= 0.7f;
            }

            if (dist <= effective_radius) {
                if (out_intensity) {
                    *out_intensity = f.intensity * (1.0f - (dist / effective_radius) * 0.3f);
                }
                return true;
            }
        }
        return false;
    }

    // 沃伦施法绝魔检定：施法者或目标处在力场中时，法术将遭生锈赭红死域强行熄灭并引发混沌震荡
    static bool check_and_suppress_spell(
        Entity caster,
        Vec2i target_coord,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<PositionComponent>(caster)) return false;
        auto& caster_pos = reg.get<PositionComponent>(caster);

        float intensity = 0.0f;
        bool caster_suppressed = is_suppressed_at(caster_pos.coord, caster_pos.elevation, reg, &intensity);
        bool target_suppressed = is_suppressed_at(target_coord, caster_pos.elevation, reg, &intensity);

        if (caster_suppressed || target_suppressed) {
            std::string caster_name = reg.has<ActorComponent>(caster) 
                                      ? reg.get<ActorComponent>(caster).name 
                                      : "施法者";
            
            out_logs.push_back("【奥塔拉尔死域扼杀!】空气中泛起刺鼻铁锈气息与赭红矿粉!");
            out_logs.push_back("[" + caster_name + "] 构筑的沃伦法门通道瞬间崩塌！法力被反魔力场吞噬殆尽！");

            // 施法者承受混沌反噬震荡
            if (reg.has<WarrenChannelerComponent>(caster)) {
                auto& chan = reg.get<WarrenChannelerComponent>(caster);
                chan.current_stress += 40.0f;
                chan.chaos_corruption += 25.0f;
                out_logs.push_back("[" + caster_name + "] 遭受反噬重创！法术压力飙升 +40%，混沌污染 +25%！");
            }

            if (reg.has<ActorComponent>(caster)) {
                reg.get<ActorComponent>(caster).take_damage(18, DamageType::ChaosDisruption);
            }

            return true; // 成功阻断了法术
        }

        return false;
    }

    // 破魔实战：卡拉姆以奥塔拉尔匕首近战刺杀时，直接穿透并瓦解贾格特暴君坚冰护甲与敌方魔法结界
    static void apply_otataral_melee_bonus(
        Entity attacker,
        Entity target,
        int32_t& in_out_damage,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        bool has_otataral = false;
        if (reg.has<OtataralComponent>(attacker)) {
            has_otataral = reg.get<OtataralComponent>(attacker).is_active;
        }
        if (reg.has<InventoryComponent>(attacker)) {
            has_otataral = has_otataral || reg.get<InventoryComponent>(attacker).has_otataral_equipped();
        }

        if (!has_otataral) return;

        // 若目标为贾格特暴君且具有上古坚冰护甲 (Frost Armor)
        if (reg.has<JaghutTyrantComponent>(target)) {
            auto& tyrant = reg.get<JaghutTyrantComponent>(target);
            if (tyrant.frost_armor > 0) {
                int32_t shatter_amt = std::min(tyrant.frost_armor, 35);
                tyrant.frost_armor -= shatter_amt;
                in_out_damage += 25; // 破盾追加额外暴击真实物理破甲伤害

                out_logs.push_back("【奥塔拉尔断魔绝杀!】锈红锋刃无视 Omtose Phellack 太古坚冰护甲！");
                out_logs.push_back("暴君坚冰护甲粉碎 -" + std::to_string(shatter_amt) + " 点！剩余护甲: " + std::to_string(tyrant.frost_armor));
            }
        }
    }
};

} // namespace Malazan
