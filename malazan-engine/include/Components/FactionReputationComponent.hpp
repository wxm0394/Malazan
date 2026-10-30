#pragma once
#include "Core/Types.hpp"
#include <unordered_map>
#include <string>

namespace Malazan {

enum class FactionRelation : uint8_t {
    Hated,      // 死敌 (-100 ~ -50)
    Unfriendly, // 警惕冷淡 (-49 ~ -10)
    Neutral,    // 中立观望 (-9 ~ +24)
    Friendly,   // 盟友互助 (+25 ~ +74)
    Revered     // 誓约崇敬 (+75 ~ +100)
};

struct FactionReputationComponent {
    std::unordered_map<Faction, int32_t> standing{
        {Faction::Bridgeburners, 100},
        {Faction::PhoenixInnCircle, 40},
        {Faction::CityAssassinsGuild, 15},
        {Faction::ImperialClaw, -100},
        {Faction::TisteAndii, 20},
        {Faction::NeutralCivilians, 10}
    };

    void modify(Faction faction, int32_t delta) {
        standing[faction] = std::clamp(standing[faction] + delta, -100, 100);
    }

    [[nodiscard]] int32_t get(Faction faction) const {
        auto it = standing.find(faction);
        return it != standing.end() ? it->second : 0;
    }

    [[nodiscard]] FactionRelation get_relation(Faction faction) const {
        int32_t val = get(faction);
        if (val <= -50) return FactionRelation::Hated;
        if (val <= -10) return FactionRelation::Unfriendly;
        if (val < 25) return FactionRelation::Neutral;
        if (val < 75) return FactionRelation::Friendly;
        return FactionRelation::Revered;
    }

    [[nodiscard]] static std::string relation_to_string(FactionRelation rel) {
        switch (rel) {
            case FactionRelation::Hated: return "生死仇敌 (Hated)";
            case FactionRelation::Unfriendly: return "戒备警惕 (Unfriendly)";
            case FactionRelation::Neutral: return "中立观望 (Neutral)";
            case FactionRelation::Friendly: return "友善信任 (Friendly)";
            case FactionRelation::Revered: return "崇敬盟约 (Revered)";
        }
        return "未知";
    }
};

} // namespace Malazan
