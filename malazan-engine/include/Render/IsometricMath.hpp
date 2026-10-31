#pragma once
#include <SDL3/SDL.h>
#include "Core/Types.hpp"
#include <cmath>
#include <vector>

namespace Malazan {

// 2.5D 等距视角投影 (Isometric 2:1 Projection) 与几何着色工具
class IsometricRenderer {
public:
    static constexpr float TILE_W = 54.0f;       // 菱形瓦片水平对角线宽
    static constexpr float TILE_H = 27.0f;       // 菱形瓦片垂直对角线高
    static constexpr float ELEVATION_STEP = 36.0f; // 屋顶相对于街面的 3D 高度拉伸像素

    // 将 (GridX, GridY, ElevationZ) 转换为屏幕空间 2D 坐标
    static SDL_FPoint world_to_screen(float gx, float gy, float gz, float origin_x, float origin_y) {
        float sx = (gx - gy) * (TILE_W * 0.5f) + origin_x;
        float sy = (gx + gy) * (TILE_H * 0.5f) - (gz * ELEVATION_STEP) + origin_y;
        return {sx, sy};
    }

    // 绘制 2.5D 等距地面菱形瓦片 (Quad = 2 Triangles)
    static void draw_iso_tile(
        SDL_Renderer* renderer, 
        float gx, float gy, float gz,
        float origin_x, float origin_y,
        SDL_FColor color
    ) {
        float h_w = TILE_W * 0.5f;
        float h_h = TILE_H * 0.5f;

        SDL_FPoint center = world_to_screen(gx, gy, gz, origin_x, origin_y);

        // 菱形四顶点：北、东、南、西
        SDL_FPoint n{center.x, center.y - h_h};
        SDL_FPoint e{center.x + h_w, center.y};
        SDL_FPoint s{center.x, center.y + h_h};
        SDL_FPoint w{center.x - h_w, center.y};

        SDL_Vertex verts[4] = {
            { n, color, {0.0f, 0.0f} },
            { e, color, {1.0f, 0.0f} },
            { s, color, {1.0f, 1.0f} },
            { w, color, {0.0f, 1.0f} }
        };

        int indices[6] = {0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(renderer, nullptr, verts, 4, indices, 6);
    }

    // 绘制 3D 挤出建筑方体 (立体实心砖石外墙 + 屋顶顶面)
    static void draw_iso_building_block(
        SDL_Renderer* renderer,
        float gx, float gy,
        float origin_x, float origin_y,
        bool is_terrace,
        bool is_collapsed
    ) {
        float h_w = TILE_W * 0.5f;
        float h_h = TILE_H * 0.5f;

        // 底部与顶部中心
        SDL_FPoint b_center = world_to_screen(gx, gy, 0.0f, origin_x, origin_y);
        SDL_FPoint t_center = world_to_screen(gx, gy, 1.0f, origin_x, origin_y);

        // 底菱形顶点 (Z = 0)
        SDL_FPoint b_e{b_center.x + h_w, b_center.y};
        SDL_FPoint b_s{b_center.x, b_center.y + h_h};
        SDL_FPoint b_w{b_center.x - h_w, b_center.y};

        // 顶菱形顶点 (Z = 1)
        SDL_FPoint t_n{t_center.x, t_center.y - h_h};
        SDL_FPoint t_e{t_center.x + h_w, t_center.y};
        SDL_FPoint t_s{t_center.x, t_center.y + h_h};
        SDL_FPoint t_w{t_center.x - h_w, t_center.y};

        // 配色方案：建筑朝向阴影（左侧暗面，右侧受光面，顶面受月光照耀）
        SDL_FColor wall_left{0.18f, 0.15f, 0.12f, 1.0f};  // 阴影面
        SDL_FColor wall_right{0.28f, 0.23f, 0.18f, 1.0f}; // 漫反射受光面

        SDL_FColor roof_top = is_collapsed 
            ? SDL_FColor{0.32f, 0.18f, 0.12f, 1.0f} // 瓦砾碎石
            : (is_terrace 
                ? SDL_FColor{0.45f, 0.38f, 0.30f, 1.0f}  // 青铜平顶露台
                : SDL_FColor{0.25f, 0.35f, 0.48f, 1.0f}); // 达鲁基斯坦青石斜瓦顶

        // 1. 绘制左侧立面 (South-West Face: t_w, t_s, b_s, b_w)
        SDL_Vertex left_verts[4] = {
            { t_w, wall_left, {0,0} }, { t_s, wall_left, {1,0} },
            { b_s, wall_left, {1,1} }, { b_w, wall_left, {0,1} }
        };
        int quad_idx[6] = {0, 1, 2, 0, 2, 3};
        SDL_RenderGeometry(renderer, nullptr, left_verts, 4, quad_idx, 6);

        // 2. 绘制右侧立面 (South-East Face: t_s, t_e, b_e, b_s)
        SDL_Vertex right_verts[4] = {
            { t_s, wall_right, {0,0} }, { t_e, wall_right, {1,0} },
            { b_e, wall_right, {1,1} }, { b_s, wall_right, {0,1} }
        };
        SDL_RenderGeometry(renderer, nullptr, right_verts, 4, quad_idx, 6);

        // 3. 绘制顶部顶面 (Top Face: t_n, t_e, t_s, t_w)
        SDL_Vertex top_verts[4] = {
            { t_n, roof_top, {0,0} }, { t_e, roof_top, {1,0} },
            { t_s, roof_top, {1,1} }, { t_w, roof_top, {0,1} }
        };
        SDL_RenderGeometry(renderer, nullptr, top_verts, 4, quad_idx, 6);

        // 顶部屋檐勾勒高光线
        SDL_SetRenderDrawColor(renderer, 90, 120, 160, 200);
        SDL_RenderLine(renderer, t_n.x, t_n.y, t_e.x, t_e.y);
        SDL_RenderLine(renderer, t_e.x, t_e.y, t_s.x, t_s.y);
        SDL_RenderLine(renderer, t_s.x, t_s.y, t_w.x, t_w.y);
        SDL_RenderLine(renderer, t_w.x, t_w.y, t_n.x, t_n.y);
    }

    // 绘制 3D 实体战术角色立柱 Token (带地面投影与 3D 高度立体感)
    static void draw_iso_character_token(
        SDL_Renderer* renderer,
        float gx, float gy, float gz,
        float origin_x, float origin_y,
        SDL_FColor base_color,
        char glyph,
        float scale = 1.0f
    ) {
        SDL_FPoint ground = world_to_screen(gx, gy, gz, origin_x, origin_y);

        // 1. 地面椭圆柔和投影 (Drop Shadow)
        float shadow_w = 18.0f * scale;
        float shadow_h = 9.0f * scale;
        SDL_SetRenderDrawColor(renderer, 5, 8, 12, 140);
        SDL_FRect shadow_rect{ground.x - shadow_w, ground.y - shadow_h * 0.5f, shadow_w * 2.0f, shadow_h};
        SDL_RenderFillRect(renderer, &shadow_rect);

        // 2. 3D 战术立柱 (从 ground.y 向上延伸 height 像素)
        float token_h = 24.0f * scale;
        float token_w = 12.0f * scale;
        float ty = ground.y - token_h;

        // 侧柱背板
        SDL_FRect body_rect{ground.x - token_w, ty, token_w * 2.0f, token_h};
        SDL_SetRenderDrawColor(
            renderer, 
            static_cast<uint8_t>(base_color.r * 200),
            static_cast<uint8_t>(base_color.g * 200),
            static_cast<uint8_t>(base_color.b * 200),
            255
        );
        SDL_RenderFillRect(renderer, &body_rect);

        // 顶面立绘圆环/方块
        SDL_FRect cap_rect{ground.x - token_w, ty - 8.0f, token_w * 2.0f, 12.0f};
        SDL_SetRenderDrawColor(
            renderer, 
            static_cast<uint8_t>(base_color.r * 255),
            static_cast<uint8_t>(base_color.g * 255),
            static_cast<uint8_t>(base_color.b * 255),
            255
        );
        SDL_RenderFillRect(renderer, &cap_rect);

        // 金色高光边框
        SDL_SetRenderDrawColor(renderer, 240, 220, 160, 255);
        SDL_RenderRect(renderer, &body_rect);
        SDL_RenderRect(renderer, &cap_rect);

        // 字符雕刻
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        float cx = ground.x - 4.0f;
        float cy = ty + (token_h * 0.5f) - 6.0f;
        
        // 简易点阵写字
        if (glyph == 'K') {
            SDL_RenderLine(renderer, cx, cy, cx, cy + 10.0f);
            SDL_RenderLine(renderer, cx, cy + 5.0f, cx + 7.0f, cy);
            SDL_RenderLine(renderer, cx, cy + 5.0f, cx + 7.0f, cy + 10.0f);
        } else if (glyph == 'Q') {
            SDL_FRect q_rect{cx, cy, 7.0f, 8.0f};
            SDL_RenderRect(renderer, &q_rect);
            SDL_RenderLine(renderer, cx + 4.0f, cy + 6.0f, cx + 8.0f, cy + 10.0f);
        } else if (glyph == 'C') {
            SDL_RenderLine(renderer, cx + 7.0f, cy, cx, cy);
            SDL_RenderLine(renderer, cx, cy, cx, cy + 10.0f);
            SDL_RenderLine(renderer, cx, cy + 10.0f, cx + 7.0f, cy + 10.0f);
        } else if (glyph == 'R') { // 暴君雷斯特
            SDL_RenderLine(renderer, cx, cy, cx, cy + 10.0f);
            SDL_RenderLine(renderer, cx, cy, cx + 7.0f, cy);
            SDL_RenderLine(renderer, cx + 7.0f, cy, cx + 7.0f, cy + 5.0f);
            SDL_RenderLine(renderer, cx, cy + 5.0f, cx + 7.0f, cy + 5.0f);
            SDL_RenderLine(renderer, cx + 3.0f, cy + 5.0f, cx + 7.0f, cy + 10.0f);
        } else if (glyph == 'P') { // 克鲁普 (Pastry & Philosopher)
            SDL_RenderLine(renderer, cx, cy, cx, cy + 10.0f);
            SDL_RenderLine(renderer, cx, cy, cx + 7.0f, cy);
            SDL_RenderLine(renderer, cx + 7.0f, cy, cx + 7.0f, cy + 5.0f);
            SDL_RenderLine(renderer, cx, cy + 5.0f, cx + 7.0f, cy + 5.0f);
        } else if (glyph == 'O') { // 克罗库斯 (Oponn's coin)
            SDL_FRect o_rect{cx, cy, 7.0f, 10.0f};
            SDL_RenderRect(renderer, &o_rect);
        } else if (glyph == 'A') { // 阿诺曼德·雷克 (Anomander Rake)
            SDL_RenderLine(renderer, cx + 3.5f, cy, cx, cy + 10.0f);
            SDL_RenderLine(renderer, cx + 3.5f, cy, cx + 7.0f, cy + 10.0f);
            SDL_RenderLine(renderer, cx + 1.5f, cy + 6.0f, cx + 5.5f, cy + 6.0f);
        } else if (glyph == 'G') { // 城防军卫兵 (City Watch Guard)
            SDL_RenderLine(renderer, cx + 7.0f, cy, cx, cy);
            SDL_RenderLine(renderer, cx, cy, cx, cy + 10.0f);
            SDL_RenderLine(renderer, cx, cy + 10.0f, cx + 7.0f, cy + 10.0f);
            SDL_RenderLine(renderer, cx + 7.0f, cy + 5.0f, cx + 7.0f, cy + 10.0f);
            SDL_RenderLine(renderer, cx + 3.0f, cy + 5.0f, cx + 7.0f, cy + 5.0f);
        } else if (glyph == 'T') { // 泰兰·伊玛斯 (T'lan Imass Bone Warrior)
            SDL_RenderLine(renderer, cx, cy, cx + 7.0f, cy);
            SDL_RenderLine(renderer, cx + 3.5f, cy, cx + 3.5f, cy + 10.0f);
        } else if (glyph == 'D') { // 混沌裂隙恶魔 (Chaos Rift Demon)
            SDL_RenderLine(renderer, cx, cy, cx, cy + 10.0f);
            SDL_RenderLine(renderer, cx, cy, cx + 5.0f, cy + 3.0f);
            SDL_RenderLine(renderer, cx + 5.0f, cy + 3.0f, cx + 5.0f, cy + 7.0f);
            SDL_RenderLine(renderer, cx + 5.0f, cy + 7.0f, cx, cy + 10.0f);
        }
    }

    // 绘制工兵布设的莫兰斯定向地雷 (Sapper Directional Mine)
    static void draw_iso_sapper_mine(
        SDL_Renderer* renderer,
        float gx, float gy, float gz,
        float origin_x, float origin_y,
        float pulse
    ) {
        SDL_FPoint ground = world_to_screen(gx, gy, gz, origin_x, origin_y);
        SDL_FRect mine_base{ground.x - 8.0f, ground.y - 4.0f, 16.0f, 8.0f};
        SDL_SetRenderDrawColor(renderer, 26, 30, 36, 255);
        SDL_RenderFillRect(renderer, &mine_base);
        SDL_SetRenderDrawColor(renderer, 80, 85, 90, 255);
        SDL_RenderRect(renderer, &mine_base);

        // 闪烁的炼金激活指示灯 (Primer LED)
        float blink = 0.5f + 0.5f * std::sin(pulse * 9.0f);
        SDL_FRect primer_led{ground.x - 2.0f, ground.y - 2.0f, 4.0f, 4.0f};
        SDL_SetRenderDrawColor(renderer, 255, 40, 40, static_cast<uint8_t>(255 * blink));
        SDL_RenderFillRect(renderer, &primer_led);
    }

    // 绘制异界空间撕裂维度旋涡 (Dynamic Warren Rift)
    static void draw_iso_warren_rift(
        SDL_Renderer* renderer,
        float gx, float gy, float gz,
        float origin_x, float origin_y,
        uint8_t rift_type,
        float pulse
    ) {
        SDL_FPoint ground = world_to_screen(gx, gy, gz, origin_x, origin_y);
        float r_screen = (TILE_W * 0.9f) + 4.0f * std::sin(pulse * 4.5f);
        
        SDL_FColor center_col{0.75f, 0.15f, 0.95f, 0.65f}; // 混沌暗紫
        SDL_FColor edge_col{0.35f, 0.05f, 0.55f, 0.0f};
        if (rift_type == 1) { // Kurald Galain 纯暗
            center_col = {0.05f, 0.05f, 0.18f, 0.85f};
            edge_col = {0.15f, 0.15f, 0.35f, 0.0f};
        } else if (rift_type == 2) { // Tellann 荒火
            center_col = {0.95f, 0.65f, 0.12f, 0.75f};
            edge_col = {0.85f, 0.25f, 0.05f, 0.0f};
        }

        constexpr int SEGMENTS = 14;
        std::vector<SDL_Vertex> verts;
        verts.push_back({ground, center_col, {0,0}});
        for (int i = 0; i <= SEGMENTS; ++i) {
            float angle = (i * 2.0f * 3.14159f) / SEGMENTS + (pulse * 2.5f); // 动态旋流
            float px = ground.x + std::cos(angle) * r_screen;
            float py = ground.y + std::sin(angle) * (r_screen * 0.5f);
            verts.push_back({{px, py}, edge_col, {0,0}});
        }
        std::vector<int> idx;
        for (int i = 1; i <= SEGMENTS; ++i) {
            idx.push_back(0);
            idx.push_back(i);
            idx.push_back(i + 1);
        }
        SDL_RenderGeometry(renderer, nullptr, verts.data(), static_cast<int>(verts.size()), 
                           idx.data(), static_cast<int>(idx.size()));

        // 空间撕裂黑色垂直裂隙核心
        SDL_FRect core{ground.x - 3.0f, ground.y - 12.0f, 6.0f, 24.0f};
        SDL_SetRenderDrawColor(renderer, 10, 5, 18, 255);
        SDL_RenderFillRect(renderer, &core);
        SDL_SetRenderDrawColor(renderer, 220, 160, 255, 200);
        SDL_RenderRect(renderer, &core);
    }

    // 绘制下水道与地下墓穴铸铁铁格栅 (Sewer Grate Stairs)
    static void draw_iso_sewer_grate(
        SDL_Renderer* renderer,
        float gx, float gy, float gz,
        float origin_x, float origin_y
    ) {
        SDL_FPoint p = world_to_screen(gx, gy, gz, origin_x, origin_y);
        SDL_FRect grate_base{p.x - 14.0f, p.y - 7.0f, 28.0f, 14.0f};
        SDL_SetRenderDrawColor(renderer, 10, 12, 16, 240);
        SDL_RenderFillRect(renderer, &grate_base);
        SDL_SetRenderDrawColor(renderer, 100, 95, 85, 255);
        SDL_RenderRect(renderer, &grate_base);
        for (float ox = -10.0f; ox <= 10.0f; ox += 4.0f) {
            SDL_RenderLine(renderer, p.x + ox, p.y - 7.0f, p.x + ox, p.y + 7.0f);
        }
        SDL_RenderLine(renderer, p.x - 14.0f, p.y, p.x + 14.0f, p.y);
    }

    // 绘制地下墓穴高压易爆瓦斯气云 (Combustible Gas Cloud)
    static void draw_iso_gas_cloud(
        SDL_Renderer* renderer,
        float gx, float gy, float gz,
        float origin_x, float origin_y,
        float concentration,
        float pulse
    ) {
        SDL_FPoint ground = world_to_screen(gx, gy, gz, origin_x, origin_y);
        float r_screen = (TILE_W * 0.75f) + 3.0f * std::sin(pulse * 2.5f + (gx + gy));
        
        SDL_FColor center_col{0.10f, 0.85f, 0.70f, std::min(0.55f, concentration / 140.0f)};
        SDL_FColor edge_col{0.05f, 0.35f, 0.30f, 0.0f};

        constexpr int SEGMENTS = 12;
        std::vector<SDL_Vertex> verts;
        verts.push_back({ground, center_col, {0,0}});
        for (int i = 0; i <= SEGMENTS; ++i) {
            float angle = (i * 2.0f * 3.14159f) / SEGMENTS;
            float px = ground.x + std::cos(angle) * r_screen;
            float py = ground.y + std::sin(angle) * (r_screen * 0.5f);
            verts.push_back({{px, py}, edge_col, {0,0}});
        }
        std::vector<int> idx;
        for (int i = 1; i <= SEGMENTS; ++i) {
            idx.push_back(0);
            idx.push_back(i);
            idx.push_back(i + 1);
        }
        SDL_RenderGeometry(renderer, nullptr, verts.data(), static_cast<int>(verts.size()), 
                           idx.data(), static_cast<int>(idx.size()));
    }

    // 绘制地下墓穴玄武岩古老坚壁 (Catacomb Subterranean Wall)
    static void draw_iso_catacomb_wall(
        SDL_Renderer* renderer,
        float gx, float gy,
        float origin_x, float origin_y
    ) {
        float gz = -1.0f;
        SDL_FPoint b_n = world_to_screen(gx, gy, gz, origin_x, origin_y);
        SDL_FPoint b_e = world_to_screen(gx + 1.0f, gy, gz, origin_x, origin_y);
        SDL_FPoint b_s = world_to_screen(gx + 1.0f, gy + 1.0f, gz, origin_x, origin_y);
        SDL_FPoint b_w = world_to_screen(gx, gy + 1.0f, gz, origin_x, origin_y);

        float wall_h = 24.0f;
        SDL_FPoint t_n = {b_n.x, b_n.y - wall_h};
        SDL_FPoint t_e = {b_e.x, b_e.y - wall_h};
        SDL_FPoint t_s = {b_s.x, b_s.y - wall_h};
        SDL_FPoint t_w = {b_w.x, b_w.y - wall_h};

        SDL_FColor side_color{0.07f, 0.08f, 0.10f, 1.0f};
        SDL_FColor top_color{0.13f, 0.15f, 0.18f, 1.0f};

        int quad_idx[6] = {0, 1, 2, 0, 2, 3};
        SDL_Vertex left_verts[4] = {
            { t_w, side_color, {0,0} }, { t_s, side_color, {1,0} },
            { b_s, side_color, {1,1} }, { b_w, side_color, {0,1} }
        };
        SDL_RenderGeometry(renderer, nullptr, left_verts, 4, quad_idx, 6);

        SDL_Vertex right_verts[4] = {
            { t_s, side_color, {0,0} }, { t_e, side_color, {1,0} },
            { b_e, side_color, {1,1} }, { b_s, side_color, {0,1} }
        };
        SDL_RenderGeometry(renderer, nullptr, right_verts, 4, quad_idx, 6);

        SDL_Vertex top_verts[4] = {
            { t_n, top_color, {0,0} }, { t_e, top_color, {1,0} },
            { t_s, top_color, {1,1} }, { t_w, top_color, {0,1} }
        };
        SDL_RenderGeometry(renderer, nullptr, top_verts, 4, quad_idx, 6);

        SDL_SetRenderDrawColor(renderer, 50, 60, 75, 180);
        SDL_RenderLine(renderer, t_w.x, t_w.y, t_s.x, t_s.y);
        SDL_RenderLine(renderer, t_s.x, t_s.y, t_e.x, t_e.y);
    }

    // 绘制 2.5D 蓝火路灯与环形漫射辐射光晕
    static void draw_iso_blue_gas_lamp(
        SDL_Renderer* renderer,
        float gx, float gy,
        float origin_x, float origin_y,
        float pulse
    ) {
        SDL_FPoint ground = world_to_screen(gx, gy, 0.0f, origin_x, origin_y);

        // 1. 地面大范围动态呼吸蓝光漫射 (Radial Light Aura)
        float glow_radius = (TILE_W * 1.6f) + 4.0f * std::sin(pulse * 4.0f);
        SDL_FColor glow_center{0.0f, 0.9f, 1.0f, 0.35f};
        SDL_FColor glow_edge{0.0f, 0.2f, 0.6f, 0.0f};

        // 构造八角扇面三角形逼近光晕
        constexpr int SEGMENTS = 10;
        std::vector<SDL_Vertex> light_verts;
        light_verts.push_back({ground, glow_center, {0,0}});
        for (int i = 0; i <= SEGMENTS; ++i) {
            float angle = (i * 2.0f * 3.14159f) / SEGMENTS;
            float px = ground.x + std::cos(angle) * glow_radius;
            float py = ground.y + std::sin(angle) * (glow_radius * 0.5f); // 2:1 椭圆压扁
            light_verts.push_back({{px, py}, glow_edge, {0,0}});
        }
        std::vector<int> light_idx;
        for (int i = 1; i <= SEGMENTS; ++i) {
            light_idx.push_back(0);
            light_idx.push_back(i);
            light_idx.push_back(i + 1);
        }
        SDL_RenderGeometry(renderer, nullptr, light_verts.data(), static_cast<int>(light_verts.size()), 
                           light_idx.data(), static_cast<int>(light_idx.size()));

        // 2. 金属黑铁灯柱 (竖立高 32px)
        float lamp_top_y = ground.y - 32.0f;
        SDL_SetRenderDrawColor(renderer, 40, 45, 55, 255);
        SDL_RenderLine(renderer, ground.x, ground.y, ground.x, lamp_top_y);
        SDL_RenderLine(renderer, ground.x - 1.0f, ground.y, ground.x - 1.0f, lamp_top_y);

        // 3. 蓝火灯芯核心 (Glowing Flame Core)
        float flame_glow = 0.8f + 0.2f * std::sin(pulse * 6.0f);
        SDL_FRect flame_core{ground.x - 4.0f, lamp_top_y - 6.0f, 8.0f, 10.0f};
        SDL_SetRenderDrawColor(renderer, 0, 229, 255, static_cast<uint8_t>(255 * flame_glow));
        SDL_RenderFillRect(renderer, &flame_core);

        SDL_FRect flame_hotspot{ground.x - 2.0f, lamp_top_y - 4.0f, 4.0f, 6.0f};
        SDL_SetRenderDrawColor(renderer, 240, 255, 255, 255);
        SDL_RenderFillRect(renderer, &flame_hotspot);
    }

    // 绘制奥塔拉尔（Otataral）反魔力场绝死光环 (生锈赭红微光脉冲)
    static void draw_iso_otataral_field(
        SDL_Renderer* renderer,
        float gx, float gy, float gz,
        float origin_x, float origin_y,
        float radius,
        float pulse
    ) {
        SDL_FPoint ground = world_to_screen(gx, gy, gz, origin_x, origin_y);

        float r_screen = (radius * TILE_W * 0.65f) + 2.0f * std::sin(pulse * 3.5f);
        SDL_FColor center_col{0.80f, 0.30f, 0.10f, 0.35f}; // 赭红死域核心
        SDL_FColor edge_col{0.60f, 0.20f, 0.05f, 0.0f};    // 边缘消散

        constexpr int SEGMENTS = 16;
        std::vector<SDL_Vertex> verts;
        verts.push_back({ground, center_col, {0,0}});
        for (int i = 0; i <= SEGMENTS; ++i) {
            float angle = (i * 2.0f * 3.14159f) / SEGMENTS;
            float px = ground.x + std::cos(angle) * r_screen;
            float py = ground.y + std::sin(angle) * (r_screen * 0.5f); // 2:1 椭圆
            verts.push_back({{px, py}, edge_col, {0,0}});
        }
        std::vector<int> idx;
        for (int i = 1; i <= SEGMENTS; ++i) {
            idx.push_back(0);
            idx.push_back(i);
            idx.push_back(i + 1);
        }
        SDL_RenderGeometry(renderer, nullptr, verts.data(), static_cast<int>(verts.size()), 
                           idx.data(), static_cast<int>(idx.size()));

        // 赭红铁锈外圈光芒
        SDL_SetRenderDrawColor(renderer, 210, 80, 30, 160);
        for (int i = 0; i < SEGMENTS; ++i) {
            float a1 = (i * 2.0f * 3.14159f) / SEGMENTS;
            float a2 = ((i + 1) * 2.0f * 3.14159f) / SEGMENTS;
            SDL_RenderLine(renderer, 
                           ground.x + std::cos(a1) * r_screen, ground.y + std::sin(a1) * (r_screen * 0.5f),
                           ground.x + std::cos(a2) * r_screen, ground.y + std::sin(a2) * (r_screen * 0.5f));
        }
    }
};

} // namespace Malazan
