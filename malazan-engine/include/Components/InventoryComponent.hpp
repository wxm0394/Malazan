#pragma once
#include "Core/Types.hpp"
#include "Items/Item.hpp"
#include <vector>
#include <optional>
#include <string>

namespace Malazan {

struct InventoryComponent {
    std::optional<Item> equipped_main_hand{std::nullopt};
    std::optional<Item> equipped_off_hand{std::nullopt};
    std::vector<Item> backpack;
    size_t max_capacity{16};

    bool add_item(Item item) {
        if (backpack.size() >= max_capacity) {
            return false;
        }
        backpack.push_back(std::move(item));
        return true;
    }

    void equip(Item item) {
        if (item.category == ItemCategory::Weapon) {
            equipped_main_hand = std::move(item);
        } else {
            equipped_off_hand = std::move(item);
        }
    }

    bool remove_item(size_t index) {
        if (index >= backpack.size()) return false;
        backpack.erase(backpack.begin() + index);
        return true;
    }

    bool equip_main_hand(size_t backpack_index) {
        if (backpack_index >= backpack.size()) return false;
        if (backpack[backpack_index].category != ItemCategory::Weapon) return false;

        Item to_equip = std::move(backpack[backpack_index]);
        backpack.erase(backpack.begin() + backpack_index);

        if (equipped_main_hand.has_value()) {
            backpack.push_back(std::move(*equipped_main_hand));
        }
        equipped_main_hand = std::move(to_equip);
        return true;
    }

    bool equip_off_hand(size_t backpack_index) {
        if (backpack_index >= backpack.size()) return false;
        if (backpack[backpack_index].category != ItemCategory::Weapon && 
            backpack[backpack_index].category != ItemCategory::Armor) return false;

        Item to_equip = std::move(backpack[backpack_index]);
        backpack.erase(backpack.begin() + backpack_index);

        if (equipped_off_hand.has_value()) {
            backpack.push_back(std::move(*equipped_off_hand));
        }
        equipped_off_hand = std::move(to_equip);
        return true;
    }

    [[nodiscard]] bool has_otataral_equipped() const {
        if (equipped_main_hand.has_value() && equipped_main_hand->has_otataral) return true;
        if (equipped_off_hand.has_value() && equipped_off_hand->has_otataral) return true;
        return false;
    }

    [[nodiscard]] float get_max_otataral_radius() const {
        float r = 0.0f;
        if (equipped_main_hand.has_value() && equipped_main_hand->has_otataral) {
            r = std::max(r, equipped_main_hand->otataral_radius);
        }
        if (equipped_off_hand.has_value() && equipped_off_hand->has_otataral) {
            r = std::max(r, equipped_off_hand->otataral_radius);
        }
        return r;
    }

    [[nodiscard]] int32_t count_item(const std::string& item_id) const {
        int32_t total = 0;
        for (const auto& item : backpack) {
            if (item.id == item_id) {
                total += std::max(1, item.charges);
            }
        }
        return total;
    }
};

} // namespace Malazan
