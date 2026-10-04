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
        {"S1_3a", 9, 20.0, 150.0,  8.0,  3,  1,  0},
        {"S1_3b",10, 30.0, 160.0,  8.0,  4,  1,  0},
        {"S1_1", 11, 30.0, 180.0,  8.0,  3,  1,  0},
        {"S1_2", 12, 20.0, 200.0,  8.0,  2,  0,  0}
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

    g.connections = {
        v1v2, v2v4, v4it, itH,           // 前馈
        itv4, v4v2, v2v1, hIT,             // 反馈
        s1_3a_to_1,s1_3b_to_1,s1_1_to_2
    };

    return g;
}

#endif
