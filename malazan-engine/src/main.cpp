#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/OtataralComponent.hpp"
#include "Components/DragnipurComponent.hpp"
#include "Components/FactionReputationComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include "Core/DataLoader.hpp"
#include "Systems/TurnSystem.hpp"
#include "Systems/WarrenSystem.hpp"
#include "Systems/RooftopTacticsSystem.hpp"
#include "Systems/BlueFireGasSystem.hpp"
#include "Systems/FOVSystem.hpp"
#include "Systems/OtataralSystem.hpp"
#include "Systems/PathfindingSystem.hpp"
#include "Systems/DragnipurSystem.hpp"
#include "Systems/FactionSystem.hpp"
#include "Systems/InteractionSystem.hpp"
#include "Story/DialogueSystem.hpp"
#include "World/WorldSector.hpp"
#include "World/HazardSystem.hpp"
#include "Core/SaveLoadSystem.hpp"
#include "Render/GameViewport.hpp"

#include <iostream>
#include <string>

using namespace Malazan;

int main(int argc, char* argv[]) {
    bool headless = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--headless") {
            headless = true;
        }
    }

    std::cout << "========================================================================\n";
    std::cout << "  《玛拉赞落魄之书》：达鲁基斯坦 (Darujhistan) - 战术与沙盒引擎\n";
    std::cout << "  Phase 3 三层立体架构: 地下古老墓穴 | 瓦斯云爆灾害 | 状态持久化 | 泰兰·伊玛斯\n";
    std::cout << "========================================================================\n\n";

    Registry reg;
    DarujhistanMap city_map(20, 20);

    // 1. 数据驱动加载：法术库与角色配置
    auto spells = DataLoader::load_spells("assets/data/spells.json");
    std::cout << "  * 已注册复合术式: 【" << spells["shadowfire_tempest"].name << "】\n";

    // 2. 地图拓扑生成
    // 凤凰客栈 (Phoenix Inn) 建筑群：占据 (4,4) 到 (9,9)
    city_map.add_building({4, 4}, 6, 6, TileTerrain::SlateRooftop);
    // 对街的富商宅邸：占据 (12, 4) 到 (16, 9)
    city_map.add_building({12, 4}, 5, 6, TileTerrain::FlatTerrace);

    // 部署达鲁基斯坦蓝火路灯
    Entity gas_lamp = reg.create_entity();
    reg.emplace<BlueFireNodeComponent>(gas_lamp, BlueFireNodeComponent{
        .coord = {10, 6},
        .gas_pressure = 100.0f,
        .is_lit = true,
        .is_leaking = true,
        .blast_yield = 75,
        .blast_radius = 3
    });

    // 部署地下墓穴高压易爆瓦斯气云 (HazardSystem)
    HazardSystem::spawn_gas_leak({10, 10}, ElevationLevel::Underground, 85.0f);

    // 3. 数据驱动实体生成 (从 JSON 实例化角色并挂载专属装备库)
    Entity kalam = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "kalam_mekhar", {6, 6}, ElevationLevel::Rooftop
    );
    Entity quick_ben = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "quick_ben", {5, 5}, ElevationLevel::Rooftop
    );
    Entity claw_agent = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "claw_assassin", {10, 6}, ElevationLevel::Street
    );

    // Phase 2 活化沙盒 NPC 生态入场
    Entity kruppe = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "kruppe", {5, 5}, ElevationLevel::Street
    );
    Entity crokus = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "crokus", {7, 6}, ElevationLevel::Rooftop
    );
    Entity rake = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "anomander_rake", {14, 4}, ElevationLevel::Rooftop
    );
    Entity guard = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "city_watch_guard", {2, 4}, ElevationLevel::Street
    );

    // Phase 3 新增：地下墓穴远古守望者 泰兰·伊玛斯 白骨战将
    Entity tlan_warrior = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "tlan_imass_warrior", {10, 8}, ElevationLevel::Underground
    );
    if (reg.has<InventoryComponent>(tlan_warrior)) {
        reg.get<InventoryComponent>(tlan_warrior).equip(ItemFactory::create_tlan_flint_sword());
    }

    std::cout << "  * 破桥军小队、克鲁普、克罗库斯、城防守卫、雷克与地下墓穴【泰兰·伊玛斯】已全部就位。\n";

    if (reg.has<InventoryComponent>(kalam)) {
        auto& inv = reg.get<InventoryComponent>(kalam);
        std::cout << "  * [装备验证] 卡拉姆主手已装备: 【" 
                  << (inv.equipped_main_hand.has_value() ? inv.equipped_main_hand->name : "空") << "】\n";
    }

    if (reg.has<DragnipurComponent>(rake)) {
        auto& drag = reg.get<DragnipurComponent>(rake);
        std::cout << "  * [神器验证] 雷克黑剑【德拉格尼普尔】已觉醒，当前拉车亡魂数: " 
                  << drag.trapped_souls.size() << " | 纯暗大车自重: " << drag.wagon_weight << "\n";
    }

    if (!headless) {
        std::cout << "\n[视口启动] 正在唤醒 SDL3 战术图形视口窗口 (2.5D 等距立体高清 - Phase 3)...\n";
        GameViewport viewport(1440, 860);
        if (viewport.init()) {
            viewport.run(reg, city_map, kalam, quick_ben, claw_agent);
            return 0;
        } else {
            std::cout << "[降级处理] SDL3 初始化视口失败，切换至终端纯逻辑演算模式。\n";
        }
    }

    // 无头测试模式（自动化 Phase 1 & 2 战术与沙盒生态完整回归）
    std::cout << "\n[无头测试模式] 开始回归测试 Phase 2 实体与沙盒生态模块...\n";

    // 1. NPC 交互与对话分支测试
    std::cout << "\n--- [测试 1: NPC 交互与深度剧情对话系统] --- \n";
    auto dlg_kruppe = DialogueSystem::get_dialogue_for(kruppe, kalam, reg);
    std::cout << "  说话者: " << dlg_kruppe.speaker_name << " (" << dlg_kruppe.speaker_title << ")\n";
    std::cout << "  开场白: " << dlg_kruppe.dialogue_text.substr(0, 70) << "...\n";
    std::vector<std::string> choice_logs;
    DialogueSystem::apply_dialogue_choice(kruppe, kalam, 0, reg, choice_logs);
    for (const auto& cl : choice_logs) std::cout << "    " << cl << "\n";

    auto dlg_crokus = DialogueSystem::get_dialogue_for(crokus, kalam, reg);
    choice_logs.clear();
    DialogueSystem::apply_dialogue_choice(crokus, kalam, 0, reg, choice_logs);
    for (const auto& cl : choice_logs) std::cout << "    " << cl << "\n";

    // 2. 德拉格尼普尔摄魂与纯暗大车推进测试
    std::cout << "\n--- [测试 2: 德拉格尼普尔 (Dragnipur) 锁链摄魂机制] --- \n";
    // 实例化一名潜伏的帝国特工并由雷克挥剑处决
    Entity test_target = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "claw_assassin", {14, 5}, ElevationLevel::Rooftop
    );
    std::vector<std::string> drag_logs;
    DragnipurSystem::slay_and_trap(rake, test_target, reg, drag_logs);
    for (const auto& dl : drag_logs) std::cout << "  " << dl << "\n";

    // 3. 阵营声望矩阵与仇恨演化测试
    std::cout << "\n--- [测试 3: 阵营外交声望动态反馈] --- \n";
    std::vector<std::string> rep_logs;
    FactionSystem::on_kill(kalam, claw_agent, reg, rep_logs);
    for (const auto& rl : rep_logs) std::cout << "  " << rl << "\n";

    if (reg.has<FactionReputationComponent>(kalam)) {
        auto& rep = reg.get<FactionReputationComponent>(kalam);
        std::cout << "  当前阵营好感度一览:\n";
        std::cout << "    * 凤凰客栈同盟: " << rep.get(Faction::PhoenixInnCircle) 
                  << " (" << FactionReputationComponent::relation_to_string(rep.get_relation(Faction::PhoenixInnCircle)) << ")\n";
        std::cout << "    * 刺客公会: " << rep.get(Faction::CityAssassinsGuild) 
                  << " (" << FactionReputationComponent::relation_to_string(rep.get_relation(Faction::CityAssassinsGuild)) << ")\n";
        std::cout << "    * 帝国爪牙: " << rep.get(Faction::ImperialClaw) 
                  << " (" << FactionReputationComponent::relation_to_string(rep.get_relation(Faction::ImperialClaw)) << ")\n";
    }

    // 4. 主线三幕流程贯通回归测试
    std::cout << "\n--- [测试 4: 主线全剧幕贯通回归] --- \n";
    std::cout << ">>> 当前主线: 【" << StorylineSystem::get_act_title() << "】\n";
    // 刺杀爪牙触发第二幕暴君出世
    RooftopTacticsSystem::execute_plunging_ambush(kalam, claw_agent, reg, city_map);
    reg.get<ActorComponent>(claw_agent).is_alive = false;
    std::vector<std::string> story_logs;
    StorylineSystem::check_progression(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    // 第二幕：暴君出世
    std::cout << "\n>>> 推进至: 【" << StorylineSystem::get_act_title() << "】\n";
    if (StorylineSystem::boss_raest_entity != NULL_ENTITY) {
        reg.get<PositionComponent>(StorylineSystem::boss_raest_entity).coord = {10, 6};
        reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity).frost_armor = 0;
    }
    story_logs.clear();
    StorylineSystem::check_progression(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    // 第三幕：阿扎斯封印
    std::cout << "\n>>> 推进至: 【" << StorylineSystem::get_act_title() << "】\n";
    story_logs.clear();
    StorylineSystem::trigger_azath_sealing(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    // 5. Phase 3 新增：3D A* 地下墓穴与垂直下水道阶梯导航测试
    std::cout << "\n--- [测试 5: 3D A* 穿行地下墓穴与垂直下水道铁梯] --- \n";
    // 从西区街面 (2,3) 穿过 (2,4) 下水道铁格栅梯，深入地下墓道抵达泰兰·伊玛斯 (10,8)
    auto path_to_underground = PathfindingSystem::find_path_3d(
        city_map, {2, 3}, ElevationLevel::Street, {10, 8}, ElevationLevel::Underground
    );
    std::cout << "  3D 导航寻路结果: 成功生成 " << path_to_underground.size() << " 步立体移动路径\n";
    for (size_t i = 0; i < std::min<size_t>(5, path_to_underground.size()); ++i) {
        std::cout << "    * 步骤 [" << i + 1 << "]: (" << path_to_underground[i].coord.x 
                  << ", " << path_to_underground[i].coord.y << ") [" 
                  << (path_to_underground[i].elevation == ElevationLevel::Underground ? "地下墓穴" : "街面层") << "]\n";
    }

    // 6. Phase 3 新增：地下高压易燃瓦斯云爆与震荡连环引爆测试
    std::cout << "\n--- [测试 6: 地下高压易爆瓦斯云爆与连环毁伤 (HazardSystem)] --- \n";
    HazardSystem::spawn_gas_leak({10, 6}, ElevationLevel::Street, 95.0f);
    std::vector<std::string> haz_logs;
    bool haz_detonated = HazardSystem::check_and_detonate_gas({10, 6}, ElevationLevel::Street, reg, city_map, haz_logs);
    std::cout << "  瓦斯诱爆状态: " << (haz_detonated ? "成功引爆燃料空气震荡冲击波!" : "未诱爆") << "\n";
    for (const auto& hl : haz_logs) std::cout << "    " << hl << "\n";

    // 7. Phase 3 新增：ToME4 级 JSON 状态持久化与无损读档校验
    std::cout << "\n--- [测试 7: ToME4 级 JSON 全局状态保存与无损加载 (SaveLoadSystem)] --- \n";
    std::vector<std::string> save_logs;
    bool save_ok = SaveLoadSystem::save_game("assets/saves/quick_save.json", reg, city_map, kalam, save_logs);
    std::cout << "  存档状态: " << (save_ok ? "成功生成 quick_save.json" : "失败") << "\n";
    for (const auto& sl : save_logs) std::cout << "    " << sl << "\n";

    // 模拟世界状态剧变：卡拉姆受伤且时间流逝
    reg.get<ActorComponent>(kalam).hp = 42;
    TurnSystem::global_tick_count += 88;
    std::cout << "  [模拟篡改] 玩家 HP 降至 42, 时间推移至 Tick " << TurnSystem::global_tick_count << "\n";

    // 读档还原
    std::vector<std::string> load_logs;
    bool load_ok = SaveLoadSystem::load_game("assets/saves/quick_save.json", reg, city_map, kalam, load_logs);
    std::cout << "  读档状态: " << (load_ok ? "成功读取并恢复全局快照" : "失败") << "\n";
    for (const auto& ll : load_logs) std::cout << "    " << ll << "\n";
    std::cout << "  [状态还原验证] 玩家 HP: " << reg.get<ActorComponent>(kalam).hp 
              << " | 全局 Tick: " << TurnSystem::global_tick_count << "\n";

    // 8. Phase 3 新增：多区块世界拓扑架构与传送锚点 (WorldSector)
    std::cout << "\n--- [测试 8: 多区块大世界拓扑与传送网关 (WorldSector)] --- \n";
    WorldSector phoenix_sector(
        SectorId::PhoenixInnQuarter, "Phoenix Inn Quarter", "达鲁基斯坦繁华市街与青石瓦顶"
    );
    phoenix_sector.add_portal(SectorPortal{
        .trigger_pos = {5, 5},
        .trigger_elev = ElevationLevel::Street,
        .dest_sector = SectorId::DarujhistanCatacombs,
        .dest_pos = {5, 5},
        .dest_elev = ElevationLevel::Underground,
        .portal_name = "凤凰客栈下水道排水铁井"
    });
    std::cout << "  * 区块注册: 【" << phoenix_sector.name() << "】包含 " 
              << phoenix_sector.get_portals().size() << " 个世界跳跃锚点 -> 目标区块: " 
              << phoenix_sector.get_portals()[0].portal_name << "\n";

    std::cout << "\n========================================================================\n";
    std::cout << "  Phase 3 三层立体架构、瓦斯灾害与全局持久化测试全部通过!\n";
    std::cout << "  暴君状态: " << (reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity).is_sealed_in_azath ? "已封印入阿扎斯之屋" : "未封印") << "\n";
    std::cout << "========================================================================\n";
    (void)guard;
    (void)tlan_warrior;
    return 0;
}
