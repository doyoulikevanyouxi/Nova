#ifndef VALENCE_SYSTEM_H
#define VALENCE_SYSTEM_H

#include <algorithm>
#include <cmath>

// ============================================================
// ValenceSystem - 效价系统
// ============================================================
//
// 效价是"好/坏"的标签：
//   +1：非常好（血糖高）
//    0：中性
//   -1：非常坏（血糖低）
//
// 效价影响：
//   1. 多巴胺调制：效价高 → DA 高
//   2. 记忆写入强度：正效价加速写入
//   3. 遗忘速率：负效价加速遗忘
//
// ============================================================

struct ValenceSystem {
    double valence = 0.0;       // 当前效价（-1 到 +1）
    double integrator = 0.0;    // 积分器
    double tau = 200.0;         // 时间常数（ms）

    // ===== 更新 =====
    // 根据血糖计算效价
    // glucose > 0.5 → 正效价
    // glucose < 0.5 → 负效价
    void update(double glucose, double dt) {
        double targetValence = (glucose - 0.5) * 2.0;

        integrator += (targetValence - integrator) / tau * dt;
        valence = integrator;

        if (valence > 1.0) valence = 1.0;
        if (valence < -1.0) valence = -1.0;
    }

    // ===== 多巴胺调制 =====
    // 效价 +1 → DA 基线 0.9
    // 效价  0 → DA 基线 0.5
    // 效价 -1 → DA 基线 0.1
    double getDopamineModulation() const {
        return 0.5 + valence * 0.4;
    }

    // ===== 记忆写入强度 =====
    // 正效价加速写入
    double getMemoryWriteStrength() const {
        return std::max(0.0, valence);
    }

    // ===== 遗忘速率 =====
    // 负效价加速遗忘
    double getForgettingRate() const {
        return std::max(0.0, -valence);
    }

    // ===== 状态描述 =====
    const char* state() const {
        if (valence > 0.3) return "Positive";
        if (valence < -0.3) return "Negative";
        return "Neutral";
    }
};

#endif
