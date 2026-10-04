#ifndef DENDRITE_CLUSTER_H
#define DENDRITE_CLUSTER_H

#include "DendriteBranch.h"
#include "NeuronType.h"
#include <vector>
#include <cmath>

// ============================================================
// DendriteCluster - 树突簇（第二级整合）
// ============================================================
//
// 簇包含一组分支，做第二级整合。
// 簇的位置是相对于胞体的。
// 簇的输出经过衰减后传给胞体。
//
// 信号流：
//   分支（带衰减） → 簇（局部整合） → 衰减 → 胞体
//
// 簇类型：
//   Basal:  基树突，接收前馈输入
//   Apical: 顶树突，接收反馈输入
//   Uniform: 无区分（篮状、吊灯）
//
// ============================================================

struct DendriteCluster {
    // ===== 标识 =====
    int id = -1;
    ClusterType type = ClusterType::Uniform;

    // ===== 包含的分支索引 =====
    std::vector<int> branchIndices;

    // ===== 位置（相对于胞体）=====
    double x = 0.0;
    double y = 0.0;

    // ===== 局部电位 =====
    double d = -65.0;
    double p = 0.0;

    // ===== 物理结构 =====
    double length = 1.0;
    double diameter = 1.0;

    // ===== 簇参数 =====
    double R_input = 2.0;   // 1.0 -> 2.0
    double tau_dendrite = 10.0;   // 20.0 -> 10.0
    double theta_plateau = -60.0;
    double A_plateau = 1.0;
    double tau_plateau = 100.0;

    // ===== 构造 =====
    DendriteCluster() = default;

    DendriteCluster(int clusterId, ClusterType t) : id(clusterId), type(t) {}

    // ===== 添加分支 =====
    void addBranch(int branchIdx) {
        branchIndices.push_back(branchIdx);
    }

    // ===== 到胞体的距离 =====
    double distanceToSoma() const {
        return std::sqrt(x * x + y * y);
    }

    // ===== 空间常数 =====
    double spaceConstant() const {
        return std::sqrt(diameter) * 5.0;
    }

    // ===== 簇到胞体的衰减 =====
    double attenuationToSoma() const {
        return std::exp(-distanceToSoma() / spaceConstant());
    }

    // ===== 主更新 =====
    // 从分支收集输入，做局部整合
    void update(const std::vector<DendriteBranch>& branches, double dt) {
        const double d_rest = -65.0;

        // 从分支收集输入（带衰减）
        double I_fromBranches = 0.0;
        for (int idx : branchIndices) {
            I_fromBranches += branches[idx].outputToCluster();
        }

        // 膜方程
        double dd = (-(d - d_rest) + R_input * I_fromBranches) / tau_dendrite;
        d += dd * dt;
        if (d > 0.0)   d = 0.0;
        if (d < -90.0) d = -90.0;

        // 平台电位
        double dp = -p / tau_plateau;
        if (d > theta_plateau) dp += A_plateau;
        p += dp * dt;
        if (p < 0.0)  p = 0.0;
        if (p > 50.0) p = 50.0;
    }

    // ===== 输出到胞体（带衰减）=====
    double outputToSoma() const {
        const double d_rest = -65.0;
        double deviation = d - d_rest;
        double plateauContribution = p * 0.05;
        return (deviation + plateauContribution) * attenuationToSoma();
    }
};

#endif
