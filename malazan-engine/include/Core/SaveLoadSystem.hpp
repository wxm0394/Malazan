#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Core/json.hpp"
#include "Components/Components.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/OtataralComponent.hpp"
#include "Components/DragnipurComponent.hpp"
#include "Components/FactionReputationComponent.hpp"
#include "Components/BossComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include "Systems/TurnSystem.hpp"
#include "Systems/WarrenSystem.hpp"
#include "Story/StorylineSystem.hpp"
#include "Systems/DeckOfDragonsSystem.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

namespace Malazan {

using json = nlohmann::json;

class SaveLoadSystem {
public:
    static bool save_game(
        const std::string& filepath,
        Registry& reg,
        const DarujhistanMap& map,
        Entity player,
        std::vector<std::string>& out_logs
    ) {
        try {
            // 确保保存目录存在
            std::filesystem::path p(filepath);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }

            json root;
            root["engine_version"] = "Malazan-0.3.0-Phase3";
            root["save_timestamp"] = "2026-10-31";
            root["global_tick"] = TurnSystem::global_tick_count;
            root["current_act"] = static_cast<uint8_t>(StorylineSystem::current_act);
            root["azath_alertness"] = WarrenSystem::azath_house_alertness;
            root["active_tarot_card"] = DeckOfDragonsSystem::active_card.title;

            // 1. 玩家专属数据保存
            if (reg.has<ActorComponent>(player) && reg.has<PositionComponent>(player)) {
                auto& p_actor = reg.get<ActorComponent>(player);
                auto& p_pos = reg.get<PositionComponent>(player);

                json p_json;
                p_json["name"] = p_actor.name;
                p_json["hp"] = p_actor.hp;
                p_json["max_hp"] = p_actor.max_hp;
                p_json["is_alive"] = p_actor.is_alive;
                p_json["x"] = p_pos.coord.x;
                p_json["y"] = p_pos.coord.y;
                p_json["elevation"] = static_cast<uint8_t>(p_pos.elevation);
                p_json["rooftop_jump_range"] = p_pos.rooftop_jump_range;

                // 背包与装备保存
                if (reg.has<InventoryComponent>(player)) {
                    auto& inv = reg.get<InventoryComponent>(player);
                    if (inv.equipped_main_hand.has_value()) {
                        p_json["equipped_main_hand"] = inv.equipped_main_hand->id;
                    }
                    if (inv.equipped_off_hand.has_value()) {
                        p_json["equipped_off_hand"] = inv.equipped_off_hand->id;
                    }
                    json backpack_arr = json::array();
                    for (const auto& item : inv.backpack) {
                        backpack_arr.push_back({
                            {"id", item.id},
                            {"name", item.name},
                            {"charges", item.charges}
                        });
                    }
                    p_json["backpack"] = backpack_arr;
                }

                // 阵营声望保存
                if (reg.has<FactionReputationComponent>(player)) {
                    auto& rep = reg.get<FactionReputationComponent>(player);
                    json rep_json;
                    for (auto& [fac, val] : rep.standing) {
                        rep_json[std::to_string(static_cast<uint8_t>(fac))] = val;
                    }
                    p_json["faction_standing"] = rep_json;
                }

                root["player"] = p_json;
            }

            // 2. 德拉格尼普尔黑剑世界状态保存
            for (auto [ent, drag, actor] : reg.view<DragnipurComponent, ActorComponent>()) {
                (void)ent;
                json drag_json;
                drag_json["bearer"] = actor.name;
                drag_json["wagon_weight"] = drag.wagon_weight;
                drag_json["chaos_distance"] = drag.chaos_pursuit_distance;
                json souls_arr = json::array();
                for (const auto& s : drag.trapped_souls) {
                    souls_arr.push_back({
                        {"name", s.name},
                        {"combat_power", s.combat_power},
                        {"pull_effort", s.pull_effort}
                    });
                }
                drag_json["trapped_souls"] = souls_arr;
                root["dragnipur"] = drag_json;
                break;
            }

            // 3. 地图状态保存 (坍塌的屋顶等动态修改)
            json collapsed_arr = json::array();
            for (int32_t y = 0; y < map.height(); ++y) {
                for (int32_t x = 0; x < map.width(); ++x) {
                    Vec2i p_tile{x, y};
                    const auto& r_cell = map.get_cell(p_tile, ElevationLevel::Rooftop);
                    if (r_cell.roof_collapsed) {
                        collapsed_arr.push_back({x, y});
                    }
                }
            }
            root["map_collapsed_roofs"] = collapsed_arr;

            // 写入磁盘文件
            std::ofstream out_file(filepath);
            if (!out_file.is_open()) {
                out_logs.push_back("[存档失败] 无法创建存档文件: " + filepath);
                return false;
            }
            out_file << root.dump(2);
            out_logs.push_back("=================================================================");
            out_logs.push_back("【世界存档成功】进度已安全写入: " + filepath);
            out_logs.push_back("当前世界时间片: Tick " + std::to_string(TurnSystem::global_tick_count)
                               + " | 剧幕: " + StorylineSystem::get_act_title());
            out_logs.push_back("=================================================================");
            return true;
        } catch (const std::exception& e) {
            out_logs.push_back(std::string("[存档异常] ") + e.what());
            return false;
        }
    }

