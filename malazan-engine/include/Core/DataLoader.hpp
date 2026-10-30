#pragma once
#include "Core/Types.hpp"
#include "Core/ECS.hpp"
#include "Components/Components.hpp"
#include "Components/AIComponent.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/OtataralComponent.hpp"
#include "Items/Item.hpp"
#include "Core/json.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Malazan {

using json = nlohmann::json;

inline WarrenType parse_warren_type(const std::string& str) {
    if (str == "Telas") return WarrenType::Telas;
    if (str == "Rashan") return WarrenType::Rashan;
    if (str == "Meanas") return WarrenType::Meanas;
    if (str == "Denul") return WarrenType::Denul;
    if (str == "Thyr") return WarrenType::Thyr;
    if (str == "Tellann") return WarrenType::Tellann;
    if (str == "Driss") return WarrenType::Driss;
    if (str == "KuraldGalain") return WarrenType::KuraldGalain;
    if (str == "OmtosePhellack") return WarrenType::OmtosePhellack;
    return WarrenType::Chaos;
}

inline Faction parse_faction(const std::string& str) {
    if (str == "Bridgeburners") return Faction::Bridgeburners;
    if (str == "ImperialClaw") return Faction::ImperialClaw;
    if (str == "CityAssassinsGuild") return Faction::CityAssassinsGuild;
    if (str == "TisteAndii") return Faction::TisteAndii;
    if (str == "PhoenixInnCircle") return Faction::PhoenixInnCircle;
    return Faction::NeutralCivilians;
}

struct SpellDefinition {
    std::string id;
    std::string name;
    std::string description;
    int32_t cooldown{1};
    int32_t range{5};
    int32_t damage{0};
    int32_t heal{0};
    std::vector<std::pair<WarrenType, float>> weave;
};

class DataLoader {
public:
    static std::unordered_map<std::string, SpellDefinition> load_spells(const std::string& filepath) {
        std::unordered_map<std::string, SpellDefinition> result;
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[DataLoader] 错误: 无法打开法术数据文件: " << filepath << "\n";
            return result;
        }

        try {
            json data = json::parse(file);
            for (const auto& item : data["spells"]) {
                SpellDefinition spell;
                spell.id = item["id"].get<std::string>();
                spell.name = item["name"].get<std::string>();
                spell.description = item.value("description", "");
                spell.cooldown = item.value("cooldown", 1);
                spell.range = item.value("range", 5);
                spell.damage = item.value("damage", 0);
                spell.heal = item.value("heal", 0);

                if (item.contains("weave")) {
                    for (const auto& w : item["weave"]) {
                        WarrenType wt = parse_warren_type(w["warren"].get<std::string>());
                        float intensity = w["intensity"].get<float>();
                        spell.weave.emplace_back(wt, intensity);
                    }
                }
                result[spell.id] = std::move(spell);
            }
            std::cout << "[DataLoader] 成功加载 " << result.size() << " 个沃伦法术模板 (来自 " << filepath << ")\n";
        } catch (const std::exception& e) {
            std::cerr << "[DataLoader] JSON 解析失败: " << e.what() << "\n";
        }

