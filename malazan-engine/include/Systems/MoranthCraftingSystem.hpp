#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/InventoryComponent.hpp"
#include "Items/Item.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace Malazan {

enum class MoranthRecipeType : uint8_t {
    Sharper,
    Burner,
    Cusser,
    DirectionalMine
};

struct CraftingRequirement {
    std::string material_id;
    std::string item_name;
    int32_t quantity{1};

    // 兼容 ingredients 字段
    std::string item_id() const { return material_id; }
    int32_t count() const { return quantity; }
};

struct CraftingRecipe {
    MoranthRecipeType type{MoranthRecipeType::Sharper};
    std::string recipe_id;
    std::string name;
    std::string output_name;
    std::string description;
    std::vector<CraftingRequirement> required_materials;
    std::vector<CraftingRequirement> ingredients;
    std::function<Item()> output_builder;
};

class MoranthCraftingSystem {
public:
    static std::vector<CraftingRecipe> get_all_recipes() {
        std::vector<CraftingRecipe> recipes;

        CraftingRecipe sharper{
            .type = MoranthRecipeType::Sharper,
            .recipe_id = "craft_sharper",
            .name = "莫兰斯·破片弹 (Sharper)",
            .output_name = "莫兰斯·破片弹 (Sharper)",
            .description = "几丁质破片杀伤弹，适合大范围削弱敌方轻装单位。",
            .required_materials = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 1},
                {"sulfur_crystal", "天然硫磺结晶", 1},
                {"alchemical_fuse", "炼金速燃引信", 1}
            },
            .ingredients = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 1},
                {"sulfur_crystal", "天然硫磺结晶", 1},
                {"alchemical_fuse", "炼金速燃引信", 1}
            },
            .output_builder = []() { return ItemFactory::create_moranth_sharper(); }
        };
        recipes.push_back(sharper);

        CraftingRecipe burner{
            .type = MoranthRecipeType::Burner,
            .recipe_id = "craft_burner",
            .name = "莫兰斯·燃烧弹 (Burner)",
            .output_name = "莫兰斯·燃烧弹 (Burner)",
            .description = "高热蓝火聚能引燃弹，可诱爆地底与街区瓦斯气云。",
            .required_materials = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 1},
                {"blue_fire_residue", "达鲁基斯坦蓝火残渣", 1},
                {"alchemical_fuse", "炼金速燃引信", 1}
            },
            .ingredients = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 1},
                {"blue_fire_residue", "达鲁基斯坦蓝火残渣", 1},
                {"alchemical_fuse", "炼金速燃引信", 1}
            },
            .output_builder = []() { return ItemFactory::create_moranth_burner(); }
        };
        recipes.push_back(burner);

        CraftingRecipe cusser{
            .type = MoranthRecipeType::Cusser,
            .recipe_id = "craft_cusser",
            .name = "莫兰斯·卡瑟攻城巨弹 (Cusser)",
            .output_name = "莫兰斯·卡瑟攻城巨弹 (Cusser)",
            .description = "毁灭性攻城重型炸弹，直接轰塌整座石砌建筑并引发连环地震。",
            .required_materials = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 2},
                {"blue_fire_residue", "达鲁基斯坦蓝火残渣", 2},
                {"sulfur_crystal", "天然硫磺结晶", 2},
                {"alchemical_fuse", "炼金速燃引信", 1}
            },
            .ingredients = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 2},
                {"blue_fire_residue", "达鲁基斯坦蓝火残渣", 2},
                {"sulfur_crystal", "天然硫磺结晶", 2},
                {"alchemical_fuse", "炼金速燃引信", 1}
            },
            .output_builder = []() { return ItemFactory::create_moranth_cusser(); }
        };
        recipes.push_back(cusser);

        CraftingRecipe mine{
            .type = MoranthRecipeType::DirectionalMine,
            .recipe_id = "craft_mine",
            .name = "破桥军莫兰斯定向地雷 (Directional Mine)",
            .output_name = "破桥军莫兰斯定向地雷 (Directional Mine)",
            .description = "工兵预设伏击防线，任何敌对目标踩入扇形死域立即引爆。",
            .required_materials = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 1},
                {"sulfur_crystal", "天然硫磺结晶", 1},
                {"blue_fire_residue", "达鲁基斯坦蓝火残渣", 1}
            },
            .ingredients = {
                {"moranth_chitin_casing", "莫兰斯几丁质外壳", 1},
                {"sulfur_crystal", "天然硫磺结晶", 1},
                {"blue_fire_residue", "达鲁基斯坦蓝火残渣", 1}
            },
            .output_builder = []() { return ItemFactory::create_moranth_directional_mine(); }
        };
        recipes.push_back(mine);

        return recipes;
    }

    // 统计背包中某原材料的总量（支持别名匹配）
    static int32_t count_ingredient(const InventoryComponent& inv, const std::string& item_id) {
        int32_t total = 0;
        for (const auto& item : inv.backpack) {
            if (item.id == item_id ||
                (item_id == "moranth_chitin_casing" && item.id == "mat_moranth_chitin") ||
                (item_id == "sulfur_crystal" && item.id == "mat_sulfur_crystal") ||
                (item_id == "blue_fire_residue" && item.id == "mat_blue_fire_residue") ||
                (item_id == "alchemical_fuse" && item.id == "mat_quick_fuse")) {
                total += std::max(1, item.charges);
            }
        }
        return total;
    }

    // 检查是否满足合成所需材料
    static bool can_craft(const InventoryComponent& inv, const std::string& recipe_id) {
        auto recipes = get_all_recipes();
        for (const auto& r : recipes) {
            if (r.recipe_id == recipe_id) {
                for (const auto& ing : r.required_materials) {
                    if (count_ingredient(inv, ing.material_id) < ing.quantity) {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }

    static bool can_craft(const InventoryComponent& inv, MoranthRecipeType type) {
        auto recipes = get_all_recipes();
        for (const auto& r : recipes) {
            if (r.type == type) {
                return can_craft(inv, r.recipe_id);
            }
        }
        return false;
    }

    static bool can_craft(Entity ent, MoranthRecipeType type, Registry& reg) {
        if (!reg.has<InventoryComponent>(ent)) return false;
        return can_craft(reg.get<InventoryComponent>(ent), type);
    }

    static bool can_craft(Entity ent, const std::string& recipe_id, Registry& reg) {
        if (!reg.has<InventoryComponent>(ent)) return false;
        return can_craft(reg.get<InventoryComponent>(ent), recipe_id);
    }

    // 执行合成消耗材料并入包
    static bool craft(
        InventoryComponent& inv,
        const std::string& recipe_id,
        std::vector<std::string>& out_logs
    ) {
        auto recipes = get_all_recipes();
        for (const auto& r : recipes) {
            if (r.recipe_id == recipe_id) {
                if (!can_craft(inv, recipe_id)) {
                    out_logs.push_back("[工坊提示] 原材料不足，无法调配组装: " + r.output_name);
                    return false;
                }

                // 消耗材料
                for (const auto& ing : r.required_materials) {
                    int32_t needed = ing.quantity;
                    for (size_t i = 0; i < inv.backpack.size() && needed > 0; ) {
                        bool match = (inv.backpack[i].id == ing.material_id) ||
                            (ing.material_id == "moranth_chitin_casing" && inv.backpack[i].id == "mat_moranth_chitin") ||
                            (ing.material_id == "sulfur_crystal" && inv.backpack[i].id == "mat_sulfur_crystal") ||
                            (ing.material_id == "blue_fire_residue" && inv.backpack[i].id == "mat_blue_fire_residue") ||
                            (ing.material_id == "alchemical_fuse" && inv.backpack[i].id == "mat_quick_fuse");

                        if (match) {
                            if (inv.backpack[i].charges <= needed) {
                                needed -= inv.backpack[i].charges;
                                inv.remove_item(i);
                            } else {
                                inv.backpack[i].charges -= needed;
                                needed = 0;
                                ++i;
                            }
                        } else {
                            ++i;
                        }
                    }
                }

                // 生成产物
                Item crafted_item = r.output_builder();
                inv.add_item(crafted_item);

                out_logs.push_back("=================================================================");
                out_logs.push_back("【莫兰斯军械炼成!】工兵手艺精进，成功组装出: 【" + r.output_name + "】");
                out_logs.push_back("=================================================================");
                return true;
            }
        }
        out_logs.push_back("[工坊错误] 未找到配方: " + recipe_id);
        return false;
    }

    static bool craft(
        InventoryComponent& inv,
        MoranthRecipeType type,
        std::vector<std::string>& out_logs
    ) {
        auto recipes = get_all_recipes();
        for (const auto& r : recipes) {
            if (r.type == type) {
                return craft(inv, r.recipe_id, out_logs);
            }
        }
        out_logs.push_back("[工坊错误] 未知配方类型。");
        return false;
    }

    static bool craft(
        Entity ent,
        MoranthRecipeType type,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<InventoryComponent>(ent)) {
            out_logs.push_back("[工坊错误] 实体无行囊组件。");
            return false;
        }
        return craft(reg.get<InventoryComponent>(ent), type, out_logs);
    }

    static bool craft(
        Entity ent,
        const std::string& recipe_id,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<InventoryComponent>(ent)) {
            out_logs.push_back("[工坊错误] 实体无行囊组件。");
            return false;
        }
        return craft(reg.get<InventoryComponent>(ent), recipe_id, out_logs);
    }
};

} // namespace Malazan