    static bool load_game(
        const std::string& filepath,
        Registry& reg,
        DarujhistanMap& map,
        Entity& player,
        std::vector<std::string>& out_logs
    ) {
        try {
            std::ifstream in_file(filepath);
            if (!in_file.is_open()) {
                out_logs.push_back("[读档失败] 未找到存档文件: " + filepath);
                return false;
            }

            json root = json::parse(in_file);

            // 恢复全局状态
            TurnSystem::global_tick_count = root.value("global_tick", 0);
            StorylineSystem::current_act = static_cast<StoryAct>(root.value("current_act", 1));
            WarrenSystem::azath_house_alertness = root.value("azath_alertness", 0.0f);

            // 恢复玩家状态
            if (root.contains("player") && reg.has<ActorComponent>(player)) {
                const auto& p_json = root["player"];
                auto& p_actor = reg.get<ActorComponent>(player);
                auto& p_pos = reg.get<PositionComponent>(player);

                p_actor.hp = p_json.value("hp", 100);
                p_actor.max_hp = p_json.value("max_hp", 100);
                p_actor.is_alive = p_json.value("is_alive", true);
                p_pos.coord = {p_json.value("x", 6), p_json.value("y", 6)};
                p_pos.elevation = static_cast<ElevationLevel>(p_json.value("elevation", 2));
                p_pos.rooftop_jump_range = p_json.value("rooftop_jump_range", 3);

                if (p_json.contains("faction_standing") && reg.has<FactionReputationComponent>(player)) {
                    auto& rep = reg.get<FactionReputationComponent>(player);
                    for (auto& [k, v] : p_json["faction_standing"].items()) {
                        Faction f = static_cast<Faction>(std::stoi(k));
                        rep.standing[f] = v.get<int32_t>();
                    }
                }
            }

            // 恢复德拉格尼普尔状态
            if (root.contains("dragnipur")) {
                const auto& d_json = root["dragnipur"];
                for (auto [ent, drag, actor] : reg.view<DragnipurComponent, ActorComponent>()) {
                    (void)ent;
                    drag.wagon_weight = d_json.value("wagon_weight", 1200.0f);
                    drag.chaos_pursuit_distance = d_json.value("chaos_distance", 85.0f);
                    if (d_json.contains("trapped_souls")) {
                        drag.trapped_souls.clear();
                        for (const auto& s : d_json["trapped_souls"]) {
                            drag.trapped_souls.push_back(TrappedSoul{
                                .name = s.value("name", "Unknown Soul"),
                                .former_faction = Faction::NeutralCivilians,
                                .combat_power = s.value("combat_power", 100),
                                .pull_effort = s.value("pull_effort", 1.0f)
                            });
                        }
                    }
                    break;
                }
            }

            // 恢复地图坍塌瓦砾
            if (root.contains("map_collapsed_roofs")) {
                for (const auto& c_pt : root["map_collapsed_roofs"]) {
                    Vec2i p_tile{c_pt[0].get<int32_t>(), c_pt[1].get<int32_t>()};
                    map.collapse_rooftop(p_tile);
                }
            }

            out_logs.push_back("=================================================================");
            out_logs.push_back("【世界读档成功】已成功恢复达鲁基斯坦战术局势!");
            out_logs.push_back("当前主线: " + StorylineSystem::get_act_title() 
                               + " | 玩家 HP: " + std::to_string(reg.get<ActorComponent>(player).hp)
                               + " | 恢复世界时间片: Tick " + std::to_string(TurnSystem::global_tick_count));
            out_logs.push_back("=================================================================");
            return true;
        } catch (const std::exception& e) {
            out_logs.push_back(std::string("[读档异常] ") + e.what());
            return false;
        }
    }
};

} // namespace Malazan
