#pragma once
#include "Core/Types.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace Malazan {

// 角色基础属性组件
struct ActorComponent {
    std::string name{"Unnamed"};
    Faction faction{Faction::NeutralCivilians};
    int32_t hp{100};
    int32_t max_hp{100};
    bool is_alive{true};

    void take_damage(int32_t amount, DamageType type) {
        (void)type;
        hp -= amount;
        if (hp <= 0) {
            hp = 0;
            is_alive = false;
        }
    }
};

// 位置与立体层级组件
struct PositionComponent {
    Vec2i coord{0, 0};
    ElevationLevel elevation{ElevationLevel::Street};
    bool can_climb_roofs{false}; // 能否攀爬屋檐并进行屋顶飞跃
    int32_t rooftop_jump_range{2}; // 屋顶间隙跳跃距离
};

// ToME 风格能量回合调度组件 (1000 Energy = 1 Turn)
struct TurnEnergyComponent {
    int32_t energy{0};
    int32_t speed{1000}; // 标准速度 1000，敏捷刺客可达 1300+，重甲士兵约 800

    [[nodiscard]] bool can_act() const { return energy >= 1000; }
    void consume(int32_t cost = 1000) { energy -= cost; }
};

// 沃伦引导组件（法门体系）
struct WarrenChannelerComponent {
    std::unordered_map<WarrenType, float> affinities; // 沃伦亲和力 (0~100)
    float current_stress{0.0f};                       // 当前法门压力
    float max_safe_stress{100.0f};                    // 安全负荷阈值
    float chaos_corruption{0.0f};                     // 混沌污染度

    [[nodiscard]] float get_affinity(WarrenType type) const {
        auto it = affinities.find(type);
        return it != affinities.end() ? it->second : 0.0f;
    }
};

// 潜行与刺客感知组件
struct StealthComponent {
    bool in_stealth{false};
    float stealth_power{60.0f}; // 隐匿值，对抗敌方侦测
    float noise_radius{0.0f};   // 产生的声波扩散范围（格数）
};

// 达鲁基斯坦环境：蓝火煤气管网节点
struct BlueFireNodeComponent {
    Vec2i coord{0, 0};
    float gas_pressure{100.0f}; // 管道气压
    bool is_lit{true};          // 是否处于点亮燃烧状态
    bool is_leaking{false};     // 是否已遭击穿泄漏
    int32_t blast_yield{65};    // 殉爆造成的范围杀伤力
    int32_t blast_radius{3};    // 爆炸波及半径
};

// 莫兰斯炼金弹药组件（破桥军特权装备）
struct MoranthMunitionComponent {
    std::unordered_map<MoranthMunitionType, uint32_t> ammo{
        {MoranthMunitionType::Sharper, 0},
        {MoranthMunitionType::Burner, 0},
        {MoranthMunitionType::Smoker, 0},
        {MoranthMunitionType::Quencher, 0}
    };

    [[nodiscard]] bool use(MoranthMunitionType type) {
        if (ammo[type] > 0) {
            ammo[type]--;
            return true;
        }
        return false;
    }
};

// 高度优势战术修正
struct TacticalElevationComponent {
    float high_ground_dmg_bonus{0.35f}; // 屋顶攻击街面目标的伤害加成 (+35%)
    float low_ground_hit_penalty{0.25f}; // 街面反击屋顶时的命中惩罚 (-25%)
};

} // namespace Malazan
