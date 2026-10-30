#pragma once
#include <SDL3/SDL.h>
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/OtataralComponent.hpp"
#include "Components/BossComponent.hpp"
#include "Components/DragnipurComponent.hpp"
#include "Components/FactionReputationComponent.hpp"
#include "Items/Item.hpp"
#include "World/DarujhistanMap.hpp"
#include "Render/Font8x8.hpp"
#include "Systems/TurnSystem.hpp"
#include "Systems/WarrenSystem.hpp"
#include "Systems/RooftopTacticsSystem.hpp"
#include "Systems/BlueFireGasSystem.hpp"
#include "Systems/DeckOfDragonsSystem.hpp"
#include "Systems/AISystem.hpp"
#include "Systems/FOVSystem.hpp"
#include "Systems/OtataralSystem.hpp"
#include "Systems/PathfindingSystem.hpp"
#include "Systems/DragnipurSystem.hpp"
#include "Systems/FactionSystem.hpp"
#include "Systems/InteractionSystem.hpp"
#include "Story/StorylineSystem.hpp"
#include "Story/DialogueSystem.hpp"
#include "Render/IsometricMath.hpp"

#include <vector>
#include <string>
#include <deque>
#include <cmath>
#include <algorithm>

namespace Malazan {

class GameViewport {
public:
    GameViewport(int width = 1440, int height = 860)
        : m_width(width), m_height(height) {}

    ~GameViewport() {
        if (m_renderer) SDL_DestroyRenderer(m_renderer);
        if (m_window) SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool init() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        m_window = SDL_CreateWindow(
            "Malazan: Gardens of the Moon - Darujhistan Tactical Core (Phase 2)",
            m_width, m_height,
            SDL_WINDOW_RESIZABLE
        );
        if (!m_window) {
            SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
            return false;
        }

        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer) {
            SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
            return false;
        }

        add_log("=== 达鲁基斯坦 (Darujhistan) 战术视口已启动 [Phase 2] ===");
        add_log("[E] 与周边 NPC (克鲁普/克罗库斯/雷克) 交谈");
        add_log("[8/X] 凝视巨剑【德拉格尼普尔】内部死界与拉车亡魂");
        add_log("[TAB] 切换 街面 / 屋顶 立体层级视口");
        add_log("[W,A,S,D] 控制卡拉姆移动/轻功飞跃");
        add_log("[F] 切换战争迷雾 (Fog of War) 开关");
        add_log("[I] 打开/关闭 破桥军战术行囊装备库");
        add_log("[1] 快班释放【暗影灼热风暴】");
        add_log("[2] 卡拉姆投掷莫兰斯燃烧弹诱爆蓝火");
        add_log("[3] 高空俯冲暗杀");
        add_log("[6] 饮下德努尔愈合合剂 (+45 HP)");
        add_log("[7] 投掷莫兰斯·卡瑟毁灭巨弹 (摧毁街区建筑)");
        add_log("[SPACE] 推进世界时间片");
        return true;
    }

    void add_log(const std::string& msg) {
        m_logs.push_back(msg);
        if (m_logs.size() > 14) {
            m_logs.pop_front();
        }
    }

    void update_fov(Registry& reg, const DarujhistanMap& map, Entity kalam) {
        if (!reg.has<PositionComponent>(kalam)) return;
        auto& pos = reg.get<PositionComponent>(kalam);
        FOVSystem::compute_fov(pos.coord, pos.elevation, 8, map, m_fog_grid);
    }

    void run(Registry& reg, DarujhistanMap& map, Entity player_kalam, Entity quick_ben, Entity claw_agent) {
        bool running = true;
        SDL_Event event;
        float pulse_timer = 0.0f;

        // 初始化视野
        update_fov(reg, map, player_kalam);

        while (running) {
            pulse_timer += 0.05f;

            // 事件处理
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                } else if (event.type == SDL_EVENT_KEY_DOWN) {
                    handle_key(event.key.key, reg, map, player_kalam, quick_ben, claw_agent);
                }
            }

            // 渲染
            render(reg, map, player_kalam, quick_ben, claw_agent, pulse_timer);
            SDL_Delay(16); // ~60 FPS
        }
    }

