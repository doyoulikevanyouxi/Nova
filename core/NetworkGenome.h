#ifndef NETWORK_GENOME_H
#define NETWORK_GENOME_H

#include <string>
#include <vector>
#include <utility>

// ============================================================
// NetworkGenome - 网络基因组
// ============================================================
//
// 定义网络的完整结构：
//   1. 有哪些区域（RegionGenome）
//   2. 区域之间怎么连接（ConnectionGenome）
//
// 这是"基因"：它决定了网络的初始架构。
// 生长和修剪过程会在此基础上修改网络。
//
// ============================================================

// 区域配置
struct RegionGenome {
    std::string name;
    int layer;
    double centerX, centerY, radius;
    int numPyramidal;
    int numBasket;
    int numChandelier;

    // 区域内部连接概率
    double p2pProb = 0.3;      // P→P 概率
    double excWeight = 1.0;    // 兴奋性突触权重
    double inhWeight = -3.0;   // 抑制性突触权重
};

// 连接配置
struct ConnectionGenome {
    std::string fromRegion;
    std::string toRegion;
    bool isFeedforward;

    // 显式映射（源神经元索引 → 目标神经元索引）
    // 如果为空，使用自动连接
    std::vector<std::pair<int, int>> explicitMapping;

    // 自动连接参数（当 explicitMapping 为空时使用）
    int connectionsPerNeuron = 2;

    double weight = 5.0;
    double delayMs = 5.0;
    bool isPlastic = false;
    bool isExcitatory = true;
};

// 网络基因组
struct NetworkGenome {
    std::vector<RegionGenome> regions;
    std::vector<ConnectionGenome> connections;
};

