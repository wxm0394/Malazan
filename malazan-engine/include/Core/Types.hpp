#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace Malazan {

// 2D 整数网格坐标
struct Vec2i {
    int32_t x{0};
    int32_t y{0};

    constexpr bool operator==(const Vec2i& other) const = default;
    constexpr Vec2i operator+(const Vec2i& other) const { return {x + other.x, y + other.y}; }
    constexpr Vec2i operator-(const Vec2i& other) const { return {x - other.x, y - other.y}; }
    
    [[nodiscard]] constexpr int32_t manhattan_dist(const Vec2i& other) const {
        return (x > other.x ? x - other.x : other.x - x) + 
               (y > other.y ? y - other.y : other.y - y);
    }
};

// 达鲁基斯坦立体高程 Z-Level
enum class ElevationLevel : uint8_t {
    Underground = 0, // 地下水道、煤气主干管、阿扎斯沉睡古墓
    Street      = 1, // 石板路、集市、广场、蓝火路灯
    Rooftop     = 2  // 斜瓦顶、横梁、烟囱通道（刺客高速路）
};

[[nodiscard]] inline constexpr std::string_view elevation_to_string(ElevationLevel lvl) {
    switch (lvl) {
        case ElevationLevel::Underground: return "地下 (Underground)";
        case ElevationLevel::Street:      return "街面 (Street)";
        case ElevationLevel::Rooftop:     return "屋顶 (Rooftop)";
        default:                          return "未知";
    }
}

// 派系划分
enum class Faction : uint8_t {
    NeutralCivilians,    // 达鲁基斯坦平民
    Bridgeburners,       // 玛拉赞第九军团·破桥军 (Whiskeyjack, Kalam, Quick Ben)
    CityAssassinsGuild,  // 刺客公会 (Vorcan)
    ImperialClaw,        // 帝国爪牙特工 (Pearl, Surly's network)
    TisteAndii,          // 漂浮城提斯·安迪尔族 (Anomander Rake 阵营)
    PhoenixInnCircle,    // 凤凰客栈同盟 (Kruppe, Crokus, Murillio)
    JaghutTyrantRaest,   // 沉睡与苏醒的贾格特暴君
    TlanImass,           // 泰兰·伊玛斯太古骨骸战士 (Logros/Kron T'lan Imass)
    ChaosVoidDemons      // 沃伦裂隙混沌空间恶魔 (Chaos Rift Spawns)
};

// 沃伦法术界 (Warrens)
enum class WarrenType : uint8_t {
    Telas,            // 火之沃伦
    Rashan,           // 阴影与隐匿沃伦
    Meanas,           // 幻术与欺瞒沃伦
    Denul,            // 治愈与生命沃伦
    Thyr,             // 炽光沃伦
    Driss,            // 大地与石之沃伦
    KuraldGalain,     // 上古纯暗（Elder Dark，提斯·安迪尔母界）
    OmtosePhellack,   // 上古坚冰（Elder Ice，贾格特界）
    Tellann,          // 上古荒火（Elder Fire，提兰·伊玛斯界）
    Chaos             // 混沌（未成形的纯破坏力）
};

// 伤害类型
enum class DamageType : uint8_t {
    PhysicalSlash,    // 刀剑劈砍 (如 Kalam 匕首)
    PhysicalPierce,   // 箭矢/飞刀
    MoranthExplosive, // 莫兰斯炸药（无视抗性、破坏地形与建筑物）
    WarrenFire,       // Telas 火浪
    WarrenDarkness,   // Rashan 黑暗与腐蚀
    ElderCold,        // 贾格特绝对零度冻结
    ChaosDisruption   // 空间裂解反噬
};

// 莫兰斯炼金弹药类型
enum class MoranthMunitionType : uint8_t {
    Sharper,         // 破片弹：高范围物理割裂，秒杀轻甲
    Burner,          // 燃烧弹：剧烈引燃，可诱爆蓝火煤气管网
    Smoker,          // 烟雾弹：全屏遮蔽视野，强行进入隐匿
    Quencher,        // 灭能弹：熄灭范围内的魔法反应与沃伦连接
    DirectionalMine  // 定向破片雷：工兵战术布设，范围触发毁灭杀伤
};

} // namespace Malazan
