#ifndef REGION_H
#define REGION_H

#include "NeuronType.h"
#include <string>
#include <vector>

// ============================================================
// Region - 区域
// ============================================================
//
// 一个区域是一组功能相关的神经元。
// 区域有位置、大小、层级，用于定义区域间连接规则。
//
// 例子：
//   V1（初级视觉皮层）：层级 1，接收外部输入
//   IT（颞下皮层）：层级 4，物体识别
//   PFC（前额叶）：层级 7，整合与决策
//
// ============================================================

struct Region {
    std::string name;              // 区域名，如 "V1"
    double centerX = 0.0;          // 中心位置
    double centerY = 0.0;
    double radius = 10.0;          // 区域半径
    int layer = 0;                 // 层级

    std::vector<int> neurons;      // 神经元 ID 列表

    Region() = default;

    Region(const std::string& n, double cx, double cy, double r, int l)
        : name(n), centerX(cx), centerY(cy), radius(r), layer(l) {}

    int size() const { return (int)neurons.size(); }
};

#endif
