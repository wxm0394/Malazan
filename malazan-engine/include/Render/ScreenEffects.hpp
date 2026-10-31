#pragma once
#include <algorithm>
#include <cmath>
#include <utility>
#include <cstdint>

namespace Malazan {

class ScreenEffects {
public:
    static inline float trauma{0.0f};          // 震屏创伤值 (0.0 ~ 1.0)
    static inline float blood_flash{0.0f};      // 受到重创时的红屏暗角 (0.0 ~ 1.0)
    static inline float distortion_wave{0.0f};  // 沃伦裂隙引力畸变波纹 (0.0 ~ 1.0)

    static void add_trauma(float amount) {
        trauma = std::clamp(trauma + amount, 0.0f, 1.0f);
    }

    static void trigger_blood_flash(float intensity = 0.8f) {
        blood_flash = std::clamp(intensity, 0.0f, 1.0f);
    }

    static void trigger_distortion_wave(float intensity = 0.9f) {
        distortion_wave = std::clamp(intensity, 0.0f, 1.0f);
    }

    // 获取每帧震屏偏移 (基于创伤的非线性平方衰减, ~60 FPS)
    static std::pair<float, float> get_shake_offset(float pulse) {
        if (trauma <= 0.005f) {
            trauma = 0.0f;
            return {0.0f, 0.0f};
        }

        float shake = trauma * trauma; // 平方衰减，保证大爆炸时强烈，小创伤时细腻
        float max_displacement = 22.0f;
        float dx = shake * max_displacement * std::sin(pulse * 38.0f);
        float dy = shake * max_displacement * std::cos(pulse * 46.0f);

        // 创伤自然衰减阻尼
        trauma = std::max(0.0f, trauma - 0.035f);

        return {dx, dy};
    }

    // 更新屏幕特效渐隐
    static void update_frame() {
        if (blood_flash > 0.005f) {
            blood_flash = std::max(0.0f, blood_flash - 0.04f);
        } else {
            blood_flash = 0.0f;
        }

        if (distortion_wave > 0.005f) {
            distortion_wave = std::max(0.0f, distortion_wave - 0.03f);
        } else {
            distortion_wave = 0.0f;
        }
    }
};

} // namespace Malazan