        return result;
    }

    static Entity spawn_actor_from_json(
        Registry& reg, 
        const std::string& filepath, 
        const std::string& template_id,
        Vec2i spawn_pos,
        ElevationLevel elev
    ) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[DataLoader] 无法打开角色模板文件: " << filepath << "\n";
            return NULL_ENTITY;
        }

        try {
            json data = json::parse(file);
            for (const auto& t : data["actor_templates"]) {
                if (t["id"].get<std::string>() == template_id) {
                    Entity ent = reg.create_entity();

                    // ActorComponent
                    reg.emplace<ActorComponent>(ent, ActorComponent{
                        .name = t["name"].get<std::string>(),
                        .faction = parse_faction(t.value("faction", "NeutralCivilians")),
                        .hp = t.value("hp", 100),
                        .max_hp = t.value("max_hp", 100)
                    });

                    // PositionComponent
                    reg.emplace<PositionComponent>(ent, PositionComponent{
                        .coord = spawn_pos,
                        .elevation = elev,
                        .can_climb_roofs = t.value("can_climb_roofs", false),
                        .rooftop_jump_range = t.value("rooftop_jump_range", 2)
                    });

                    // TurnEnergyComponent
                    reg.emplace<TurnEnergyComponent>(ent, TurnEnergyComponent{
                        .energy = 0,
                        .speed = t.value("speed", 1000)
                    });

                    // WarrenChannelerComponent
                    if (t.contains("warren_affinities")) {
                        WarrenChannelerComponent channeler;
                        channeler.max_safe_stress = t.value("max_safe_stress", 100.0f);
                        for (auto& [w_name, val] : t["warren_affinities"].items()) {
                            channeler.affinities[parse_warren_type(w_name)] = val.get<float>();
                        }
                        reg.emplace<WarrenChannelerComponent>(ent, std::move(channeler));
                    }

                    // StealthComponent
                    if (t.contains("stealth_power")) {
                        reg.emplace<StealthComponent>(ent, StealthComponent{
                            .in_stealth = true,
                            .stealth_power = t["stealth_power"].get<float>()
                        });
                    }

                    // MoranthMunitionComponent
                    if (t.contains("munitions")) {
                        MoranthMunitionComponent munitions;
                        const auto& m = t["munitions"];
                        munitions.ammo[MoranthMunitionType::Sharper] = m.value("Sharper", 0);
                        munitions.ammo[MoranthMunitionType::Burner] = m.value("Burner", 0);
                        munitions.ammo[MoranthMunitionType::Smoker] = m.value("Smoker", 0);
                        munitions.ammo[MoranthMunitionType::Quencher] = m.value("Quencher", 0);
                        reg.emplace<MoranthMunitionComponent>(ent, std::move(munitions));
                    }

                    // AIComponent
                    if (t.contains("ai")) {
                        AIComponent aic;
                        const auto& a = t["ai"];
                        aic.sight_range = a.value("sight_range", 6);
                        aic.hearing_range = a.value("hearing_range", 8);
                        aic.attack_range = a.value("attack_range", 2);
                        aic.attack_power = a.value("attack_power", 28);
                        if (a.contains("patrol_waypoints")) {
                            for (const auto& wp : a["patrol_waypoints"]) {
                                aic.patrol_waypoints.push_back({wp[0].get<int32_t>(), wp[1].get<int32_t>()});
                            }
                        }
                        reg.emplace<AIComponent>(ent, std::move(aic));
                    }

                    reg.emplace<TacticalElevationComponent>(ent, TacticalElevationComponent{});

                    // 默认专属战术装备库加载
                    if (template_id == "kalam_mekhar") {
                        InventoryComponent inv;
                        inv.equipped_main_hand = ItemFactory::create_otataral_dagger();
                        inv.equipped_off_hand = ItemFactory::create_bridgeburner_blade();
                        inv.add_item(ItemFactory::create_moranth_burner());
                        inv.add_item(ItemFactory::create_moranth_sharper());
                        inv.add_item(ItemFactory::create_moranth_cusser());
                        inv.add_item(ItemFactory::create_denul_draught());
                        reg.emplace<InventoryComponent>(ent, std::move(inv));

                        reg.emplace<OtataralComponent>(ent, OtataralComponent{
                            .radius = 3.5f,
                            .deaden_intensity = 1.0f,
                            .is_active = true
                        });
                    } else if (template_id == "quick_ben") {
                        InventoryComponent inv;
                        inv.add_item(ItemFactory::create_denul_draught());
                        reg.emplace<InventoryComponent>(ent, std::move(inv));
                    }

                    return ent;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "[DataLoader] 加载角色模板失败: " << e.what() << "\n";
        }

        return NULL_ENTITY;
    }
};

} // namespace Malazan
