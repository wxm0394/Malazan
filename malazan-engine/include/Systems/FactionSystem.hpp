#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/FactionReputationComponent.hpp"
#include <vector>
#include <string>

namespace Malazan {

class FactionSystem {
public:
    static void on_kill(
        Entity killer,
        Entity victim,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<ActorComponent>(victim)) return;
        auto& vic_actor = reg.get<ActorComponent>(victim);

        if (!reg.has<FactionReputationComponent>(killer)) return;
        auto& rep = reg.get<FactionReputationComponent>(killer);

        if (vic_actor.faction == Faction::ImperialClaw) {
            rep.modify(Faction::PhoenixInnCircle, 15);
            rep.modify(Faction::CityAssassinsGuild, 25);
            rep.modify(Faction::Bridgeburners, 5);

            out_logs.push_back("【阵营声望变动】剿灭帝国爪牙！刺客公会声望 +25 (" 
                               + std::to_string(rep.get(Faction::CityAssassinsGuild)) + ") | 凤凰客栈 +15 (" 
                               + std::to_string(rep.get(Faction::PhoenixInnCircle)) + ")");
        } else if (vic_actor.faction == Faction::NeutralCivilians) {
            rep.modify(Faction::NeutralCivilians, -50);
            rep.modify(Faction::CityAssassinsGuild, -15);
            out_logs.push_back("【通缉警戒!】达鲁基斯坦平民遇害！城防军与市民声望骤降 -50！");
        } else if (vic_actor.faction == Faction::CityAssassinsGuild) {
            rep.modify(Faction::CityAssassinsGuild, -60);
            out_logs.push_back("【血仇缔结!】沃尔坎公会成员遇害！公会声望暴跌 -60，已成死仇！");
        }
    }
};

} // namespace Malazan
