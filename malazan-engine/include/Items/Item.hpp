#pragma once
#include "Core/Types.hpp"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace Malazan {

enum class ItemCategory : uint8_t {
    Weapon,
    Armor,
    Munition,
    Consumable,
    Relic
};

enum class ItemRarity : uint8_t {
    Common,          // 普通铁器/布甲
    MoranthCraft,    // 莫兰斯炼金术造物（炼金火药、轻甲黑壳）
    WarrenInfused,   // 沃伦附魔器物
    OtataralForged,  // 奥塔拉尔反魔矿石锻造（赭红锈刃，撕裂法网）
    ElderArtifact    // 太古神器（提斯·安迪尔族暗夜之刃、龙之塔罗法器等）
};

struct Item {
    std::string id{"unknown_item"};
    std::string name{"无名道具"};
    std::string description{""};
    ItemCategory category{ItemCategory::Consumable};
    ItemRarity rarity{ItemRarity::Common};

    int32_t damage{0};
    DamageType damage_type{DamageType::PhysicalSlash};
    int32_t armor_bonus{0};

    // 奥塔拉尔反魔特性
    bool has_otataral{false};
    float otataral_radius{3.0f};

    // 莫兰斯炸药特性
    bool is_munition{false};
    MoranthMunitionType munition_type{MoranthMunitionType::Sharper};

    // 恢复与辅助特性
    int32_t heal_amount{0};
    int32_t charges{1};
    int32_t max_charges{1};

    [[nodiscard]] bool is_usable() const {
        return (category == ItemCategory::Consumable || category == ItemCategory::Munition) && charges > 0;
    }
};

class ItemFactory {
public:
    static Item create_otataral_dagger() {
        return Item{
            .id = "otataral_long_dagger",
            .name = "奥塔拉尔长匕首 (Otataral Long Dagger)",
            .description = "卡拉姆贴身死决短兵。刃身由纯粹的生锈赭红奥塔拉尔反魔矿石捶打而成，天然散发能熄灭一切沃伦法门通道的绝死力场。",
            .category = ItemCategory::Weapon,
            .rarity = ItemRarity::OtataralForged,
            .damage = 42,
            .damage_type = DamageType::PhysicalSlash,
            .armor_bonus = 0,
            .has_otataral = true,
            .otataral_radius = 3.5f,
            .is_munition = false,
            .heal_amount = 0,
            .charges = 1,
            .max_charges = 1
        };
    }

    static Item create_bridgeburner_blade() {
        return Item{
            .id = "bridgeburner_blade",
            .name = "破桥军战壕军刀 (Bridgeburner Trench Blade)",
            .description = "玛拉赞帝国第九军团标准配发合金宽刃刀，兼具肉搏劈砍与格挡能力，伴随第七纵队浴血数载。",
            .category = ItemCategory::Weapon,
            .rarity = ItemRarity::Common,
            .damage = 28,
            .damage_type = DamageType::PhysicalSlash,
            .armor_bonus = 4,
            .has_otataral = false,
            .otataral_radius = 0.0f,
            .is_munition = false,
            .heal_amount = 0,
            .charges = 1,
            .max_charges = 1
        };
    }

    static Item create_moranth_burner() {
        return Item{
            .id = "moranth_burner",
            .name = "莫兰斯燃烧弹 (Moranth Burner)",
            .description = "莫兰斯黑甲同盟研制的强效炼金火剂，掷出后剧烈灼烧周围地面，可瞬间引爆泄漏的达鲁基斯坦蓝火煤气管网。",
            .category = ItemCategory::Munition,
            .rarity = ItemRarity::MoranthCraft,
            .damage = 60,
            .damage_type = DamageType::MoranthExplosive,
            .armor_bonus = 0,
            .has_otataral = false,
            .otataral_radius = 0.0f,
            .is_munition = true,
            .munition_type = MoranthMunitionType::Burner,
            .heal_amount = 0,
            .charges = 3,
            .max_charges = 3
        };
    }

    static Item create_moranth_sharper() {
        return Item{
            .id = "moranth_sharper",
            .name = "莫兰斯破片弹 (Moranth Sharper)",
            .description = "高爆破片陶罐炸弹。爆炸瞬间喷射数以千计的锋利陶片，粉碎木质屋顶结构并撕裂周围所有无重甲护体目标。",
            .category = ItemCategory::Munition,
            .rarity = ItemRarity::MoranthCraft,
            .damage = 75,
            .damage_type = DamageType::MoranthExplosive,
            .armor_bonus = 0,
            .has_otataral = false,
            .otataral_radius = 0.0f,
            .is_munition = true,
            .munition_type = MoranthMunitionType::Sharper,
            .heal_amount = 0,
            .charges = 2,
            .max_charges = 2
        };
    }

    static Item create_moranth_cusser() {
        return Item{
            .id = "moranth_cusser",
            .name = "莫兰斯“卡瑟”重爆弹 (Moranth Cusser)",
            .description = "破桥军最恐惧亦最依赖的毁灭之球。装药量足以将一整座石造多层要塞直接轰平为瓦砾废墟，震慑上古首领级神魔。",
            .category = ItemCategory::Munition,
            .rarity = ItemRarity::MoranthCraft,
            .damage = 180,
            .damage_type = DamageType::MoranthExplosive,
            .armor_bonus = 0,
            .has_otataral = false,
            .otataral_radius = 0.0f,
            .is_munition = true,
            .munition_type = MoranthMunitionType::Sharper, // 最高杀伤型
            .heal_amount = 0,
            .charges = 1,
            .max_charges = 1
        };
    }

    static Item create_denul_draught() {
        return Item{
            .id = "denul_draught",
            .name = "德努尔愈合合剂 (Denul Draught)",
            .description = "萃取自德努尔（Denul）治愈沃伦活性能量的金色药剂，快速愈合内外贯穿创伤，止血生肌。",
            .category = ItemCategory::Consumable,
            .rarity = ItemRarity::WarrenInfused,
            .damage = 0,
            .damage_type = DamageType::PhysicalSlash,
            .armor_bonus = 0,
            .has_otataral = false,
            .otataral_radius = 0.0f,
            .is_munition = false,
            .heal_amount = 45,
            .charges = 2,
            .max_charges = 2
        };
    }
};

} // namespace Malazan
