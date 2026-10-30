#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "World/DarujhistanMap.hpp"
#include "Core/DataLoader.hpp"
#include "Systems/TurnSystem.hpp"
#include "Systems/WarrenSystem.hpp"
#include "Systems/RooftopTacticsSystem.hpp"
#include "Systems/BlueFireGasSystem.hpp"
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
    std::cout << "  《玛拉赞落魄之书》：达鲁基斯坦 (Darujhistan) - 战术核心引擎\n";
    std::cout << "  数据驱动: JSON Spells & Actors | 渲染: SDL3 交互视口 | 核心: C++20 ECS\n";
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

    // 3. 数据驱动实体生成 (从 JSON 实例化角色)
    Entity kalam = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "kalam_mekhar", {6, 6}, ElevationLevel::Rooftop
    );
    Entity quick_ben = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "quick_ben", {5, 5}, ElevationLevel::Rooftop
    );
    Entity claw_agent = DataLoader::spawn_actor_from_json(
        reg, "assets/data/actors.json", "claw_assassin", {10, 6}, ElevationLevel::Street
    );

    std::cout << "  * 破桥军小队与帝国爪牙已成功从 JSON 模板部署入场。\n";

    if (!headless) {
        std::cout << "\n[视口启动] 正在唤醒 SDL3 战术图形视口窗口 (2.5D 等距立体高清)...\n";
        GameViewport viewport(1440, 860);
        if (viewport.init()) {
            viewport.run(reg, city_map, kalam, quick_ben, claw_agent);
            return 0;
        } else {
            std::cout << "[降级处理] SDL3 初始化视口失败，切换至终端纯逻辑演算模式。\n";
        }
    }

    // 无头模拟模式（自动化全主线剧情 1~3 幕回归测试）
    std::cout << "\n[无头测试模式] 开始推进《月之花园》达鲁基斯坦三幕完整主线流程...\n";
    std::cout << ">>> 当前主线: 【" << StorylineSystem::get_act_title() << "】\n";
    std::cout << "    任务目标: " << StorylineSystem::get_current_objective() << "\n\n";

    // 第一幕：暗巷斩杀爪牙
    RooftopTacticsSystem::attempt_rooftop_leap(kalam, reg, city_map, {9, 6});
    std::vector<SpellWeaveNode> weave = {
        {WarrenType::Telas, 35.0f},
        {WarrenType::Rashan, 25.0f}
    };
    WarrenSystem::cast_multi_warren_spell(quick_ben, reg, "暗影灼热风暴", weave, {10, 6});
    BlueFireGasSystem::throw_moranth_munition(kalam, MoranthMunitionType::Burner, {10, 6}, reg, city_map);

    std::vector<std::string> story_logs;
    StorylineSystem::check_progression(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    // 第二幕：迎战上古贾格特暴君雷斯特
    std::cout << "\n>>> 推进至: 【" << StorylineSystem::get_act_title() << "】\n";
    std::cout << "    任务目标: " << StorylineSystem::get_current_objective() << "\n";
    
    // 快班全力调动远古 Tellann 死火与 Telas 火浪轰击暴君
    std::vector<SpellWeaveNode> boss_weave = {
        {WarrenType::Tellann, 50.0f},
        {WarrenType::Telas, 45.0f}
    };
    WarrenSystem::cast_multi_warren_spell(quick_ben, reg, "提兰远古死火破冰术", boss_weave, {13, 7});

    // 再次投掷莫兰斯破片炸药直击暴君
    BlueFireGasSystem::throw_moranth_munition(kalam, MoranthMunitionType::Sharper, {13, 7}, reg, city_map);

    story_logs.clear();
    // 暴君移至地脉陷阱点 (10, 6)
    if (StorylineSystem::boss_raest_entity != NULL_ENTITY) {
        reg.get<PositionComponent>(StorylineSystem::boss_raest_entity).coord = {10, 6};
        reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity).frost_armor = 0;
    }
    StorylineSystem::check_progression(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    // 第三幕：触发阿扎斯破土封印
    std::cout << "\n>>> 推进至: 【" << StorylineSystem::get_act_title() << "】\n";
    std::cout << "    任务目标: " << StorylineSystem::get_current_objective() << "\n";
    story_logs.clear();
    StorylineSystem::trigger_azath_sealing(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    std::cout << "\n========================================================================\n";
    std::cout << "  主线剧情结算: " << StorylineSystem::get_act_title() << "\n";
    std::cout << "  暴君状态: 已封印入阿扎斯古木 (" 
              << (reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity).is_sealed_in_azath ? "成功" : "失败") 
              << ") | 阿扎斯地脉指数: " << WarrenSystem::azath_house_alertness << "%\n";
    std::cout << "========================================================================\n";
    return 0;
}
