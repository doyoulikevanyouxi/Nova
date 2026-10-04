#ifndef DENDRITE_BRANCH_H
#define DENDRITE_BRANCH_H

#include <vector>
#include <cmath>
#include <algorithm>

// ============================================================
// DendriteBranch - 树突分支（第一级整合）
// ============================================================
//
// 分支是最小的整合单元，直接接收突触输入。
// 分支的位置是相对于它所属的簇的。
// 分支的输出经过衰减后传给簇。
//
// 信号流：
//   突触 → 分支（局部整合） → 衰减 → 簇
//
// ============================================================

struct DendriteBranch {
    // ===== 位置（相对于簇）=====
    double x = 0.0;   // 水平偏移
    double y = 0.0;   // 垂直偏移

    // ===== 局部电位 =====
    double d = -65.0;   // 局部膜电位 (mV)
    double p = 0.0;     // 平台电位 (mV)

    // ===== 输入缓冲 =====
    std::vector<double> inputs;
    std::vector<double> weights;

    // ===== 物理结构 =====
    double length = 1.0;
    double diameter = 1.0;

    // ===== 分支参数 =====
    double R_input = 50.0;
    double tau_dendrite = 20.0;
    double theta_plateau = -60.0;
    double A_plateau = 1.0;
    double tau_plateau = 100.0;

    // ===== 构造 =====
    DendriteBranch() = default;

    explicit DendriteBranch(int numInputs) {
        inputs.resize(numInputs, 0.0);
        weights.resize(numInputs, 0.1);
    }

    // ===== 到簇的距离 =====
    double distanceToCluster() const {
        return std::sqrt(x * x + y * y);
    }

    // ===== 空间常数 =====
    double spaceConstant() const {
        return std::sqrt(diameter) * 5.0;
    }

    // ===== 分支到簇的衰减 =====
    double attenuationToCluster() const {
        return std::exp(-distanceToCluster() / spaceConstant());
    }

    // ===== 重置输入 =====
    void clearInputs() {
        std::fill(inputs.begin(), inputs.end(), 0.0);
    }

    // ===== 主更新 =====
    void update(double dt) {
        const double d_rest = -65.0;

        // 总突触电流
        double I_syn = 0.0;
        for (size_t j = 0; j < inputs.size(); ++j) {
            I_syn += weights[j] * inputs[j];
        }

        // 膜方程
        double dd = (-(d - d_rest) + R_input * I_syn) / tau_dendrite;
        d += dd * dt;
        if (d > 0.0)   d = 0.0;
        if (d < -90.0) d = -90.0;

        // 平台电位
        double dp = -p / tau_plateau;
        if (d > theta_plateau) dp += A_plateau;
        p += dp * dt;
        if (p < 0.0)  p = 0.0;
        if (p > 50.0) p = 50.0;

        clearInputs();
    }

    // ===== 原始输出（未衰减）=====
    double rawOutput() const {
        const double d_rest = -65.0;
        double deviation = d - d_rest;
        double plateauContribution = p * 0.05;
        return deviation + plateauContribution;
    }

    // ===== 输出到簇（带衰减）=====
    double outputToCluster() const {
        return rawOutput() * attenuationToCluster();
    }
};

#endif
