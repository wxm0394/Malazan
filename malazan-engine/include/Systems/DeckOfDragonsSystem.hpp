#pragma once
#include "Core/Types.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <random>

namespace Malazan {

// 龙之塔罗牌位（Houses & Unaligned）
enum class DeckHouse : uint8_t {
    HighHouseDark,   // 高阶暗之殿 (Rake, Mother Dark)
    HighHouseShadow, // 高阶暗影之殿 (Shadowthrone, Cotillion)
    HighHouseDeath,  // 高阶死亡之殿 (Hood)
    HighHouseLife,   // 高阶生命之殿 (Burn, Queen of Life)
    HouseOfChains,   // 锁链之殿 (Crippled God)
    UnalignedOponn   // 无派系·双子运神奥彭 (The Spinner & The Push)
};

struct TarotCard {
    std::string title;           // 牌名（例："暗之骑士 (Knight of Dark)"）
    DeckHouse house;             // 所属殿宇
    std::string patron_name;     // 象征神祇/登神者
    std::string omen_description;// 命运预兆神谕
    
    // 现实扭曲数值修正 (Fate Modifiers)
    float warren_power_mult{1.0f};     // 全图沃伦法术伤害倍率
    float stealth_mult{1.0f};          // 全图潜行修正
    float oponn_gamble_chance{0.0f};   // 奥彭硬币正反面抛掷概率 (翻倍或自伤)
    float backfire_mult{1.0f};         // 沃伦反噬倍率
    float moranth_blast_mult{1.0f};    // 炼金爆炸威能
};

class DeckOfDragonsSystem {
public:
    static inline TarotCard active_card = {
        .title = "沉睡地母 (The Sleeping Mother)",
        .house = DeckHouse::HighHouseLife,
        .patron_name = "伯恩 (Burn)",
        .omen_description = "大地尚在沉眠，万物法门维系在平衡之中。",
        .warren_power_mult = 1.0f,
        .stealth_mult = 1.0f,
        .oponn_gamble_chance = 0.0f,
        .backfire_mult = 1.0f,
        .moranth_blast_mult = 1.0f
    };

    static inline std::vector<TarotCard> deck = {
        {
            .title = "暗之骑士 (Knight of Dark)",
            .house = DeckHouse::HighHouseDark,
            .patron_name = "安诺曼德·拉克 (Anomander Rake)",
            .omen_description = "黑剑龙影笼罩达鲁基斯坦上空。暗界与死界法术威能大幅跃升！",
            .warren_power_mult = 1.6f,
            .stealth_mult = 1.2f,
            .oponn_gamble_chance = 0.0f,
            .backfire_mult = 0.8f,
            .moranth_blast_mult = 1.0f
        },
        {
            .title = "暗影绳索 (The Rope - Assassin)",
            .house = DeckHouse::HighHouseShadow,
            .patron_name = "考狄利安 (Cotillion)",
            .omen_description = "影之刺客漫步于斜瓦之上。屋顶潜行与近战背刺伤害提升 100%！",
            .warren_power_mult = 1.0f,
            .stealth_mult = 2.0f,
            .oponn_gamble_chance = 0.0f,
            .backfire_mult = 1.0f,
            .moranth_blast_mult = 1.3f
        },
        {
            .title = "双子运神·纺车 (The Spinner)",
            .house = DeckHouse::UnalignedOponn,
            .patron_name = "奥彭 (Oponn)",
            .omen_description = "命运硬币正在旋转。任何攻击均有 50% 造成毁灭性暴击，或反弹自身！",
            .warren_power_mult = 1.2f,
            .stealth_mult = 1.0f,
            .oponn_gamble_chance = 0.5f,
            .backfire_mult = 2.0f,
            .moranth_blast_mult = 1.5f
        },
        {
            .title = "死之主神 (Lord of Death)",
            .house = DeckHouse::HighHouseDeath,
            .patron_name = "胡德 (Hood)",
            .omen_description = "死神之冠降临达鲁基斯坦。生机被抽离，治愈法门被完全压制。",
            .warren_power_mult = 1.3f,
            .stealth_mult = 0.8f,
            .oponn_gamble_chance = 0.0f,
            .backfire_mult = 1.5f,
            .moranth_blast_mult = 1.0f
        },
        {
            .title = "跛脚神之残渣 (The Crippled God)",
            .house = DeckHouse::HouseOfChains,
            .patron_name = "锁链之主",
            .omen_description = "异界毒素渗透地脉！施法极易产生混沌撕裂与剧毒异化。",
            .warren_power_mult = 0.9f,
            .stealth_mult = 0.7f,
            .oponn_gamble_chance = 0.25f,
            .backfire_mult = 3.0f,
            .moranth_blast_mult = 0.8f
        }
    };

    // 占卜抽牌：改变全城现实法则
    static const TarotCard& draw_next_card() {
        static std::mt19937 rng(1337);
        std::uniform_int_distribution<size_t> dist(0, deck.size() - 1);
        active_card = deck[dist(rng)];

        std::cout << "\n========================================================================\n";
        std::cout << "  【龙之塔罗发牌 (Deck of Dragons)】\n";
        std::cout << "  翻开牌面: " << active_card.title << " [" << active_card.patron_name << "]\n";
        std::cout << "  神谕预兆: " << active_card.omen_description << "\n";
        std::cout << "  现实参数修正: 法术倍率 x" << active_card.warren_power_mult 
                  << " | 潜行倍率 x" << active_card.stealth_mult 
                  << " | 反噬倍率 x" << active_card.backfire_mult << "\n";
        std::cout << "========================================================================\n\n";

        return active_card;
    }
};

} // namespace Malazan
