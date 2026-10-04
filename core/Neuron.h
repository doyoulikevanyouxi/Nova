#ifndef NEURON_H
#define NEURON_H

#include "NeuronType.h"
#include "DendriteBranch.h"
#include "DendriteCluster.h"
#include "Soma.h"
#include "AxonInitialSegment.h"
#include <vector>
#include <functional>
#include <cmath>

// ============================================================
// Neuron - 完整神经元
// ============================================================
//
// 神经元是自主的：它放电时，自己触发 STDP 和突触释放。
// 不需要外部框架统一处理。
//
// 数据流：
//   突触 → 分支 → 簇 → 胞体 → AIS → hasSpiked
//                                        ↓
//                            触发 onSpikeCallback
//                                        ↓
//                    Network 处理 STDP 和释放
//
// ============================================================

class Neuron {
public:
    // ===== 标识 =====
    int id = -1;
    NeuronType type;

    // ===== 组件 =====
    std::vector<DendriteBranch> branches;
    std::vector<DendriteCluster> clusters;
    Soma soma;
    AxonInitialSegment ais;

    // ===== 输出 =====
    bool hasSpiked = false;

    // ===== 突触列表（由 Network 填充）=====
    std::vector<int> outgoingSynapses;
    std::vector<int> incomingSynapses;

    // ===== 放电回调（由 Network 设置）=====
    std::function<void(int,
                       const std::vector<int>&,
                       const std::vector<int>&)> onSpikeCallback;

    // ===== 默认构造 =====
    Neuron() = default;

    // ===== 构造：根据类型初始化 =====
    Neuron(int neuronId, const NeuronType& neuronType) {
        id = neuronId;
        type = neuronType;

        int totalBranches = 0;
        for (int k : type.branchesPerCluster) totalBranches += k;

        const int inputsPerBranch = 8;
        for (int i = 0; i < totalBranches; ++i) {
            branches.emplace_back(inputsPerBranch);
            branches[i].R_input = type.defaultR_input;
            for (auto& w : branches[i].weights) w = 1.0;
        }

        int branchIdx = 0;
        for (int c = 0; c < type.numClusters; ++c) {
            DendriteCluster cluster(c, type.clusterTypes[c]);

            if (type.clusterTypes[c] == ClusterType::Basal) {
                cluster.x = 0.0;
                cluster.y = -5.0;
            } else if (type.clusterTypes[c] == ClusterType::Apical) {
                cluster.x = 0.0;
                cluster.y = 5.0;
            } else {
                cluster.x = 0.0;
                cluster.y = 0.0;
            }
            cluster.diameter = 1.0;

            int branchesInCluster = type.branchesPerCluster[c];
            for (int k = 0; k < branchesInCluster; ++k) {
                cluster.addBranch(branchIdx);

                double angle = 2.0 * M_PI * k / branchesInCluster;
                double radius = 3.0;
                branches[branchIdx].x = radius * std::cos(angle);
                branches[branchIdx].y = radius * std::sin(angle);
                branches[branchIdx].diameter = 1.0;

                branchIdx++;
            }
            clusters.push_back(cluster);
        }

        for (auto& c : clusters) {
            c.R_input = 2.0;
            c.tau_dendrite = 10.0;
        }
        soma.I_pace = type.soma_I_pace;
        soma.g_M = type.soma_g_M;
        soma.sigma_noise = type.soma_sigma_noise;

        ais = AxonInitialSegment(type);
    }

    // ===== 突触投递 =====
    void deliver(int branchIdx, int inputIdx, double amount) {
        if (branchIdx < 0 || branchIdx >= (int)branches.size()) return;
        if (inputIdx < 0 || inputIdx >= (int)branches[branchIdx].inputs.size()) return;
        branches[branchIdx].inputs[inputIdx] += amount;
    }

    // ===== 主更新 =====
    void step(double dt) {
        // 1. 分支整合
        for (auto& b : branches) b.update(dt);

        // 2. 簇整合
        for (auto& c : clusters) c.update(branches, dt);

        // 3. 簇输出汇总
        double I_from_clusters = 0.0;
        for (auto& c : clusters) {
            I_from_clusters += c.outputToSoma();
        }

        // 4. 胞体整合
        soma.step(I_from_clusters, dt);

        // 5. AIS 决定放电
        ais.step(soma.v, dt);
        hasSpiked = ais.hasSpiked;

        // 6. 放电时自主通知
        //    这是神经元的内在行为，不需要外部触发
        if (hasSpiked && onSpikeCallback) {
            onSpikeCallback(id, outgoingSynapses, incomingSynapses);
        }
    }

    // ===== 重置 =====
    void reset() {
        for (auto& b : branches) {
            b.d = -65.0;
            b.p = 0.0;
            b.clearInputs();
        }
        for (auto& c : clusters) {
            c.d = -65.0;
            c.p = 0.0;
        }
        soma.reset();
        ais.v = -65.0;
        ais.refractoryTimer = 0.0;
        hasSpiked = false;
    }

    int branchCount() const { return (int)branches.size(); }
    int clusterCount() const { return (int)clusters.size(); }
    int totalInputChannels() const {
        int total = 0;
        for (auto& b : branches) total += (int)b.inputs.size();
        return total;
    }
};

#endif
