#pragma once
#include <SDL3/SDL.h>
#include "Core/ECS.hpp"
#include "Core/Types.hpp"
#include "Components/Components.hpp"
#include "Components/InventoryComponent.hpp"
#include "Components/OtataralComponent.hpp"
#include "Components/BossComponent.hpp"
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
#include "Story/StorylineSystem.hpp"
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
            "Malazan: Gardens of the Moon - Darujhistan Tactical Core",
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

        add_log("=== 达鲁基斯坦 (Darujhistan) 战术视口已启动 ===");
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

        switch (key) {
            case SDLK_ESCAPE: {
                if (m_show_inventory) {
                    m_show_inventory = false;
                } else {
                    SDL_Event quit_ev;
                    quit_ev.type = SDL_EVENT_QUIT;
                    SDL_PushEvent(&quit_ev);
                }
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
        float y = 35.0f;

        // 标题
        FontRenderer::draw_text(m_renderer, "DARUJHISTAN: CITY OF BLUE FIRE", hud_x, y, 2.2f, {230, 200, 120, 255});
        y += 35.0f;

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
        y += 54.0f;

        // 当前高程与迷雾模式指示
        std::string layer_str = (m_view_elevation == ElevationLevel::Rooftop)
            ? "[TAB] VIEW: ROOFTOP (SLATE & ALLEYS)"
            : "[TAB] VIEW: STREET (COBBLESTONES & LAMPS)";
        FontRenderer::draw_text(m_renderer, layer_str, hud_x, y, 1.6f, {0, 229, 255, 255});
        y += 20.0f;

        std::string fow_str = m_fog_enabled ? "[F] FOG OF WAR: ENABLED (SIGHT: 8 TILES)" : "[F] FOG OF WAR: OMNISCIENT (CHEAT)";
        FontRenderer::draw_text(m_renderer, fow_str, hud_x, y, 1.5f, m_fog_enabled ? SDL_Color{120, 255, 150, 255} : SDL_Color{255, 180, 50, 255});
        y += 24.0f;

        // 全局阿扎斯之屋威胁槽
        FontRenderer::draw_text(m_renderer, "AZATH HOUSE AWAKENING GAUGE:", hud_x, y, 1.6f, {200, 80, 80, 255});
        y += 20.0f;
        
        // 进度条背景
        SDL_FRect bar_bg{hud_x, y, 520.0f, 14.0f};
        SDL_SetRenderDrawColor(m_renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(m_renderer, &bar_bg);
        
        // 进度条前景
        float alert_pct = std::clamp(WarrenSystem::azath_house_alertness / 100.0f, 0.0f, 1.0f);
        SDL_FRect bar_fg{hud_x, y, 520.0f * alert_pct, 14.0f};
        SDL_SetRenderDrawColor(m_renderer, 220, 30, 30, 255);
        SDL_RenderFillRect(m_renderer, &bar_fg);
        y += 26.0f;

        // 龙之塔罗命理卡片
        const auto& card = DeckOfDragonsSystem::active_card;
        SDL_FRect tarot_bg{hud_x, y, 520.0f, 40.0f};
        SDL_SetRenderDrawColor(m_renderer, 32, 28, 20, 255);
        SDL_RenderFillRect(m_renderer, &tarot_bg);
        SDL_SetRenderDrawColor(m_renderer, 218, 165, 32, 255);
        SDL_RenderRect(m_renderer, &tarot_bg);

        std::string card_hdr = "[FATE CARD] " + card.title + " (" + card.patron_name + ")";
        FontRenderer::draw_text(m_renderer, card_hdr, hud_x + 8.0f, y + 5.0f, 1.4f, {255, 215, 0, 255});
        std::string card_fx = "FX: SPELL x" + std::to_string(card.warren_power_mult).substr(0, 3)
                              + " | STEALTH x" + std::to_string(card.stealth_mult).substr(0, 3)
                              + " | BACKFIRE x" + std::to_string(card.backfire_mult).substr(0, 3);
        FontRenderer::draw_text(m_renderer, card_fx, hud_x + 8.0f, y + 22.0f, 1.2f, {200, 200, 180, 255});
        y += 48.0f;

        // 角色状态卡片
        auto draw_actor_stat = [&](Entity e, const std::string& title, SDL_Color color) {
            if (!reg.has<ActorComponent>(e)) return;
            auto& a = reg.get<ActorComponent>(e);
            auto& p = reg.get<PositionComponent>(e);

            std::string status = title + " - HP: " + std::to_string(a.hp) + "/" + std::to_string(a.max_hp)
                                 + " [" + (p.elevation == ElevationLevel::Rooftop ? "ROOF" : "STREET") + "]";
            FontRenderer::draw_text(m_renderer, status, hud_x, y, 1.5f, color);
            y += 20.0f;
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
            FontRenderer::draw_text(m_renderer, boss_str, hud_x, y, 1.5f, {0, 229, 255, 255});
            y += 20.0f;
        }

        y += 10.0f;

        // 操作指引面板
        FontRenderer::draw_text(m_renderer, "TACTICAL COMMANDS:", hud_x, y, 1.6f, {255, 255, 255, 255});
        y += 18.0f;
        FontRenderer::draw_text(m_renderer, "[W,A,S,D] 3D LEAP / MOVE | [I] INVENTORY", hud_x, y, 1.35f, {180, 180, 180, 255});
        y += 16.0f;
        FontRenderer::draw_text(m_renderer, "[1] SHADOWFIRE | [2] BURNER | [3] PLUNGE", hud_x, y, 1.35f, {180, 120, 255, 255});
        y += 16.0f;
        FontRenderer::draw_text(m_renderer, "[6] HEAL (DENUL) | [7] THROW CUSSER", hud_x, y, 1.35f, {255, 160, 50, 255});
        y += 16.0f;
        FontRenderer::draw_text(m_renderer, "[4] DECK OF DRAGONS | [5] AZATH SEAL", hud_x, y, 1.35f, {255, 215, 0, 255});
        y += 24.0f;

        // 战术战斗实时滚动日志
        FontRenderer::draw_text(m_renderer, "--- TACTICAL COMBAT LOG ---", hud_x, y, 1.5f, {140, 140, 140, 255});
        y += 18.0f;

        for (const auto& log : m_logs) {
            FontRenderer::draw_text(m_renderer, "> " + log, hud_x, y, 1.35f, {220, 220, 220, 255});
            y += 15.0f;
        }
    }

    void render_inventory_modal(Registry& reg, Entity kalam) {
        // 半透明深色背景遮罩
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

    int m_width;
    int m_height;
    SDL_Window* m_window{nullptr};
    SDL_Renderer* m_renderer{nullptr};
    ElevationLevel m_view_elevation{ElevationLevel::Rooftop};
    std::deque<std::string> m_logs;
    std::vector<VisibilityState> m_fog_grid;
    bool m_fog_enabled{true};
    bool m_show_inventory{false};
};

} // namespace Malazan
