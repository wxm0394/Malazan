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
        }
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
};

} // namespace Malazan
