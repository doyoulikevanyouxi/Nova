#ifndef SYNAPSE_H
#define SYNAPSE_H

#include "RandomUtils.h"
#include <cmath>

// ============================================================
// Synapse - 突触
// ============================================================
//
// 突触包含三部分功能：
//   1. 量子释放：突触前脉冲 → 概率释放 → 递质沉积
//   2. 电流衰减：AMPA（快，5ms）+ NMDA（慢，100ms）
//   3. STDP：突触前/后脉冲时序 → 权重可塑性
//
// STDP 规则：
//   当 post 放电，若 pre 最近放过 → LTP（增强）
//   当 pre 放电，若 post 最近放过 → LTD（减弱）
//
// 多巴胺门控：DA > 0.6 时，STDP 才生效。
//
// ============================================================

struct Synapse {
    // ===== 拓扑 =====
    int preId = -1;             // 突触前神经元 ID
    int postId = -1;            // 突触后神经元 ID
    int postBranch = 0;         // 突触后分支索引
    int postInput = 0;          // 突触后分支内的输入索引

    // ===== 传递参数 =====
    double weight = 1.0;        // 突触权重
    double delayMs = 1.0;       // 传递延迟 (ms)
    bool isExcitatory = true;   // 兴奋性/抑制性

    // ===== 量子释放参数 =====
    int nReleaseSites = 5;      // 释放位点数
    double releaseProb = 0.3;   // 每个位点的释放概率
    double quantalSize = 0.5;   // 量子大小

    // ===== 突触电流状态 =====
    double I_ampa = 0.0;        // AMPA 电流
    double I_nmda = 0.0;        // NMDA 电流

    // ===== 电流时间常数 =====
    double tau_ampa = 5.0;      // AMPA 衰减时间常数 (ms)
    double tau_nmda = 100.0;    // NMDA 衰减时间常数 (ms)

    // ===== AMPA/NMDA 比例 =====
    double ampaFraction = 0.7;
    double nmdaFraction = 0.3;

    // ===== STDP 参数 =====
    bool isPlastic = true;      // 是否参与 STDP
    double ltpAmplitude = 0.01;   // A_plus（LTP 幅度）
    double ltdAmplitude = 0.012;  // A_minus（LTD 幅度，略大防止爆炸）
    double tauSTDP = 20.0;        // STDP 时间窗口 (ms)
    double weightMin = 0.0;       // 权重下界
    double weightMax = 10.0;      // 权重上界

    // ===== STDP 状态 =====
    double lastPreSpikeTime = -1e9;    // 上次突触前放电时刻
    double lastPostSpikeTime = -1e9;   // 上次突触后放电时刻

    // ============================================================
    // 量子释放
    // ============================================================
    // 给定突触前是否放电，返回释放的递质量
    double release(bool preSpiked) {
        if (!preSpiked) return 0.0;

        int released = 0;
        for (int i = 0; i < nReleaseSites; ++i) {
            if (RandomUtils::uniform() < releaseProb) {
                released++;
            }
        }

        return released * quantalSize * weight;
    }

    // ============================================================
    // 触发：把释放量分配到 AMPA 和 NMDA
    // ============================================================
    void trigger(double amount) {
        I_ampa += amount * ampaFraction;
        I_nmda += amount * nmdaFraction;
    }

    // ============================================================
    // 电流衰减
    // ============================================================
    void decayCurrents(double dt) {
        I_ampa *= std::exp(-dt / tau_ampa);
        I_nmda *= std::exp(-dt / tau_nmda);
    }

    // ============================================================
    // 当前总电流（不含 Mg2+ 阻断，由 Network 层处理）
    // ============================================================
    double current() const {
        return I_ampa + I_nmda;
    }

    // ============================================================
    // STDP：突触前放电
    // ============================================================

    double onPreSpike(double t, double dopamine, double ach = 0.5) {
        lastPreSpikeTime = t;

        if (!isPlastic) return 0.0;
        if (dopamine < 0.6) return 0.0;

        double dt = t - lastPostSpikeTime;
        double dw = 0.0;

        if (dt > 0.0 && dt < 5.0 * tauSTDP) {
            double daMod = (dopamine - 0.6) / 0.4;
            double achMod = 0.5 + ach;   // 0.5 到 1.5
            dw = -ltdAmplitude * std::exp(-dt / tauSTDP) * daMod * achMod;
        }

        weight += dw;
        if (isExcitatory) {
            if (weight < weightMin) weight = weightMin;
            if (weight > weightMax) weight = weightMax;
        } else {
            if (weight > weightMin) weight = weightMin;
            if (weight < weightMax) weight = weightMax;
        }

        return dw;
    }

    // ===== STDP：突触后放电 =====
    double onPostSpike(double t, double dopamine, double ach = 0.5) {
        lastPostSpikeTime = t;

        if (!isPlastic) return 0.0;
        if (dopamine < 0.6) return 0.0;

        double dt = t - lastPreSpikeTime;
        double dw = 0.0;

        if (dt > 0.0 && dt < 5.0 * tauSTDP) {
            double daMod = (dopamine - 0.6) / 0.4;
            double achMod = 0.5 + ach;
            dw = ltpAmplitude * std::exp(-dt / tauSTDP) * daMod * achMod;
        }

        weight += dw;
        if (isExcitatory) {
            if (weight < weightMin) weight = weightMin;
            if (weight > weightMax) weight = weightMax;
        } else {
            if (weight > weightMin) weight = weightMin;
            if (weight < weightMax) weight = weightMax;
        }

        return dw;
    }

    // ============================================================
    // 理论平均释放量
    // ============================================================
    double meanRelease() const {
        return nReleaseSites * releaseProb * quantalSize * weight;
    }

    // ============================================================
    // 重置
    // ============================================================
    void resetCurrents() {
        I_ampa = 0.0;
        I_nmda = 0.0;
    }

    void resetSTDP() {
        lastPreSpikeTime = -1e9;
        lastPostSpikeTime = -1e9;
    }
};

#endif
