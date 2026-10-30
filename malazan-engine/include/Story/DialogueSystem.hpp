#pragma once
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/FactionReputationComponent.hpp"
#include "Systems/WarrenSystem.hpp"
#include <string>
#include <vector>
#include <functional>

namespace Malazan {

struct DialogueNode {
    std::string speaker_name;
    std::string speaker_title;
    std::string dialogue_text;
    std::vector<std::string> options;
    std::string outcome_summary;
};

class DialogueSystem {
public:
    static DialogueNode get_dialogue_for(Entity npc, Entity player, Registry& reg) {
        if (!reg.has<ActorComponent>(npc)) {
            return {"神秘人", "", "……（无回应）", {"离开"}, ""};
        }

        auto& actor = reg.get<ActorComponent>(npc);

        // 克鲁普 (Kruppe) 哲学与大梦之境对话
        if (actor.name.find("Kruppe") != std::string::npos || actor.name.find("克鲁普") != std::string::npos) {
            return DialogueNode{
                .speaker_name = "克鲁普 (Kruppe)",
                .speaker_title = "凤凰客栈常客 · 大梦编织者 · 甜点鉴赏大师",
                .dialogue_text = "“啊哈！克鲁普那敏锐而卓越的双眼，已在酥皮蜜饼的碎屑中预见到了阁下的到来！\n"
                                 "黑暗正如同黏稠的糖浆般从加德罗比丘陵向达鲁基斯坦倾泻……\n"
                                 "不过无须恐慌，卑微而无害的克鲁普早已在梦境的古老泉水中为诸位备好了安神之饮。”",
                .options = {
                    "[1] 接受克鲁普的甜点与【大梦庇护】(降低阿扎斯警觉度，平复法门压力)",
                    "[2] 询问关于贾格特暴君雷斯特与芬尼斯遗物的线索",
                    "[3] 默默离开凤凰客栈大堂"
                },
                .outcome_summary = "获得大梦古泉祝福：阿扎斯地脉警觉度消减 -15%，生命恢复 +30 HP！"
            };
        }

        // 克罗库斯 (Crokus) 飞贼屋顶对话
        if (actor.name.find("Crokus") != std::string::npos || actor.name.find("克罗库斯") != std::string::npos) {
            return DialogueNode{
                .speaker_name = "克罗库斯·幼手 (Crokus)",
                .speaker_title = "达鲁基斯坦本地飞贼 · 奥彭硬币持有者",
                .dialogue_text = "“嘿，卡拉姆！小心客栈东侧烟囱旁的青瓦，那里的灰浆早被蓝火烤酥了。\n"
                                 "我刚才在屋檐边抛了下硬币——硬币竖立在瓦片缝隙里！\n"
                                 "命运的两位神明（奥彭小姐的牵引与贵族大人的推搡）今夜都在注视这片瓦顶！”",
                .options = {
                    "[1] 借助克罗库斯的指引探寻屋顶捷径 (跳跃范围 +1 格)",
                    "[2] 借用奥彭硬币占卜今日运势",
                    "[3] 提醒年轻人离帝国爪牙远点"
                },
                .outcome_summary = "克罗库斯分享了暗道线索：屋顶轻功飞跃距离临时提升 +1 格！"
            };
        }

        // 阿诺曼德·雷克 (Anomander Rake) 提斯·安迪尔黑夜霸主
        if (actor.name.find("Rake") != std::string::npos || actor.name.find("雷克") != std::string::npos) {
            return DialogueNode{
                .speaker_name = "阿诺曼德·雷克 (Anomander Rake)",
                .speaker_title = "黑夜之子 · 漂浮城之主 · 德拉格尼普尔挥舞者",
                .dialogue_text = "“破桥军的刺客……女皇的野心给这片古老的大陆招致了灾祸。\n"
                                 "上古暴君雷斯特已在荒原破土，他的冰雪正企图冻结达鲁基斯坦的生机。\n"
                                 "若暴君踏足城门半步，德拉格尼普尔的大车自会将他的灵魂套上沉重的枷锁。”",
                .options = {
                    "[1] 请求雷克出剑截击暴君雷斯特",
                    "[2] 凝视黑剑【德拉格尼普尔】感受纯暗大车的咆哮",
                    "[3] 行军礼退下"
                },
                .outcome_summary = "雷克应允介入：当暴君靠近时，德拉格尼普尔将降下极夜天罚！"
            };
        }

        // 沃尔坎 (Vorcan) 刺客公会会长
        if (actor.name.find("Vorcan") != std::string::npos || actor.name.find("沃尔坎") != std::string::npos) {
            return DialogueNode{
                .speaker_name = "沃尔坎 (Vorcan)",
                .speaker_title = "达鲁基斯坦刺客公会会长 · 影之女大公",
                .dialogue_text = "“公会通常不欢迎不请自来的异国刀客，破桥军。\n"
                                 "但既然你们砍翻的是帝国爪牙这群讨厌的鬣狗，我今晚可以网开一面。\n"
                                 "别在市政厅议员的宅邸前惹事，否则我的影刃不会留情。”",
                .options = {
                    "[1] 交付帝国爪牙密信换取莫兰斯补给",
                    "[2] 达成屋顶巡逻停火默契",
                    "[3] 闪身隐入阴影"
                },
                .outcome_summary = "刺客公会达成中立协同：赠予破桥军 1 份高纯度炼金破片炸药！"
            };
        }

        (void)player;
        (void)reg;
        return DialogueNode{
            .speaker_name = actor.name,
            .speaker_title = "达鲁基斯坦市民",
            .dialogue_text = "“赞美蓝火，今夜的仲夏夜狂欢节真是热闹非凡啊……”",
            .options = {"[1] 匆匆走开"},
            .outcome_summary = ""
        };
    }

    // 执行对话选项效果
    static void apply_dialogue_choice(
        Entity npc,
        Entity player,
        int choice_idx,
        Registry& reg,
        std::vector<std::string>& out_logs
    ) {
        if (!reg.has<ActorComponent>(npc) || !reg.has<ActorComponent>(player)) return;
        auto& npc_actor = reg.get<ActorComponent>(npc);
        auto& player_actor = reg.get<ActorComponent>(player);

        if (npc_actor.name.find("Kruppe") != std::string::npos || npc_actor.name.find("克鲁普") != std::string::npos) {
            if (choice_idx == 0) {
                // 获得大梦庇护
                player_actor.hp = std::min(player_actor.max_hp, player_actor.hp + 30);
                WarrenSystem::azath_house_alertness = std::max(0.0f, WarrenSystem::azath_house_alertness - 15.0f);
                out_logs.push_back(">>> 【大梦哲人恩赐】克鲁普塞给你一块热气腾腾的蜂蜜糕点!");
                out_logs.push_back("古泉法力抚平了达鲁基斯坦地脉异动: 阿扎斯警觉度回落至 " 
                                   + std::to_string(WarrenSystem::azath_house_alertness).substr(0, 4) + "%!");
            }
        } else if (npc_actor.name.find("Crokus") != std::string::npos || npc_actor.name.find("克罗库斯") != std::string::npos) {
            if (choice_idx == 0) {
                if (reg.has<PositionComponent>(player)) {
                    reg.get<PositionComponent>(player).rooftop_jump_range += 1;
                    out_logs.push_back(">>> 【飞贼捷径指引】克罗库斯传授了斜瓦抓取绝窍，卡拉姆屋顶飞跃距离 +1 格!");
                }
            }
        }
    }
};

} // namespace Malazan
