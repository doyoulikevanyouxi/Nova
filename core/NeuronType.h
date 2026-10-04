#ifndef NEURON_TYPE_H
#define NEURON_TYPE_H

#include <vector>
#include <string>

// ============================================================
// NeuronType - 神经元类型
// ============================================================
//
// 定义神经元的形态和电学特性。
// 树突形态是类型的一部分，不同神经元类型有不同结构。
//
// 锥体细胞：顶树突 + 基树突，2 簇，每簇 2 分支
// 篮状细胞：单簇，4 分支，无顶/基区分
// 吊灯细胞：单簇，3 分支，无顶/基区分
//
// ============================================================

// 簇类型
enum class ClusterType {
    Basal,      // 基树突（接收前馈输入）
    Apical,     // 顶树突（接收反馈输入）
    Uniform     // 无区分（篮状、吊灯）
};

struct NeuronType {
    // ===== 基本属性 =====
    std::string name = "Unknown";
    bool isExcitatory = true;

    // ===== 树突形态 =====
    int numClusters = 1;                        // 簇数
    std::vector<int> branchesPerCluster = {4};  // 每簇分支数
    std::vector<ClusterType> clusterTypes = {ClusterType::Uniform};
    bool hasApicalBasalDistinction = false;

    // ===== 树突默认参数 =====
    double defaultR_input = 50.0;
    double defaultTauDendrite = 20.0;
    double defaultThetaPlateau = -60.0;
    double defaultAPlateau = 1.0;
    double defaultTauPlateau = 100.0;

    // ===== 胞体参数 =====
    double soma_I_pace = 4.0;
    double soma_g_M = 0.5;
    double soma_sigma_noise = 0.05;

    // ===== AIS 参数 =====
    double ais_threshold = -50.0;
    double ais_softness = 1.0;
    double ais_max_rate_hz = 200.0;
    double ais_refractory_ms = 2.0;



    // ===== 胞体参数 =====
    double somaR_input = 50.0;
    double somatauDendrite = 20.0;
};

// ============================================================
// 预置类型
// ============================================================

// 锥体细胞：顶树突 + 基树突
inline NeuronType pyramidalType() {
    NeuronType t;
    t.name = "Pyramidal";
    t.isExcitatory = true;

    t.numClusters = 2;
    t.branchesPerCluster = {2, 2};
    t.clusterTypes = {ClusterType::Basal, ClusterType::Apical};
    t.hasApicalBasalDistinction = true;

    t.defaultR_input = 50.0;
    t.ais_threshold = -50.0;
    t.ais_softness = 1.5;
    t.ais_max_rate_hz = 200.0;
    t.ais_refractory_ms = 2.0;

    // 锥体细胞：低自发，强适应
    t.soma_I_pace = 3.0;
    t.soma_g_M = 0.5;
    t.soma_sigma_noise = 0.05;

    return t;
}

// 篮状细胞：单簇，4 分支
inline NeuronType basketType() {
    NeuronType t;
    t.name = "Basket";
    t.isExcitatory = false;

    t.numClusters = 1;
    t.branchesPerCluster = {4};
    t.clusterTypes = {ClusterType::Uniform};
    t.hasApicalBasalDistinction = false;

    t.defaultR_input = 60.0;
    t.ais_threshold = -55.0;
    t.ais_softness = 0.5;
    t.ais_max_rate_hz = 600.0;
    t.ais_refractory_ms = 1.0;

    // 篮状细胞：高自发，弱适应
    t.soma_I_pace = 3.0;
    t.soma_g_M = 0.3;
    t.soma_sigma_noise = 0.03;

    return t;
}

// 吊灯细胞：单簇，3 分支
inline NeuronType chandelierType() {
    NeuronType t;
    t.name = "Chandelier";
    t.isExcitatory = false;

    t.numClusters = 1;
    t.branchesPerCluster = {3};
    t.clusterTypes = {ClusterType::Uniform};
    t.hasApicalBasalDistinction = false;

    t.defaultR_input = 55.0;
    t.ais_threshold = -52.0;
    t.ais_softness = 1.0;
    t.ais_max_rate_hz = 400.0;
    t.ais_refractory_ms = 1.5;

    // 吊灯细胞：中等自发
    t.soma_I_pace = 3.2;        // 从 3.0 改为 3.2
    t.soma_sigma_noise = 0.03;  // 保持 0.03
    t.soma_g_M = 0.4;


    return t;
}

#endif
