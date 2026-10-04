#ifndef NETWORK_H
#define NETWORK_H

#include "Neuron.h"
#include "SynapsePopulation.h"
#include <vector>
#include <cmath>
#include <algorithm>

// ============================================================
// Network - 神经元网络
// ============================================================
//
// 神经元是自主的，Network 只负责：
//   1. 管理神经元和突触的容器
//   2. 每步投递突触电流
//   3. 给每个神经元设置回调（STDP + 释放）
//
// DA 是一个简单的标量：
//   - 默认 0.5（低于 STDP 阈值 0.6，STDP 不生效）
//   - 手动设置（测试时）
//   - 没有动力学，没有脉冲，没有衰减
//
// ============================================================

class Network {
public:
    std::vector<Neuron> neurons;
    SynapsePopulation synapses;
    double currentTime = 0.0;

    // ===== DA（简单标量）=====
    double dopamine = 0.5;

    // ===== 添加神经元 =====
    int addNeuron(const NeuronType& type) {
        int id = (int)neurons.size();
        neurons.emplace_back(id, type);

        neurons[id].onSpikeCallback =
            [this](int neuronId,
                   const std::vector<int>& outgoing,
                   const std::vector<int>& incoming) {
                for (int sid : outgoing) {
                    synapses.synapses[sid].onPreSpike(currentTime, dopamine);
                }
                for (int sid : incoming) {
                    synapses.synapses[sid].onPostSpike(currentTime, dopamine);
                }
                synapses.onPresynapticSpike(neuronId, currentTime);
            };

        return id;
    }

    // ===== 连接 =====
    int connect(int preId, int postId,
                int postBranch, int postInput,
                double weight,
                double delayMs = 1.0,
                bool isExcitatory = true) {
        Synapse s;
        s.preId = preId;
        s.postId = postId;
        s.postBranch = postBranch;
        s.postInput = postInput;
        s.weight = weight;
        s.delayMs = delayMs;
        s.isExcitatory = isExcitatory;

        if (!isExcitatory) {
            // 抑制性：GABA_A
            s.weight = -std::abs(weight);
            s.weightMin = -10.0;
            s.weightMax = 0.0;
            s.ampaFraction = 1.0;    // 纯 GABA_A
            s.nmdaFraction = 0.0;
            s.tau_ampa = 8.0;        // GABA_A 时间常数
        }

        int synId = synapses.addSynapse(s);
        neurons[preId].outgoingSynapses.push_back(synId);
        neurons[postId].incomingSynapses.push_back(synId);
        return synId;
    }
    // ===== 主更新 =====
    void step(double dt) {
        // 1. 突触群更新：衰减电流 + 延迟投递
        synapses.step(currentTime, dt,
                      [this](int postId, int branch, int input, double amount) {
                          neurons[postId].deliver(branch, input, amount);
                      },
                      [this](int neuronId) {
                          return neurons[neuronId].soma.v;
                      });

        // 2. 神经元更新
        //    放电时 onSpikeCallback 自动触发 STDP + 释放
        for (auto& n : neurons) {
            n.step(dt);
        }

        // 3. 时间推进
        currentTime += dt;
    }

    // ===== 重置 =====
    void reset() {
        for (auto& n : neurons) n.reset();
        for (auto& s : synapses.synapses) {
            s.resetCurrents();
            s.resetSTDP();
        }
        synapses.pending.clear();
        currentTime = 0.0;
        // dopamine 不重置，由用户控制
    }

    int neuronCount() const { return (int)neurons.size(); }
    int synapseCount() const { return synapses.size(); }

    double getWeight(int synId) const {
        return synapses.synapses[synId].weight;
    }
};

#endif