// ============================================================
// 默认视觉基因组
// ============================================================
inline NetworkGenome defaultVisualGenome() {
    NetworkGenome g;

    g.regions = {
        {"V1",    1, 50.0,  20.0, 12.0,  4,  2,  1},
        {"V2",    2, 50.0,  50.0, 12.0,  4,  2,  1},
        {"V4",    3, 50.0,  80.0, 12.0,  3,  1,  1},
        {"IT",    4, 50.0, 110.0, 18.0, 10,  3,  0},
        {"H",     5, 50.0, 150.0, 20.0, 30, 10,  0},
        {"PFC",   7, 20.0, 110.0, 15.0, 12,  3,  0},
        {"S1_3a", 9, 20.0, 150.0,  8.0,  3,  1,  0},
        {"S1_3b",10, 30.0, 160.0,  8.0,  4,  1,  0},
        {"S1_1", 11, 30.0, 180.0,  8.0,  3,  1,  0},
        {"S1_2", 12, 20.0, 200.0,  8.0,  2,  0,  0},
        {"BG",   13, 20.0,  70.0, 10.0,  4,  1,  0},
        {"THAL", 15, 50.0,  70.0, 10.0,  4,  1,  0},
        {"M1",   16, 80.0,  70.0, 10.0,  4,  1,  0}
    };
    // V1 -> V2
    ConnectionGenome v1v2;
    v1v2.fromRegion = "V1";
    v1v2.toRegion = "V2";
    v1v2.isFeedforward = true;
    v1v2.explicitMapping = {
        {0,0},{0,1},{0,2},{0,3},
        {1,0},{1,1},{1,2},{1,3},
        {2,0},{2,1},{2,2},{2,3},
        {3,0},{3,1},{3,2},{3,3}
    };
    v1v2.weight = 8.0;   // 3.0 -> 8.0
    v1v2.delayMs = 2.0;

    // V2 -> V4
    ConnectionGenome v2v4;
    v2v4.fromRegion = "V2";
    v2v4.toRegion = "V4";
    v2v4.isFeedforward = true;
    v2v4.explicitMapping = {
        {0,0},{0,1},{0,2},
        {1,0},{1,1},{1,2},
        {2,0},{2,1},{2,2},
        {3,0},{3,1},{3,2}
    };
    v2v4.weight = 8.0;   // 3.5 -> 8.0
    v2v4.delayMs = 2.0;

    // V4 -> IT（唯一输入通路）
    ConnectionGenome v4it;
    v4it.fromRegion = "V4";
    v4it.toRegion = "IT";
    v4it.isFeedforward = true;
    v4it.explicitMapping = {
        {0,0},{0,1},{0,2},{1,3},{1,4},{1,5},{2,6},{2,7},{2,8},{2,9}
    };
    v4it.weight = 10.0;  // 6.0 -> 10.0
    v4it.delayMs = 3.0;

    // IT -> H
    ConnectionGenome itH;
    itH.fromRegion = "IT";
    itH.toRegion = "H";
    itH.isFeedforward = true;
    itH.explicitMapping = {
        {0,2},{0,17},{1,11},{1,26},{2,10},{2,25},{3,8},{3,24},
        {4,1},{4,15},{5,5},{5,20},{6,13},{6,28},{7,7},{7,22},
        {8,4},{8,19},{9,0},{9,29}
    };
    itH.weight = 12.0;   // 8.0 -> 12.0
    itH.delayMs = 3.0;

    // 反馈
    ConnectionGenome itv4;
    itv4.fromRegion = "IT";
    itv4.toRegion = "V4";
    itv4.isFeedforward = false;
    itv4.explicitMapping = {{0,0},{3,1},{6,2}};
    itv4.weight = 2.0;
    itv4.delayMs = 5.0;

    ConnectionGenome v4v2;
    v4v2.fromRegion = "V4";
    v4v2.toRegion = "V2";
    v4v2.isFeedforward = false;
    v4v2.explicitMapping = {{0,0},{1,1},{2,2}};
    v4v2.weight = 2.0;
    v4v2.delayMs = 5.0;

    ConnectionGenome v2v1;
    v2v1.fromRegion = "V2";
    v2v1.toRegion = "V1";
    v2v1.isFeedforward = false;
    v2v1.explicitMapping = {{0,0},{1,1},{2,2},{3,3}};
    v2v1.weight = 2.0;
    v2v1.delayMs = 5.0;

    ConnectionGenome hIT;
    hIT.fromRegion = "H";
    hIT.toRegion = "IT";
    hIT.isFeedforward = false;
    hIT.explicitMapping = {
        {2,0},{8,3},{13,6},{5,8},{11,1},{1,4},{7,7},{4,9}
    };
    hIT.weight = 2.5;
    hIT.delayMs = 5.0;


    // S1_3a → S1_1
    ConnectionGenome s1_3a_to_1;
    s1_3a_to_1.fromRegion = "S1_3a";
    s1_3a_to_1.toRegion = "S1_1";
    s1_3a_to_1.isFeedforward = true;
    s1_3a_to_1.explicitMapping = {{0,0},{1,1},{2,2}};
    s1_3a_to_1.weight = 4.0;
    s1_3a_to_1.delayMs = 2.0;

    // S1_3b → S1_1
    ConnectionGenome s1_3b_to_1;
    s1_3b_to_1.fromRegion = "S1_3b";
    s1_3b_to_1.toRegion = "S1_1";
    s1_3b_to_1.isFeedforward = true;
    s1_3b_to_1.explicitMapping = {{0,0},{1,1},{2,2},{3,0}};
    s1_3b_to_1.weight = 4.0;
    s1_3b_to_1.delayMs = 2.0;

    // S1_1 → S1_2
    ConnectionGenome s1_1_to_2;
    s1_1_to_2.fromRegion = "S1_1";
    s1_1_to_2.toRegion = "S1_2";
    s1_1_to_2.isFeedforward = true;
    s1_1_to_2.explicitMapping = {{0,0},{1,1},{2,0}};
    s1_1_to_2.weight = 4.0;
    s1_1_to_2.delayMs = 2.0;

    // ===== IT → PFC =====
    ConnectionGenome itPFC;
    itPFC.fromRegion = "IT";
    itPFC.toRegion = "PFC";
    itPFC.isFeedforward = true;
    itPFC.explicitMapping = {
        {0,0},{0,1},{1,2},{1,3},{2,4},{2,5},{3,6},{3,7},{4,8},{4,9},
        {5,10},{5,11},{6,0},{6,2},{7,1},{7,3},{8,4},{8,6},{9,5},{9,7}
    };
    itPFC.weight = 5.0;
    itPFC.delayMs = 3.0;

    // ===== H → PFC =====
    ConnectionGenome hPFC;
    hPFC.fromRegion = "H";
    hPFC.toRegion = "PFC";
    hPFC.isFeedforward = true;
    hPFC.explicitMapping = {
        {0,0},{1,0},{2,0},{3,1},{4,1},{5,1},{6,2},{7,2},{8,2},
        {9,3},{10,3},{11,3},{12,4},{13,4},{14,4},{15,5},{16,5},{17,5},
        {18,6},{19,6},{20,6},{21,7},{22,7},{23,7},{24,8},{25,8},{26,8},
        {27,9},{28,9},{29,9}
    };
    hPFC.weight = 4.0;
    hPFC.delayMs = 4.0;

    // ===== S1_2 → PFC =====
    ConnectionGenome s1_2_to_PFC;
    s1_2_to_PFC.fromRegion = "S1_2";
    s1_2_to_PFC.toRegion = "PFC";
    s1_2_to_PFC.isFeedforward = true;
    s1_2_to_PFC.explicitMapping = {{0, 4}, {0, 5}, {1, 6}, {1, 7}};
    s1_2_to_PFC.weight = 5.0;
    s1_2_to_PFC.delayMs = 3.0;

    // ===== V4 → PFC =====
    ConnectionGenome v4PFC;
    v4PFC.fromRegion = "V4";
    v4PFC.toRegion = "PFC";
    v4PFC.isFeedforward = true;
    v4PFC.explicitMapping = {{0,8},{1,9},{2,10}};
    v4PFC.weight = 4.0;
    v4PFC.delayMs = 4.0;

    // ===== V2 → PFC =====
    ConnectionGenome v2PFC;
    v2PFC.fromRegion = "V2";
    v2PFC.toRegion = "PFC";
    v2PFC.isFeedforward = true;
    v2PFC.explicitMapping = {{0,11},{1,0},{2,1}};
    v2PFC.weight = 3.0;
    v2PFC.delayMs = 5.0;

    // ===== PFC → BG =====
    ConnectionGenome pfcBG;
    pfcBG.fromRegion = "PFC";
    pfcBG.toRegion = "BG";
    pfcBG.isFeedforward = true;
    pfcBG.explicitMapping = {
        {0,0},{1,1},{2,2},{3,3},{4,0},{5,1},{6,2},{7,3}
    };
    pfcBG.weight = 6.0;
    pfcBG.delayMs = 3.0;

    // ===== BG → THAL =====
    ConnectionGenome bgTHAL;
    bgTHAL.fromRegion = "BG";
    bgTHAL.toRegion = "THAL";
    bgTHAL.isFeedforward = true;
    bgTHAL.explicitMapping = {{0,0},{1,1},{2,2},{3,3}};
    bgTHAL.weight = 8.0;
    bgTHAL.delayMs = 2.0;

    // ===== THAL → M1 =====
    ConnectionGenome thalM1;
    thalM1.fromRegion = "THAL";
    thalM1.toRegion = "M1";
    thalM1.isFeedforward = true;
    thalM1.explicitMapping = {{0,0},{1,1},{2,2},{3,3}};
    thalM1.weight = 10.0;
    thalM1.delayMs = 2.0;

    // ===== THAL → PFC（反馈，闭合环路）=====
    ConnectionGenome thalPFC;
    thalPFC.fromRegion = "THAL";
    thalPFC.toRegion = "PFC";
    thalPFC.isFeedforward = false;
    thalPFC.explicitMapping = {{0,0},{1,1},{2,2},{3,3}};
    thalPFC.weight = 0.5;
    thalPFC.delayMs = 4.0;

    g.connections = {
        v1v2, v2v4, v4it, itH,               // 视觉前馈
        itv4, v4v2, v2v1, hIT,               // 视觉反馈
        s1_3a_to_1, s1_3b_to_1, s1_1_to_2,   // 体感通路
        itPFC, hPFC, s1_2_to_PFC, v4PFC, v2PFC,  // PFC 输入
        pfcBG, bgTHAL, thalM1, thalPFC       // 行动通路
    };

    return g;
}

#endif
