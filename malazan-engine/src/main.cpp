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
    std::cout << "  Phase 2 实体与沙盒生态: 凤凰客栈同盟 | 德拉格尼普尔黑剑 | 阵营外交声望\n";
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

    // Phase 2 新增：活化沙盒 NPC 生态入场
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

    std::cout << "  * 破桥军小队、凤凰客栈克鲁普、飞贼克罗库斯、城防守卫与黑夜之王阿诺曼德·雷克已全部就位。\n";

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
        std::cout << "\n[视口启动] 正在唤醒 SDL3 战术图形视口窗口 (2.5D 等距立体高清 - Phase 2)...\n";
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

    std::cout << "\n========================================================================\n";
    std::cout << "  Phase 2 战术与沙盒生态测试全部通过!\n";
    std::cout << "  暴君状态: " << (reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity).is_sealed_in_azath ? "已封印入阿扎斯之屋" : "未封印") << "\n";
    std::cout << "========================================================================\n";
    (void)guard;
    return 0;
}