private:
    void handle_key(SDL_Keycode key, Registry& reg, DarujhistanMap& map, Entity kalam, Entity quick_ben, Entity claw) {
        bool action_took_turn = false;

        // 1. 若当前正处于对话交互模态中，数字键用于选择对话分支
        if (m_active_dialogue_npc != NULL_ENTITY) {
            if (key == SDLK_ESCAPE || key == SDLK_E) {
                m_active_dialogue_npc = NULL_ENTITY;
                add_log("[对话] 结束了与 NPC 的交谈。");
                return;
            } else if (key == SDLK_1 || key == SDLK_KP_1) {
                std::vector<std::string> choice_logs;
                DialogueSystem::apply_dialogue_choice(m_active_dialogue_npc, kalam, 0, reg, choice_logs);
                for (const auto& l : choice_logs) add_log(l);
                m_active_dialogue_npc = NULL_ENTITY;
                action_took_turn = true;
            } else if (key == SDLK_2 || key == SDLK_KP_2) {
                std::vector<std::string> choice_logs;
                DialogueSystem::apply_dialogue_choice(m_active_dialogue_npc, kalam, 1, reg, choice_logs);
                for (const auto& l : choice_logs) add_log(l);
                m_active_dialogue_npc = NULL_ENTITY;
                action_took_turn = true;
            } else if (key == SDLK_3 || key == SDLK_KP_3) {
                std::vector<std::string> choice_logs;
                DialogueSystem::apply_dialogue_choice(m_active_dialogue_npc, kalam, 2, reg, choice_logs);
                for (const auto& l : choice_logs) add_log(l);
                m_active_dialogue_npc = NULL_ENTITY;
                action_took_turn = true;
            }
            if (action_took_turn) {
                std::vector<std::string> ai_logs;
                AISystem::update_ai_turns(reg, map, kalam, ai_logs);
                for (const auto& l : ai_logs) add_log(l);
            }
            return;
        }

        switch (key) {
            case SDLK_ESCAPE: {
                if (m_show_inventory) {
                    m_show_inventory = false;
                } else if (m_show_dragnipur_wagon) {
                    m_show_dragnipur_wagon = false;
                } else {
                    SDL_Event quit_ev;
                    quit_ev.type = SDL_EVENT_QUIT;
                    SDL_PushEvent(&quit_ev);
                }
                break;
            }

            case SDLK_E: {
                // 与周边 NPC 对话交互
                Entity nearby = InteractionSystem::find_nearby_interactable(kalam, reg, 2);
                if (nearby != NULL_ENTITY) {
                    m_active_dialogue_npc = nearby;
                    m_current_dialogue = DialogueSystem::get_dialogue_for(nearby, kalam, reg);
                    add_log(">>> 与 [" + m_current_dialogue.speaker_name + "] 开启对话互动...");
                } else {
                    add_log("周边 2 格内没有可交互的同盟或市民 NPC。");
                }
                break;
            }

            case SDLK_8: case SDLK_X: {
                // 打开/关闭 德拉格尼普尔黑剑亡魂拉车界面
                m_show_dragnipur_wagon = !m_show_dragnipur_wagon;
                add_log(m_show_dragnipur_wagon 
                    ? "[太古神剑] 凝视德拉格尼普尔 (Dragnipur) 内部的纯暗大车与亡魂锁链" 
                    : "[太古神剑] 收敛心神，目光返回达鲁基斯坦街市");
                break;
            }

            case SDLK_TAB:
                // 切换显示高程
                m_view_elevation = (m_view_elevation == ElevationLevel::Street) 
                                   ? ElevationLevel::Rooftop 
                                   : ElevationLevel::Street;
                update_fov(reg, map, kalam);
                add_log(m_view_elevation == ElevationLevel::Rooftop 
                    ? "[视口] 切换至【屋顶层 (Rooftop)】- 飞贼与刺客通道"
                    : "[视口] 切换至【街面层 (Street)】- 鹅卵石路与巡逻");
                break;

            case SDLK_F:
                m_fog_enabled = !m_fog_enabled;
                add_log(m_fog_enabled 
                    ? "[战术系统] 战争迷雾已启用 (Fog of War: ON)" 
                    : "[战术系统] 战术透视全开 (Fog of War: OFF)");
                break;

            case SDLK_I:
                m_show_inventory = !m_show_inventory;
                add_log(m_show_inventory 
                    ? "[装备行囊] 已打开破桥军行囊与装备插槽" 
                    : "[装备行囊] 已关闭装备界面");
                break;

            case SDLK_W: case SDLK_UP:
                try_move_player(kalam, reg, map, {0, -1});
                update_fov(reg, map, kalam);
                action_took_turn = true;
                break;
            case SDLK_S: case SDLK_DOWN:
                try_move_player(kalam, reg, map, {0, 1});
                update_fov(reg, map, kalam);
                action_took_turn = true;
                break;
            case SDLK_A: case SDLK_LEFT:
                try_move_player(kalam, reg, map, {-1, 0});
                update_fov(reg, map, kalam);
                action_took_turn = true;
                break;
            case SDLK_D: case SDLK_RIGHT:
                try_move_player(kalam, reg, map, {1, 0});
                update_fov(reg, map, kalam);
                action_took_turn = true;
                break;

            case SDLK_1: {
                // 快班释放复合沃伦
                std::vector<SpellWeaveNode> weave = {
                    {WarrenType::Telas, 35.0f},
                    {WarrenType::Rashan, 25.0f}
                };
                auto& claw_pos = reg.get<PositionComponent>(claw);
                std::vector<std::string> spell_logs;
                WarrenSystem::cast_multi_warren_spell(quick_ben, reg, "暗影灼热风暴", weave, claw_pos.coord, &spell_logs);
                for (const auto& l : spell_logs) add_log(l);
                action_took_turn = true;
                break;
            }

            case SDLK_2: {
                // 卡拉姆投掷莫兰斯燃烧弹诱爆蓝火煤气
                std::vector<std::string> b_logs;
                BlueFireGasSystem::throw_moranth_munition(kalam, MoranthMunitionType::Burner, {10, 6}, reg, map, false, &b_logs);
                for (const auto& l : b_logs) add_log(l);
                action_took_turn = true;
                break;
            }

            case SDLK_3: {
                // 俯冲暗杀
                if (reg.get<ActorComponent>(claw).is_alive) {
                    std::vector<std::string> amb_logs;
                    RooftopTacticsSystem::execute_plunging_ambush(kalam, claw, reg, map, &amb_logs);
                    for (const auto& l : amb_logs) add_log(l);
                    
                    // 阵营声望变动结算
                    if (!reg.get<ActorComponent>(claw).is_alive) {
                        std::vector<std::string> f_logs;
                        FactionSystem::on_kill(kalam, claw, reg, f_logs);
                        for (const auto& fl : f_logs) add_log(fl);
                    }

                    update_fov(reg, map, kalam);
                    action_took_turn = true;
                } else {
                    add_log("目标已阵亡，无需再次刺杀。");
                }
                break;
            }

            case SDLK_4: {
                const auto& card = DeckOfDragonsSystem::draw_next_card();
                add_log(">>> 翻开龙之塔罗牌: 【" + card.title + "】(" + card.patron_name + ")");
                add_log("神谕: " + card.omen_description);
                action_took_turn = true;
                break;
            }

            case SDLK_5: {
                // 触发第三幕阿扎斯终极神迹封印
                std::vector<std::string> story_logs;
                StorylineSystem::trigger_azath_sealing(reg, map, story_logs);
                for (const auto& l : story_logs) {
                    add_log(l);
                }
                action_took_turn = true;
                break;
            }

            case SDLK_6: {
                // 饮下德努尔愈合合剂
                if (reg.has<ActorComponent>(kalam)) {
                    auto& actor = reg.get<ActorComponent>(kalam);
                    if (reg.has<InventoryComponent>(kalam)) {
                        auto& inv = reg.get<InventoryComponent>(kalam);
                        bool used = false;
                        for (size_t i = 0; i < inv.backpack.size(); ++i) {
                            if (inv.backpack[i].id == "denul_draught") {
                                actor.hp = std::min(actor.max_hp, actor.hp + inv.backpack[i].heal_amount);
                                inv.backpack[i].charges--;
                                if (inv.backpack[i].charges <= 0) {
                                    inv.remove_item(i);
                                }
                                used = true;
                                break;
                            }
                        }
                        if (used) {
                            add_log(">>> 卡拉姆饮下【德努尔愈合合剂】! 沃伦生机注入，生命恢复至 " 
                                    + std::to_string(actor.hp) + "/" + std::to_string(actor.max_hp));
                            action_took_turn = true;
                        } else {
                            add_log("行囊中已无德努尔合剂。");
                        }
                    }
                }
                break;
            }

            case SDLK_7: {
                // 投掷莫兰斯·卡瑟毁灭巨弹 (Cusser)
                Vec2i blast_target = {10, 6};
                if (StorylineSystem::boss_raest_entity != NULL_ENTITY && reg.has<PositionComponent>(StorylineSystem::boss_raest_entity)) {
                    blast_target = reg.get<PositionComponent>(StorylineSystem::boss_raest_entity).coord;
                }
                std::vector<std::string> cusser_logs;
                BlueFireGasSystem::throw_moranth_munition(kalam, MoranthMunitionType::Sharper, blast_target, reg, map, true, &cusser_logs);
                for (const auto& l : cusser_logs) add_log(l);
                action_took_turn = true;
                break;
            }

            case SDLK_SPACE:
                TurnSystem::global_tick_count++;
                add_log("时间片推移... (Tick +" + std::to_string(TurnSystem::global_tick_count) + ")");
                action_took_turn = true;
                break;

            default:
                break;
        }

        // 玩家行动消耗时间片后，驱动全城敌对 AI 状态机更新与主线目标推进
        if (action_took_turn) {
            std::vector<std::string> ai_logs;
            AISystem::update_ai_turns(reg, map, kalam, ai_logs);
            for (const auto& l : ai_logs) {
                add_log(l);
            }

            std::vector<std::string> story_logs;
            StorylineSystem::check_progression(reg, map, story_logs);
            for (const auto& l : story_logs) {
                add_log(l);
            }
        }
    }

    void try_move_player(Entity ent, Registry& reg, DarujhistanMap& map, Vec2i delta) {
        auto& pos = reg.get<PositionComponent>(ent);
        Vec2i target = pos.coord + delta;
        if (!map.in_bounds(target)) return;

        if (pos.elevation == ElevationLevel::Rooftop) {
            std::vector<std::string> leap_logs;
            RooftopTacticsSystem::attempt_rooftop_leap(ent, reg, map, target, &leap_logs);
            for (const auto& l : leap_logs) add_log(l);
        } else {
            const auto& cell = map.get_cell(target, ElevationLevel::Street);
            if (!cell.blocks_movement) {
                pos.coord = target;
            } else {
                add_log("前方是坚实建筑外墙，无法穿透。");
            }
        }
    }

    void render(Registry& reg, DarujhistanMap& map, Entity kalam, Entity quick_ben, Entity claw, float pulse) {
        // 清屏（达鲁基斯坦午夜深蓝黑夜空）
        SDL_SetRenderDrawColor(m_renderer, 10, 13, 18, 255);
        SDL_RenderClear(m_renderer);

        // 1. 渲染 2.5D 等距立体世界 (3D 建筑挤出、战争迷雾、奥塔拉尔死域、蓝火动态漫射、角色立柱)
        render_iso_world(reg, map, pulse);

        // 2. 渲染右侧 HUD 与信息面板
        render_hud(reg, kalam, quick_ben, claw);

        // 3. 渲染行囊物品库模态抽屉
        if (m_show_inventory) {
            render_inventory_modal(reg, kalam);
        }

        // 4. 渲染 NPC 剧情对话框
        if (m_active_dialogue_npc != NULL_ENTITY) {
            render_dialogue_modal();
        }

        // 5. 渲染德拉格尼普尔黑剑大车囚界模态
        if (m_show_dragnipur_wagon) {
            render_dragnipur_modal(reg);
        }

        SDL_RenderPresent(m_renderer);
    }

    void render_iso_world(Registry& reg, DarujhistanMap& map, float pulse) {
        float origin_x = 440.0f;
        float origin_y = 120.0f;
        int32_t w = map.width();
        int32_t h = map.height();

        // A. 基础地面层 (Z = 0)：结合战争迷雾渲染等距鹅卵石网格
        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                Vec2i p{x, y};
                size_t fog_idx = static_cast<size_t>(y * w + x);
                VisibilityState v_state = (m_fog_enabled && fog_idx < m_fog_grid.size())
                                          ? m_fog_grid[fog_idx]
                                          : VisibilityState::Visible;

                if (v_state == VisibilityState::Unexplored) {
                    // 未探索区域：绘制深邃无光地块
                    SDL_FColor dark_col{0.03f, 0.04f, 0.05f, 1.0f};
                    IsometricRenderer::draw_iso_tile(m_renderer, static_cast<float>(x), static_cast<float>(y), 
                                                    0.0f, origin_x, origin_y, dark_col);
                    continue;
                }

                const auto& street_cell = map.get_cell(p, ElevationLevel::Street);
                bool checker = ((x + y) % 2 == 0);
                SDL_FColor street_col = checker 
                    ? SDL_FColor{0.14f, 0.16f, 0.20f, 1.0f} 
                    : SDL_FColor{0.11f, 0.13f, 0.16f, 1.0f};

                if (street_cell.terrain == TileTerrain::NarrowAlley) {
                    street_col = {0.18f, 0.14f, 0.12f, 1.0f}; // 阴暗瓦砾小巷
                }

                // 记忆区 (Explored) 色彩暗化与冷灰处理
                if (v_state == VisibilityState::Explored) {
                    street_col.r *= 0.28f;
                    street_col.g *= 0.28f;
                    street_col.b *= 0.28f;
                }

                IsometricRenderer::draw_iso_tile(m_renderer, static_cast<float>(x), static_cast<float>(y), 
                                                0.0f, origin_x, origin_y, street_col);
            }
        }

        // B. 绘制所有激活的奥塔拉尔反魔绝死力场 (生锈赭红微光脉冲)
        auto ota_fields = OtataralSystem::get_active_fields(reg);
        for (const auto& field : ota_fields) {
            size_t c_idx = static_cast<size_t>(field.center.y * w + field.center.x);
            VisibilityState c_state = (m_fog_enabled && c_idx < m_fog_grid.size())
                                      ? m_fog_grid[c_idx]
                                      : VisibilityState::Visible;
            if (c_state != VisibilityState::Unexplored) {
                float gz = (field.elevation == ElevationLevel::Rooftop) ? 1.0f : 0.0f;
                IsometricRenderer::draw_iso_otataral_field(
                    m_renderer,
                    static_cast<float>(field.center.x),
                    static_cast<float>(field.center.y),
                    gz, origin_x, origin_y,
                    field.radius, pulse
                );
            }
        }

        // C. 深度排序绘制：3D 挤出立体建筑、实体 Token、蓝火煤气路灯
        for (int32_t y = 0; y < h; ++y) {
            for (int32_t x = 0; x < w; ++x) {
                Vec2i p{x, y};
                size_t fog_idx = static_cast<size_t>(y * w + x);
                VisibilityState v_state = (m_fog_enabled && fog_idx < m_fog_grid.size())
                                          ? m_fog_grid[fog_idx]
                                          : VisibilityState::Visible;

                if (v_state == VisibilityState::Unexplored) continue;

                const auto& street_cell = map.get_cell(p, ElevationLevel::Street);
                const auto& roof_cell = map.get_cell(p, ElevationLevel::Rooftop);

                // 1. 绘制 3D 挤出立体建筑方体 (受光面、阴影面、屋顶顶面)
                if (street_cell.terrain == TileTerrain::BuildingWall) {
                    bool is_terrace = (roof_cell.terrain == TileTerrain::FlatTerrace);
                    bool is_collapsed = roof_cell.roof_collapsed;
                    IsometricRenderer::draw_iso_building_block(m_renderer, static_cast<float>(x), static_cast<float>(y), 
                                                              origin_x, origin_y, is_terrace, is_collapsed);
                }

                // 2. 绘制 2.5D 蓝火煤气路灯与大范围动态光晕 (仅在可见或探索区)
                for (auto [node_ent, node] : reg.view<BlueFireNodeComponent>()) {
                    if (node.coord == p && node.is_lit) {
                        IsometricRenderer::draw_iso_blue_gas_lamp(m_renderer, static_cast<float>(x), static_cast<float>(y), 
                                                                 origin_x, origin_y, pulse);
                    }
                }

                // 3. 绘制 3D 实体角色立柱与高度层级
                for (auto [ent, pos, actor] : reg.view<PositionComponent, ActorComponent>()) {
                    if (pos.coord == p) {
                        // 迷雾隐藏机制：非视线内 (Visible) 的存活敌对目标不予渲染！
                        if (m_fog_enabled && v_state != VisibilityState::Visible && 
                            actor.faction != Faction::Bridgeburners && actor.is_alive) {
                            continue;
                        }

                        // 角色在屋顶层时，Z 轴物理升高至 1.0f (立于 3D 建筑屋顶顶面之上)
                        float gz = (pos.elevation == ElevationLevel::Rooftop) ? 1.0f : 0.0f;

                        if (!actor.is_alive) {
                            SDL_FPoint ground = IsometricRenderer::world_to_screen(static_cast<float>(x), static_cast<float>(y), 
                                                                                  gz, origin_x, origin_y);
                            FontRenderer::draw_char(m_renderer, 'X', ground.x - 6.0f, ground.y - 12.0f, 2.0f, {120, 120, 120, 180});
                            continue;
                        }

                        SDL_FColor color{1, 1, 1, 1};
                        char glyph = '?';
                        float scale = 1.0f;

                        if (actor.faction == Faction::Bridgeburners) {
                            if (actor.name.find("Kalam") != std::string::npos) {
                                color = {0.15f, 0.70f, 0.25f, 1.0f}; // 刺客翠绿
                                glyph = 'K';
                            } else {
                                color = {0.60f, 0.20f, 0.90f, 1.0f}; // 法师深紫
                                glyph = 'Q';
                            }
                        } else if (actor.faction == Faction::ImperialClaw) {
                            color = {0.85f, 0.15f, 0.15f, 1.0f}; // 帝国猩红
                            glyph = 'C';
                        } else if (actor.faction == Faction::JaghutTyrantRaest) {
                            color = {0.0f, 0.85f, 1.0f, 1.0f}; // 暴君太古极冰青蓝
                            glyph = 'R';
                            scale = 1.6f; // 首领体型显著更大
                        } else if (actor.faction == Faction::PhoenixInnCircle) {
                            if (actor.name.find("Kruppe") != std::string::npos || actor.name.find("克鲁普") != std::string::npos) {
                                color = {0.95f, 0.75f, 0.20f, 1.0f}; // 克鲁普哲人金
                                glyph = 'P';
                                scale = 1.25f;
                            } else {
                                color = {0.85f, 0.50f, 0.20f, 1.0f}; // 克罗库斯飞贼黄铜橙
                                glyph = 'O';
                            }
                        } else if (actor.faction == Faction::TisteAndii) {
                            color = {0.20f, 0.15f, 0.35f, 1.0f}; // 雷克纯暗紫黑
                            glyph = 'A';
                            scale = 1.4f;
                        } else if (actor.faction == Faction::CityAssassinsGuild) {
                            color = {0.45f, 0.65f, 0.25f, 1.0f}; // 刺客公会暗影绿
                            glyph = 'C';
                        } else if (actor.faction == Faction::NeutralCivilians) {
                            color = {0.60f, 0.65f, 0.70f, 1.0f}; // 城防军卫兵钢灰
                            glyph = 'G';
                        }

                        IsometricRenderer::draw_iso_character_token(m_renderer, static_cast<float>(x), static_cast<float>(y), 
                                                                   gz, origin_x, origin_y, color, glyph, scale);

                        // 绘制 3D 浮空生命条 (Floating Health Bar)
                        SDL_FPoint head = IsometricRenderer::world_to_screen(static_cast<float>(x), static_cast<float>(y), 
                                                                            gz, origin_x, origin_y);
                        float bar_y = head.y - (32.0f * scale);
                        float bar_w = 26.0f * scale;
                        SDL_FRect hp_bg{head.x - bar_w * 0.5f, bar_y, bar_w, 4.0f};
                        SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 200);
                        SDL_RenderFillRect(m_renderer, &hp_bg);

                        float hp_pct = std::clamp(static_cast<float>(actor.hp) / actor.max_hp, 0.0f, 1.0f);
                        SDL_FRect hp_fg{head.x - bar_w * 0.5f, bar_y, bar_w * hp_pct, 4.0f};
                        SDL_SetRenderDrawColor(m_renderer, 50, 220, 90, 255);
                        SDL_RenderFillRect(m_renderer, &hp_fg);

                        // AI 警觉图标
                        if (reg.has<AIComponent>(ent)) {
                            auto& aic = reg.get<AIComponent>(ent);
                            if (aic.state == AIBehaviorState::Combat) {
                                FontRenderer::draw_char(m_renderer, '!', head.x + bar_w * 0.5f + 2.0f, bar_y - 6.0f, 1.5f, {255, 40, 40, 255});
                            } else if (aic.state == AIBehaviorState::Suspicious) {
                                FontRenderer::draw_char(m_renderer, '?', head.x + bar_w * 0.5f + 2.0f, bar_y - 6.0f, 1.5f, {255, 215, 0, 255});
                            }
                        }
                    }
                }
            }
        }

        // D. 极寒暴风雪大气粒子特效（暴君出世时席卷达鲁基斯坦）
        if (StorylineSystem::current_act >= StoryAct::Act2_TyrantAwakening) {
            static std::vector<SDL_FPoint> snowflakes;
            if (snowflakes.empty()) {
                for (int i = 0; i < 90; ++i) {
                    snowflakes.push_back({static_cast<float>(rand() % 880), static_cast<float>(rand() % 860)});
                }
            }
            SDL_SetRenderDrawColor(m_renderer, 200, 240, 255, 180);
            for (auto& s : snowflakes) {
                s.x -= 2.6f;
                s.y += 1.8f;
                if (s.x < 0) s.x += 880;
                if (s.y > 860) s.y -= 860;
                SDL_FRect flake{s.x, s.y, 2.0f, 2.0f};
                SDL_RenderFillRect(m_renderer, &flake);
            }
        }
    }

    void render_hud(Registry& reg, Entity kalam, Entity quick_ben, Entity claw) {
        float hud_x = 880.0f;
        float y = 30.0f;

        // 标题
        FontRenderer::draw_text(m_renderer, "DARUJHISTAN: CITY OF BLUE FIRE", hud_x, y, 2.2f, {230, 200, 120, 255});
        y += 34.0f;

        // 主线战役任务追踪指示 (Main Storyline Quest Tracker)
        SDL_FRect quest_bg{hud_x, y, 520.0f, 44.0f};
        SDL_SetRenderDrawColor(m_renderer, 20, 28, 42, 255);
        SDL_RenderFillRect(m_renderer, &quest_bg);
        SDL_SetRenderDrawColor(m_renderer, 0, 180, 255, 255);
        SDL_RenderRect(m_renderer, &quest_bg);

        std::string act_title = "[MAIN QUEST] " + StorylineSystem::get_act_title();
        FontRenderer::draw_text(m_renderer, act_title, hud_x + 8.0f, y + 6.0f, 1.45f, {0, 229, 255, 255});
        std::string obj_text = "OBJ: " + StorylineSystem::get_current_objective();
        FontRenderer::draw_text(m_renderer, obj_text, hud_x + 8.0f, y + 24.0f, 1.2f, {220, 220, 220, 255});
        y += 52.0f;

        // 阵营声望矩阵显示 (Faction Reputation Status)
        if (reg.has<FactionReputationComponent>(kalam)) {
            auto& rep = reg.get<FactionReputationComponent>(kalam);
            std::string rep_str = "[REP] PHOENIX: " + std::to_string(rep.get(Faction::PhoenixInnCircle))
                                + " | GUILD: " + std::to_string(rep.get(Faction::CityAssassinsGuild))
                                + " | CLAW: " + std::to_string(rep.get(Faction::ImperialClaw));
            FontRenderer::draw_text(m_renderer, rep_str, hud_x, y, 1.4f, {200, 230, 180, 255});
            y += 20.0f;
        }

        // 附近 NPC 交互提示
        Entity nearby = InteractionSystem::find_nearby_interactable(kalam, reg, 2);
        if (nearby != NULL_ENTITY && reg.has<ActorComponent>(nearby)) {
            std::string talk_prompt = ">>> [E] TALK TO: " + reg.get<ActorComponent>(nearby).name;
            FontRenderer::draw_text(m_renderer, talk_prompt, hud_x, y, 1.45f, {255, 220, 0, 255});
            y += 20.0f;
        }

        // 当前高程与迷雾模式指示
        std::string layer_str = (m_view_elevation == ElevationLevel::Rooftop)
            ? "[TAB] VIEW: ROOFTOP (SLATE & ALLEYS)"
            : "[TAB] VIEW: STREET (COBBLESTONES & LAMPS)";
        FontRenderer::draw_text(m_renderer, layer_str, hud_x, y, 1.5f, {0, 229, 255, 255});
        y += 18.0f;

        std::string fow_str = m_fog_enabled ? "[F] FOG OF WAR: ENABLED" : "[F] FOG OF WAR: OMNISCIENT";
        FontRenderer::draw_text(m_renderer, fow_str, hud_x, y, 1.4f, m_fog_enabled ? SDL_Color{120, 255, 150, 255} : SDL_Color{255, 180, 50, 255});
        y += 22.0f;

        // 全局阿扎斯之屋威胁槽
        FontRenderer::draw_text(m_renderer, "AZATH HOUSE AWAKENING GAUGE:", hud_x, y, 1.5f, {200, 80, 80, 255});
        y += 18.0f;
        
        // 进度条背景
        SDL_FRect bar_bg{hud_x, y, 520.0f, 12.0f};
        SDL_SetRenderDrawColor(m_renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(m_renderer, &bar_bg);
        
        // 进度条前景
        float alert_pct = std::clamp(WarrenSystem::azath_house_alertness / 100.0f, 0.0f, 1.0f);
        SDL_FRect bar_fg{hud_x, y, 520.0f * alert_pct, 12.0f};
        SDL_SetRenderDrawColor(m_renderer, 220, 30, 30, 255);
        SDL_RenderFillRect(m_renderer, &bar_fg);
        y += 22.0f;

        // 龙之塔罗命理卡片
        const auto& card = DeckOfDragonsSystem::active_card;
        SDL_FRect tarot_bg{hud_x, y, 520.0f, 38.0f};
        SDL_SetRenderDrawColor(m_renderer, 32, 28, 20, 255);
        SDL_RenderFillRect(m_renderer, &tarot_bg);
        SDL_SetRenderDrawColor(m_renderer, 218, 165, 32, 255);
        SDL_RenderRect(m_renderer, &tarot_bg);

        std::string card_hdr = "[FATE CARD] " + card.title + " (" + card.patron_name + ")";
        FontRenderer::draw_text(m_renderer, card_hdr, hud_x + 8.0f, y + 4.0f, 1.35f, {255, 215, 0, 255});
        std::string card_fx = "FX: SPELL x" + std::to_string(card.warren_power_mult).substr(0, 3)
                              + " | STEALTH x" + std::to_string(card.stealth_mult).substr(0, 3)
                              + " | BACKFIRE x" + std::to_string(card.backfire_mult).substr(0, 3);
        FontRenderer::draw_text(m_renderer, card_fx, hud_x + 8.0f, y + 20.0f, 1.15f, {200, 200, 180, 255});
        y += 44.0f;

        // 角色状态卡片
        auto draw_actor_stat = [&](Entity e, const std::string& title, SDL_Color color) {
            if (!reg.has<ActorComponent>(e)) return;
            auto& a = reg.get<ActorComponent>(e);
            auto& p = reg.get<PositionComponent>(e);

            std::string status = title + " - HP: " + std::to_string(a.hp) + "/" + std::to_string(a.max_hp)
                                 + " [" + (p.elevation == ElevationLevel::Rooftop ? "ROOF" : "STREET") + "]";
            FontRenderer::draw_text(m_renderer, status, hud_x, y, 1.45f, color);
            y += 18.0f;
        };

        draw_actor_stat(kalam, "[K] KALAM MEKHAR", {80, 220, 100, 255});
        draw_actor_stat(quick_ben, "[Q] QUICK BEN", {180, 120, 255, 255});
        draw_actor_stat(claw, "[C] IMPERIAL CLAW", {255, 100, 100, 255});

        // 贾格特暴君状态
        if (StorylineSystem::boss_raest_entity != NULL_ENTITY && reg.has<ActorComponent>(StorylineSystem::boss_raest_entity)) {
            auto& boss_a = reg.get<ActorComponent>(StorylineSystem::boss_raest_entity);
            auto& boss_c = reg.get<JaghutTyrantComponent>(StorylineSystem::boss_raest_entity);
            std::string boss_str = "[BOSS: RAEST] HP: " + std::to_string(boss_a.hp) + "/" + std::to_string(boss_a.max_hp)
                                   + " | FROST ARMOR: " + std::to_string(boss_c.frost_armor);
            FontRenderer::draw_text(m_renderer, boss_str, hud_x, y, 1.45f, {0, 229, 255, 255});
            y += 18.0f;
        }

        y += 6.0f;

        // 操作指引面板
        FontRenderer::draw_text(m_renderer, "TACTICAL COMMANDS:", hud_x, y, 1.55f, {255, 255, 255, 255});
        y += 18.0f;
        FontRenderer::draw_text(m_renderer, "[W,A,S,D] 3D MOVE | [E] TALK NPC | [I] INVENTORY", hud_x, y, 1.35f, {180, 180, 180, 255});
        y += 15.0f;
        FontRenderer::draw_text(m_renderer, "[1] SHADOWFIRE | [2] BURNER | [3] PLUNGE", hud_x, y, 1.35f, {180, 120, 255, 255});
        y += 15.0f;
        FontRenderer::draw_text(m_renderer, "[6] HEAL | [7] CUSSER | [8/X] DRAGNIPUR", hud_x, y, 1.35f, {255, 160, 50, 255});
        y += 15.0f;
        FontRenderer::draw_text(m_renderer, "[4] DECK OF DRAGONS | [5] AZATH SEAL", hud_x, y, 1.35f, {255, 215, 0, 255});
        y += 22.0f;

        // 战术战斗实时滚动日志
        FontRenderer::draw_text(m_renderer, "--- TACTICAL COMBAT LOG ---", hud_x, y, 1.5f, {140, 140, 140, 255});
        y += 18.0f;

        for (const auto& log : m_logs) {
            FontRenderer::draw_text(m_renderer, "> " + log, hud_x, y, 1.3f, {220, 220, 220, 255});
            y += 14.0f;
        }
    }

    void render_inventory_modal(Registry& reg, Entity kalam) {
        SDL_FRect overlay{120.0f, 80.0f, 680.0f, 640.0f};
        SDL_SetRenderDrawColor(m_renderer, 15, 20, 28, 240);
        SDL_RenderFillRect(m_renderer, &overlay);
        SDL_SetRenderDrawColor(m_renderer, 218, 165, 32, 255);
        SDL_RenderRect(m_renderer, &overlay);

        float px = 145.0f;
        float py = 110.0f;

        FontRenderer::draw_text(m_renderer, "=== BRIDGEBURNERS TACTICAL INVENTORY ===", px, py, 1.8f, {255, 215, 0, 255});
        py += 35.0f;

        if (!reg.has<InventoryComponent>(kalam)) {
            FontRenderer::draw_text(m_renderer, "No inventory component found.", px, py, 1.5f, {200, 200, 200, 255});
            return;
        }

        auto& inv = reg.get<InventoryComponent>(kalam);

        // 装备槽位展示
        FontRenderer::draw_text(m_renderer, "[EQUIPPED WEAPONS & ARMOR]", px, py, 1.6f, {0, 229, 255, 255});
        py += 24.0f;

        std::string main_h = "MAIN HAND: " + (inv.equipped_main_hand.has_value() 
                             ? inv.equipped_main_hand->name + " (ATK: " + std::to_string(inv.equipped_main_hand->damage) + " | OTATARAL)" 
                             : "None");
        FontRenderer::draw_text(m_renderer, main_h, px + 10.0f, py, 1.45f, {255, 120, 60, 255});
        py += 22.0f;

        std::string off_h = "OFF HAND : " + (inv.equipped_off_hand.has_value() 
                            ? inv.equipped_off_hand->name + " (ATK: " + std::to_string(inv.equipped_off_hand->damage) + ")" 
                            : "None");
        FontRenderer::draw_text(m_renderer, off_h, px + 10.0f, py, 1.45f, {180, 220, 180, 255});
        py += 32.0f;

        // 奥塔拉尔力场状态
        FontRenderer::draw_text(m_renderer, "[ANTI-MAGIC FIELD STATUS]", px, py, 1.6f, {255, 100, 50, 255});
        py += 22.0f;
        std::string ota_status = "OTATARAL AURA: RADIUS " + std::to_string(inv.get_max_otataral_radius()).substr(0, 3) + " TILES | DEADENING ACTIVE";
        FontRenderer::draw_text(m_renderer, ota_status, px + 10.0f, py, 1.4f, {255, 160, 100, 255});
        py += 35.0f;

        // 背包物品列表
        FontRenderer::draw_text(m_renderer, "[BACKPACK MUNITIONS & SUPPLIES] (" + std::to_string(inv.backpack.size()) + "/" + std::to_string(inv.max_capacity) + ")", px, py, 1.6f, {230, 200, 120, 255});
        py += 26.0f;

        for (size_t i = 0; i < inv.backpack.size(); ++i) {
            const auto& item = inv.backpack[i];
            std::string item_str = "[" + std::to_string(i + 1) + "] " + item.name + " (x" + std::to_string(item.charges) + ")";
            FontRenderer::draw_text(m_renderer, item_str, px + 10.0f, py, 1.4f, {220, 220, 220, 255});
            py += 18.0f;
            FontRenderer::draw_text(m_renderer, "    " + item.description.substr(0, 50) + "...", px + 10.0f, py, 1.15f, {150, 150, 150, 255});
            py += 24.0f;
        }

        py += 20.0f;
        FontRenderer::draw_text(m_renderer, "PRESS [6] DRINK DENUL | [7] THROW CUSSER | [I/ESC] CLOSE", px, py, 1.4f, {255, 215, 0, 255});
    }

    void render_dialogue_modal() {
        SDL_FRect box{160.0f, 160.0f, 720.0f, 420.0f};
        SDL_SetRenderDrawColor(m_renderer, 18, 22, 32, 250);
        SDL_RenderFillRect(m_renderer, &box);
        SDL_SetRenderDrawColor(m_renderer, 255, 215, 0, 255);
        SDL_RenderRect(m_renderer, &box);

        float dx = 190.0f;
        float dy = 190.0f;

        // 说话者与头衔
        std::string title = "[CONVERSATION] " + m_current_dialogue.speaker_name;
        FontRenderer::draw_text(m_renderer, title, dx, dy, 1.8f, {255, 215, 0, 255});
        dy += 24.0f;
        FontRenderer::draw_text(m_renderer, m_current_dialogue.speaker_title, dx, dy, 1.3f, {0, 229, 255, 255});
        dy += 34.0f;

        // 对话文本（多行按换行符拆分）
        std::string txt = m_current_dialogue.dialogue_text;
        size_t start = 0;
        while (start < txt.size()) {
            size_t end = txt.find('\n', start);
            if (end == std::string::npos) end = txt.size();
            std::string line = txt.substr(start, end - start);
            FontRenderer::draw_text(m_renderer, line, dx, dy, 1.35f, {230, 230, 230, 255});
            dy += 20.0f;
            start = end + 1;
        }

        dy += 20.0f;
        FontRenderer::draw_text(m_renderer, "--- CHOOSE RESPONSE ---", dx, dy, 1.4f, {180, 180, 180, 255});
        dy += 24.0f;

        // 选项列表
        for (const auto& opt : m_current_dialogue.options) {
            FontRenderer::draw_text(m_renderer, opt, dx, dy, 1.35f, {255, 230, 120, 255});
            dy += 24.0f;
        }

        dy += 16.0f;
        FontRenderer::draw_text(m_renderer, "PRESS [1, 2, 3] TO SELECT | [E/ESC] EXIT", dx, dy, 1.3f, {150, 150, 150, 255});
    }

    void render_dragnipur_modal(Registry& reg) {
        SDL_FRect box{140.0f, 100.0f, 760.0f, 580.0f};
        SDL_SetRenderDrawColor(m_renderer, 10, 8, 16, 250);
        SDL_RenderFillRect(m_renderer, &box);
        SDL_SetRenderDrawColor(m_renderer, 180, 160, 220, 255);
        SDL_RenderRect(m_renderer, &box);

        float dx = 170.0f;
        float dy = 130.0f;

        FontRenderer::draw_text(m_renderer, "=== DRAGNIPUR: THE WAGON OF DARKNESS ===", dx, dy, 1.8f, {210, 180, 255, 255});
        dy += 28.0f;
        FontRenderer::draw_text(m_renderer, "Elder Sword forged by Draconus | Gate of Kurald Galain", dx, dy, 1.35f, {140, 120, 180, 255});
        dy += 34.0f;

        // 查找持有 Dragnipur 的实体（如雷克）
        bool found = false;
        for (auto [ent, drag, actor] : reg.view<DragnipurComponent, ActorComponent>()) {
            found = true;
            std::string bearer = "SWORD BEARER: " + actor.name;
            FontRenderer::draw_text(m_renderer, bearer, dx, dy, 1.5f, {255, 215, 0, 255});
            dy += 26.0f;

            // 混沌追逐距离指示
            FontRenderer::draw_text(m_renderer, "DISTANCE FROM CHAOS (SAFETY MARGIN):", dx, dy, 1.45f, {255, 100, 100, 255});
            dy += 18.0f;

            SDL_FRect c_bg{dx, dy, 680.0f, 14.0f};
            SDL_SetRenderDrawColor(m_renderer, 40, 30, 45, 255);
            SDL_RenderFillRect(m_renderer, &c_bg);

            float c_pct = std::clamp(drag.chaos_pursuit_distance / 100.0f, 0.0f, 1.0f);
            SDL_FRect c_fg{dx, dy, 680.0f * c_pct, 14.0f};
            SDL_SetRenderDrawColor(m_renderer, 160, 60, 240, 255);
            SDL_RenderFillRect(m_renderer, &c_fg);
            dy += 24.0f;

            std::string p_info = "TOTAL WAGON PULL POWER: " + std::to_string(drag.get_total_pull_power()).substr(0, 5)
                                 + " | WAGON WEIGHT: " + std::to_string(drag.wagon_weight).substr(0, 5);
            FontRenderer::draw_text(m_renderer, p_info, dx, dy, 1.4f, {200, 200, 240, 255});
            dy += 32.0f;

            FontRenderer::draw_text(m_renderer, "[ENCHAINED SOULS PULLING THE WAGON]:", dx, dy, 1.55f, {0, 229, 255, 255});
            dy += 24.0f;

            for (size_t i = 0; i < drag.trapped_souls.size(); ++i) {
                const auto& s = drag.trapped_souls[i];
                std::string s_line = "[" + std::to_string(i + 1) + "] " + s.name 
                                     + " (POWER: " + std::to_string(s.combat_power) 
                                     + " | PULL STR: " + std::to_string(s.pull_effort).substr(0, 4) + ")";
                FontRenderer::draw_text(m_renderer, s_line, dx + 10.0f, dy, 1.35f, {220, 220, 220, 255});
                dy += 20.0f;
            }
            break;
        }

        if (!found) {
            FontRenderer::draw_text(m_renderer, "Dragnipur is currently sleeping in the shadows.", dx, dy, 1.4f, {160, 160, 160, 255});
        }

        dy += 30.0f;
        FontRenderer::draw_text(m_renderer, "PRESS [8/X/ESC] TO CLOSE WAGON VIEW", dx, dy, 1.4f, {255, 215, 0, 255});
    }

    int m_width;
    int m_height;
    SDL_Window* m_window{nullptr};
    SDL_Renderer* m_renderer{nullptr};
    ElevationLevel m_view_elevation{ElevationLevel::Rooftop};
    std::deque<std::string> m_logs;
    std::vector<VisibilityState> m_fog_grid;
    bool m_fog_enabled{true};
    bool m_show_inventory{false};

    // Phase 2 新增：NPC 剧情交互与德拉格尼普尔状态
    Entity m_active_dialogue_npc{NULL_ENTITY};
    DialogueNode m_current_dialogue;
    bool m_show_dragnipur_wagon{false};
};

} // namespace Malazan
