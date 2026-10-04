#ifndef CURIOSITY_INTERFACE_H
#define CURIOSITY_INTERFACE_H

#include <algorithm>

// ============================================================
// CuriosityInterface - 好奇心接口
// ============================================================
//
// 好奇心 = 新异度 × 安全评估
//
// 输入：
//   noveltySignal：新异度 (0-1)，1 = 完全新异
//   safetySignal：安全评估 (0-1)，1 = 安全，0 = 危险
//
// 输出：
//   getDopamineBoost()：好奇心驱动 DA 上升
//   getSerotoninBoost()：危险驱动 5-HT 上升
//   getPerceptualGain()：好奇心提升感知增益
//
// 当前阶段：noveltySignal 和 safetySignal 由外部手动设置。
// 后期：由 PFC 等多区域联合计算自动产生。
//
// ============================================================

struct CuriosityInterface {
    // ===== 输入信号 =====
    double noveltySignal = 0.0;    // 新异度 (0-1)
    double safetySignal = 1.0;     // 安全评估 (0-1)

    // ===== 好奇心驱动 DA =====
    // 新异度高 + 安全 → 好奇心高 → DA 上升
    double getDopamineBoost() const {
        double curiosity = noveltySignal * safetySignal;
        return curiosity * 0.5;
    }

    // ===== 危险驱动 5-HT =====
    // 不安全 → 5-HT 上升 → 探索抑制
    double getSerotoninBoost() const {
        double danger = (1.0 - safetySignal);
        return danger * 0.5;
    }

    // ===== 好奇心驱动感知增益 =====
    double getPerceptualGain() const {
        double curiosity = noveltySignal * safetySignal;
        return 1.0 + curiosity * 0.5;
    }

    // ===== 状态描述 =====
    const char* state() const {
        if (noveltySignal > 0.5 && safetySignal > 0.5) return "Curious";
        if (safetySignal < 0.3) return "Fearful";
        if (noveltySignal < 0.2) return "Familiar";
        return "Neutral";
    }
};

#endif
