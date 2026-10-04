#ifndef AXON_INITIAL_SEGMENT_H
#define AXON_INITIAL_SEGMENT_H

#include <cmath>
#include "RandomUtils.h"
#include "NeuronType.h"

// ============================================================
// AxonInitialSegment - 轴突起始段
// ============================================================
//
// AIS 是动作电位的发起地。
// 它的参数从神经元类型读取，因为不同神经元类型的 AIS
// 有不同的通道密度和放电特性。
//
// 锥体细胞：概率性，高阈值，低 max rate
// 篮状细胞：可靠，低阈值，高 max rate
// 吊灯细胞：介于两者之间
//
// ============================================================

struct AxonInitialSegment {
    // ===== 状态 =====
    double v = -65.0;              // 膜电位 (mV)

    // ===== 参数（从类型读取）=====
    double threshold = -50.0;      // 阈值 (mV)
    double softness = 1.0;         // 阈值软化宽度 (mV)
    double max_rate_hz = 500.0;    // 最大放电率 (Hz)
    double g_coupling = 1.0;       // 与胞体的耦合强度
    double tau = 1.0;              // 跟踪胞体的时间常数 (ms)

    // ===== 不应期 =====
    double refractoryTimer = 0.0;
    double refractoryPeriod = 2.0; // ms

    // ===== 输出 =====
    bool hasSpiked = false;

    // ===== 构造：从类型初始化 =====
    AxonInitialSegment() = default;

    explicit AxonInitialSegment(const NeuronType& type) {
        threshold = type.ais_threshold;
        softness = type.ais_softness;
        max_rate_hz = type.ais_max_rate_hz;
        refractoryPeriod = type.ais_refractory_ms;
    }

    // ===== 主更新 =====
    void step(double somaV, double dt) {
        // 不应期处理
        if (refractoryTimer > 0.0) {
            refractoryTimer -= dt;
            hasSpiked = false;
            return;
        }

        // 跟踪胞体电位
        v += (somaV - v) * g_coupling / tau * dt;

        // sigmoid 放电率
        double x = (v - threshold) / softness;
        double p = 1.0 / (1.0 + std::exp(-x));
        double rate_hz = p * max_rate_hz;

        // 泊松放电
        hasSpiked = false;
        if (RandomUtils::uniform() < rate_hz * dt / 1000.0) {
            hasSpiked = true;
            refractoryTimer = refractoryPeriod;
        }
    }
};

#endif
