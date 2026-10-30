#pragma once
#include <SDL3/SDL.h>
#include <string_view>
#include <cstdint>

namespace Malazan {

// 内置极简 5x7 / 8x8 点阵字体渲染器（完全零外部字体文件依赖，开箱即用）
class FontRenderer {
public:
    // 渲染单字符
    static void draw_char(SDL_Renderer* renderer, char c, float x, float y, float scale, SDL_Color color) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        
        // 简易 ASCII 字符点阵掩码表（32 ~ 126）
        uint8_t glyph[8] = {0};
        get_glyph_data(c, glyph);

        for (int r = 0; r < 8; ++r) {
            uint8_t row = glyph[r];
            for (int col = 0; col < 8; ++col) {
                if ((row >> (7 - col)) & 1) {
                    SDL_FRect pixel_rect{x + col * scale, y + r * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &pixel_rect);
                }
            }
        }
    }

    // 渲染字符串
    static void draw_text(SDL_Renderer* renderer, std::string_view text, float x, float y, float scale, SDL_Color color) {
        float cur_x = x;
        float cur_y = y;
        for (char c : text) {
            if (c == '\n') {
                cur_x = x;
                cur_y += 10.0f * scale;
                continue;
            }
            draw_char(renderer, c, cur_x, cur_y, scale, color);
            cur_x += 8.0f * scale;
        }
    }

private:
    static void get_glyph_data(char c, uint8_t glyph[8]) {
        // 常用符号与字母的点阵硬编码定义
        switch (c) {
            case 'A': case 'a': glyph[0]=0x18; glyph[1]=0x24; glyph[2]=0x42; glyph[3]=0x7E; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x42; break;
            case 'B': case 'b': glyph[0]=0x7C; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x7C; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x7C; break;
            case 'C': case 'c': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x40; glyph[3]=0x40; glyph[4]=0x40; glyph[5]=0x42; glyph[6]=0x3C; break;
            case 'D': case 'd': glyph[0]=0x78; glyph[1]=0x44; glyph[2]=0x42; glyph[3]=0x42; glyph[4]=0x42; glyph[5]=0x44; glyph[6]=0x78; break;
            case 'E': case 'e': glyph[0]=0x7E; glyph[1]=0x40; glyph[2]=0x40; glyph[3]=0x78; glyph[4]=0x40; glyph[5]=0x40; glyph[6]=0x7E; break;
            case 'F': case 'f': glyph[0]=0x7E; glyph[1]=0x40; glyph[2]=0x40; glyph[3]=0x78; glyph[4]=0x40; glyph[5]=0x40; glyph[6]=0x40; break;
            case 'G': case 'g': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x40; glyph[3]=0x4E; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x3C; break;
            case 'H': case 'h': glyph[0]=0x42; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x7E; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x42; break;
            case 'I': case 'i': glyph[0]=0x3C; glyph[1]=0x18; glyph[2]=0x18; glyph[3]=0x18; glyph[4]=0x18; glyph[5]=0x18; glyph[6]=0x3C; break;
            case 'J': case 'j': glyph[0]=0x1E; glyph[1]=0x06; glyph[2]=0x06; glyph[3]=0x06; glyph[4]=0x06; glyph[5]=0x46; glyph[6]=0x3C; break;
            case 'K': case 'k': glyph[0]=0x42; glyph[1]=0x44; glyph[2]=0x48; glyph[3]=0x70; glyph[4]=0x48; glyph[5]=0x44; glyph[6]=0x42; break;
            case 'L': case 'l': glyph[0]=0x40; glyph[1]=0x40; glyph[2]=0x40; glyph[3]=0x40; glyph[4]=0x40; glyph[5]=0x40; glyph[6]=0x7E; break;
            case 'M': case 'm': glyph[0]=0x42; glyph[1]=0x66; glyph[2]=0x5A; glyph[3]=0x42; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x42; break;
            case 'N': case 'n': glyph[0]=0x42; glyph[1]=0x62; glyph[2]=0x52; glyph[3]=0x4A; glyph[4]=0x46; glyph[5]=0x42; glyph[6]=0x42; break;
            case 'O': case 'o': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x42; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x3C; break;
            case 'P': case 'p': glyph[0]=0x7C; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x7C; glyph[4]=0x40; glyph[5]=0x40; glyph[6]=0x40; break;
            case 'Q': case 'q': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x42; glyph[4]=0x4A; glyph[5]=0x44; glyph[6]=0x3A; break;
            case 'R': case 'r': glyph[0]=0x7C; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x7C; glyph[4]=0x48; glyph[5]=0x44; glyph[6]=0x42; break;
            case 'S': case 's': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x40; glyph[3]=0x3C; glyph[4]=0x02; glyph[5]=0x42; glyph[6]=0x3C; break;
            case 'T': case 't': glyph[0]=0x7E; glyph[1]=0x18; glyph[2]=0x18; glyph[3]=0x18; glyph[4]=0x18; glyph[5]=0x18; glyph[6]=0x18; break;
            case 'U': case 'u': glyph[0]=0x42; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x42; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x3C; break;
            case 'V': case 'v': glyph[0]=0x42; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x42; glyph[4]=0x24; glyph[5]=0x24; glyph[6]=0x18; break;
            case 'W': case 'w': glyph[0]=0x42; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x5A; glyph[4]=0x5A; glyph[5]=0x66; glyph[6]=0x42; break;
            case 'X': case 'x': glyph[0]=0x42; glyph[1]=0x24; glyph[2]=0x18; glyph[3]=0x18; glyph[4]=0x24; glyph[5]=0x42; glyph[6]=0x42; break;
            case 'Y': case 'y': glyph[0]=0x42; glyph[1]=0x42; glyph[2]=0x24; glyph[3]=0x18; glyph[4]=0x18; glyph[5]=0x18; glyph[6]=0x18; break;
            case 'Z': case 'z': glyph[0]=0x7E; glyph[1]=0x04; glyph[2]=0x08; glyph[3]=0x10; glyph[4]=0x20; glyph[5]=0x40; glyph[6]=0x7E; break;
            case '0': glyph[0]=0x3C; glyph[1]=0x46; glyph[2]=0x4A; glyph[3]=0x52; glyph[4]=0x62; glyph[5]=0x42; glyph[6]=0x3C; break;
            case '1': glyph[0]=0x18; glyph[1]=0x28; glyph[2]=0x08; glyph[3]=0x08; glyph[4]=0x08; glyph[5]=0x08; glyph[6]=0x3E; break;
            case '2': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x04; glyph[3]=0x18; glyph[4]=0x20; glyph[5]=0x40; glyph[6]=0x7E; break;
            case '3': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x04; glyph[3]=0x18; glyph[4]=0x02; glyph[5]=0x42; glyph[6]=0x3C; break;
            case '4': glyph[0]=0x08; glyph[1]=0x18; glyph[2]=0x28; glyph[3]=0x48; glyph[4]=0x7E; glyph[5]=0x08; glyph[6]=0x08; break;
            case '5': glyph[0]=0x7E; glyph[1]=0x40; glyph[2]=0x7C; glyph[3]=0x02; glyph[4]=0x02; glyph[5]=0x42; glyph[6]=0x3C; break;
            case '6': glyph[0]=0x3C; glyph[1]=0x40; glyph[2]=0x7C; glyph[3]=0x42; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x3C; break;
            case '7': glyph[0]=0x7E; glyph[1]=0x02; glyph[2]=0x04; glyph[3]=0x08; glyph[4]=0x10; glyph[5]=0x10; glyph[6]=0x10; break;
            case '8': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x3C; glyph[4]=0x42; glyph[5]=0x42; glyph[6]=0x3C; break;
            case '9': glyph[0]=0x3C; glyph[1]=0x42; glyph[2]=0x42; glyph[3]=0x3E; glyph[4]=0x02; glyph[5]=0x02; glyph[6]=0x3C; break;
            case ':': glyph[2]=0x18; glyph[4]=0x18; break;
            case '-': glyph[3]=0x7E; break;
            case '+': glyph[1]=0x18; glyph[2]=0x18; glyph[3]=0x7E; glyph[4]=0x18; glyph[5]=0x18; break;
            case '[': glyph[0]=0x3C; glyph[1]=0x20; glyph[2]=0x20; glyph[3]=0x20; glyph[4]=0x20; glyph[5]=0x20; glyph[6]=0x3C; break;
            case ']': glyph[0]=0x3C; glyph[1]=0x04; glyph[2]=0x04; glyph[3]=0x04; glyph[4]=0x04; glyph[5]=0x04; glyph[6]=0x3C; break;
            case '/': glyph[1]=0x02; glyph[2]=0x04; glyph[3]=0x08; glyph[4]=0x10; glyph[5]=0x20; glyph[6]=0x40; break;
            case '%': glyph[0]=0x62; glyph[1]=0x64; glyph[2]=0x08; glyph[3]=0x10; glyph[4]=0x20; glyph[5]=0x4C; glyph[6]=0x46; break;
            case '>': glyph[1]=0x40; glyph[2]=0x20; glyph[3]=0x10; glyph[4]=0x08; glyph[5]=0x10; glyph[6]=0x20; glyph[7]=0x40; break;
            case ' ': default: break;
        }
    }
};

} // namespace Malazan
