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
    Relic,
    CraftingMaterial
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

    static Item create_tlan_flint_sword() {
        return Item{
            .id = "tlan_flint_sword",
            .name = "提兰·伊玛斯古燧石战刃 (T'lan Imass Flint Blade)",
            .description = "三十万年前太古骨骼仪式打磨的燧石大剑，天然散发提兰（Tellann）荒火的纯净不朽微芒，永不生锈与卷刃。",
            .category = ItemCategory::Weapon,
            .rarity = ItemRarity::ElderArtifact,
            .damage = 48,
            .damage_type = DamageType::PhysicalSlash,
            .armor_bonus = 6,
            .has_otataral = false,
            .otataral_radius = 0.0f,
            .is_munition = false,
            .heal_amount = 0,
            .charges = 1,
            .max_charges = 1
        };
    }

    static Item create_finnest_acorn() {
        return Item{
            .id = "finnest_relic",
            .name = "雷斯特的芬尼斯古橡果 (Raest's Finnest Acorn)",
            .description = "上古贾格特暴君将自身近半太古魔能封存剥离所凝聚的活体容器。佩戴时魔力激荡，但亦会招致阿扎斯之木的本能索求。",
            .category = ItemCategory::Relic,
            .rarity = ItemRarity::ElderArtifact,
            .damage = 0,
            .damage_type = DamageType::ChaosDisruption,
            .armor_bonus = 15,
            .has_otataral = false,
            .otataral_radius = 0.0f,
            .is_munition = false,
            .heal_amount = 0,
            .charges = 1,
            .max_charges = 1
        };
    }

    static Item create_sulfur_crystal(int32_t count = 1) {
        return Item{
            .id = "sulfur_crystal",
            .name = "炼金天然硫磺结晶 (Sulfur Crystal)",
            .description = "莫兰斯黑火药提纯核心组分，遇火剧烈释放氧化膨胀能量。",
            .category = ItemCategory::CraftingMaterial,
            .rarity = ItemRarity::Common,
            .charges = count,
            .max_charges = 99
        };
    }

    static Item create_blue_fire_residue(int32_t count = 1) {
        return Item{
            .id = "blue_fire_residue",
            .name = "达鲁基斯坦蓝火残渣 (Blue Fire Residue)",
            .description = "地底高压煤气冷凝萃取物，极度易挥发并带有高热沃伦催化反应。",
            .category = ItemCategory::CraftingMaterial,
            .rarity = ItemRarity::MoranthCraft,
            .charges = count,
            .max_charges = 99
        };
    }

    static Item create_moranth_chitin_casing(int32_t count = 1) {
        return Item{
            .id = "moranth_chitin_casing",
            .name = "莫兰斯几丁质外壳 (Moranth Chitin Casing)",
            .description = "黑莫兰斯军械独有的耐压虫壳，能在高压下封闭炸药能量直至引信触发。",
            .category = ItemCategory::CraftingMaterial,
            .rarity = ItemRarity::MoranthCraft,
            .charges = count,
            .max_charges = 99
        };
    }

    static Item create_quick_alchemical_fuse(int32_t count = 1) {
        return Item{
            .id = "alchemical_fuse",
            .name = "炼金速燃引信 (Quick Fuse)",
            .description = "浸泡过特拉斯（Telas）微量火油的精准延时引信。",
            .category = ItemCategory::CraftingMaterial,
            .rarity = ItemRarity::Common,
            .charges = count,
            .max_charges = 99
        };
    }

    static Item create_moranth_directional_mine(int32_t count = 1) {
        return Item{
            .id = "moranth_directional_mine",
            .name = "破桥军莫兰斯定向地雷 (Sapper Directional Mine)",
            .description = "工兵费德勒与赫奇联合研制，布设在街面或屋顶，敌方踏入扇面立即引发毁灭性破片与冲击波。",
            .category = ItemCategory::Munition,
            .rarity = ItemRarity::MoranthCraft,
            .damage = 125,
            .damage_type = DamageType::MoranthExplosive,
            .is_munition = true,
            .munition_type = MoranthMunitionType::DirectionalMine,
            .charges = count,
            .max_charges = 10
        };
    }
};

} // namespace Malazan
