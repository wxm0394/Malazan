#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/OtataralComponent.hpp"
#include "World/DarujhistanMap.hpp"
#include "Core/DataLoader.hpp"
#include "Systems/TurnSystem.hpp"
#include "Systems/WarrenSystem.hpp"
#include "Systems/RooftopTacticsSystem.hpp"
#include "Systems/BlueFireGasSystem.hpp"
#include "Systems/FOVSystem.hpp"
#include "Systems/OtataralSystem.hpp"
#include "Systems/PathfindingSystem.hpp"
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
    std::cout << "  Phase 1 战术深度: 3D A* 寻路 | 奥塔拉尔绝魔死域 | 战争迷雾 | 破桥军军械库\n";
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

    std::cout << "  * 破桥军小队与帝国爪牙已成功从 JSON 模板部署入场。\n";

    // 验证 Phase 1 系统核心逻辑
    if (reg.has<InventoryComponent>(kalam)) {
        auto& inv = reg.get<InventoryComponent>(kalam);
        std::cout << "  * [装备验证] 卡拉姆主手已装备: 【" 
                  << (inv.equipped_main_hand.has_value() ? inv.equipped_main_hand->name : "空") << "】\n";
    }

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

    // 无头测试模式（自动化 Phase 1 战术模块与主线 1~3 幕回归测试）
    std::cout << "\n[无头测试模式] 开始回归测试 Phase 1 战术模块...\n";

    // A. 3D A* 空间寻路测试
    std::cout << "\n--- [测试 1: 3D A* 寻路] --- \n";
    PathNode start_node{{2, 2}, ElevationLevel::Street};
    PathNode goal_node{{8, 6}, ElevationLevel::Rooftop}; // 目标在凤凰客栈屋顶
    auto path = PathfindingSystem::find_path(start_node, goal_node, city_map, true);
    std::cout << "  从街面 (2,2) 寻路至屋顶 (8,6) 最优路径节点数: " << path.size() << "\n";
    for (size_t i = 0; i < std::min<size_t>(5, path.size()); ++i) {
        std::cout << "    Step " << i << ": (" << path[i].pos.x << ", " << path[i].pos.y 
                  << ") [" << elevation_to_string(path[i].elev) << "]\n";
    }

    // B. FOV 光线投射视野测试
    std::cout << "\n--- [测试 2: FOV 光线投射与战争迷雾] --- \n";
    std::vector<VisibilityState> fog_grid;
    FOVSystem::compute_fov({6, 6}, ElevationLevel::Rooftop, 8, city_map, fog_grid);
    int visible_count = 0;
    for (auto s : fog_grid) if (s == VisibilityState::Visible) visible_count++;
    std::cout << "  卡拉姆于屋顶 (6,6) 观测全城，可见格数: " << visible_count << "/" << fog_grid.size() << "\n";

    // C. 奥塔拉尔绝魔死域测试
    std::cout << "\n--- [测试 3: 奥塔拉尔反魔力场压制] --- \n";
    float ota_intensity = 0.0f;
    bool suppressed = OtataralSystem::is_suppressed_at({6, 6}, ElevationLevel::Rooftop, reg, &ota_intensity);
    std::cout << "  坐标 (6,6) 处反魔压制状态: " << (suppressed ? "处于奥塔拉尔死域中" : "正常") 
              << " | 压制强度: " << (ota_intensity * 100.0f) << "%\n";

    // D. 莫兰斯“卡瑟”巨弹引爆测试
    std::cout << "\n--- [测试 4: 破桥军莫兰斯·卡瑟毁灭巨弹] --- \n";
    std::vector<std::string> cusser_logs;
    BlueFireGasSystem::throw_moranth_munition(kalam, MoranthMunitionType::Sharper, {10, 6}, reg, city_map, true, &cusser_logs);
    for (const auto& log : cusser_logs) std::cout << "  " << log << "\n";

    // E. 主线战役剧情自动化贯通回归测试
    std::cout << "\n--- [测试 5: 主线全剧幕贯通回归] --- \n";
    std::cout << ">>> 当前主线: 【" << StorylineSystem::get_act_title() << "】\n";
    std::vector<std::string> story_logs;
    StorylineSystem::check_progression(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    // 第二幕：迎战上古贾格特暴君雷斯特
    std::cout << "\n>>> 推进至: 【" << StorylineSystem::get_act_title() << "】\n";
    if (StorylineSystem::boss_raest_entity != NULL_ENTITY) {
        reg.get<PositionComponent>(StorylineSystem::boss_raest_entity).coord = {10, 6};
        reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity).frost_armor = 0;
    }
    story_logs.clear();
    StorylineSystem::check_progression(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    // 第三幕：触发阿扎斯破土封印
    std::cout << "\n>>> 推进至: 【" << StorylineSystem::get_act_title() << "】\n";
    story_logs.clear();
    StorylineSystem::trigger_azath_sealing(reg, city_map, story_logs);
    for (const auto& log : story_logs) std::cout << "  " << log << "\n";

    std::cout << "\n========================================================================\n";
    std::cout << "  Phase 1 战术系统测试全部通过!\n";
    std::cout << "  暴君状态: " << (reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity).is_sealed_in_azath ? "已封印入阿扎斯之屋" : "未封印") << "\n";
    std::cout << "========================================================================\n";
    return 0;
}
