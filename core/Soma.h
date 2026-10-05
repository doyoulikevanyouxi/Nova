#ifndef SOMA_H
#define SOMA_H

#include "NeuronType.h"
#include "RandomUtils.h"
#include <cmath>

// ============================================================
// Soma - 胞体
// ============================================================
//
// 胞体整合所有簇的输入，产生亚阈值电位传给 AIS。
//
// 胞体不产生动作电位——那是 AIS 的职责。
// 所以胞体不需要快速 Na 和 Kv 通道。
//
// 胞体只有：
//   I_leak  - 漏电流（被动）
//   I_pace  - 恒定起搏电流（自发放电的核心）
//   I_M     - M 型 K（慢适应，增益控制）
//   noise   - 随机涨落
//
// 自发放电由 I_pace 驱动：
//   - I_pace 高 → 静息电位接近阈值 → 自发率高
//   - I_pace 低 → 静息电位远离阈值 → 自发率低
//
// ============================================================

struct Soma {
    // ===== 状态 =====
    double v = -65.0;
    double m_M = 0.0;
    double ca_soma = 0.0;          // 胞体钙浓度（驱动 SK）

    // ===== 膜参数 =====
    double C_m = 1.0;
    double g_L = 0.3;
    double g_M = 0.5;
    double g_SK = 3.0;             // SK 通道电导（新增）
    double E_L = -65.0;
    double E_K = -77.0;

    // ===== 起搏 =====
    double I_pace = 4.0;
    double sigma_noise = 0.05;

    // ===== 门控时间常数 =====
    double tau_m_M = 50.0;
    double tau_ca = 150.0;         // 钙衰减（新增）

    // ===== 每次放电的钙增量 =====
    double ca_increment = 0.1;     // 新增

    // ===== M 通道稳态 =====
    double m_M_inf(double V) const {
        return 1.0 / (1.0 + std::exp(-(V + 35.0) / 10.0));
    }

    // ===== 放电事件：钙内流 =====
    void onSpike() {
        ca_soma += ca_increment;
        if (ca_soma > 1.0) ca_soma = 1.0;
    }

    // ===== 主更新 =====
    void step(double I_from_clusters, double dt) {
        // M 通道门控
        m_M += (m_M_inf(v) - m_M) / tau_m_M * dt;
        if (m_M < 0.0) m_M = 0.0;
        if (m_M > 1.0) m_M = 1.0;

        // 钙衰减
        ca_soma += (-ca_soma / tau_ca) * dt;
        if (ca_soma < 0.0) ca_soma = 0.0;

        // 电流
        double I_leak = g_L * (v - E_L);
        double I_M = g_M * m_M * (v - E_K);
        double I_SK = g_SK * ca_soma * ca_soma * (v - E_K);
        double noise = sigma_noise * RandomUtils::gaussian();

        // 膜方程
        double dv = (-I_leak - I_M - I_SK + I_pace
                     + I_from_clusters + noise) / C_m;
        v += dv * dt;

        // 钳制
        if (v < -90.0) v = -90.0;
        if (v > -30.0) v = -30.0;
    }

    // ===== 重置 =====
    void reset() {
        v = -65.0;
        m_M = 0.0;
        ca_soma = 0.0;
    }
};

#endif
