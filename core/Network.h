#ifndef NETWORK_H
#define NETWORK_H

#include "NetworkGenome.h"
#include "Neuron.h"
#include "SynapsePopulation.h"
#include <vector>
#include <cmath>
#include "Region.h"
#include "MetabolicPool.h"
#include "ValenceSystem.h"
#include "CuriosityInterface.h"

#include <map>

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
    std::map<std::string, Region> regions;   // 新增
    ValenceSystem valenceSystem;
    CuriosityInterface curiosityInterface;
    double currentTime = 0.0;
    // ===== 食物注入 =====
    double externalFood = 0.0;

    // ===== 调质系统 =====
    double dopamine = 0.5;         // DA：STDP 门控
    double acetylcholine = 0.5;    // ACh：学习率调制
    double serotonin = 0.5;        // 5-HT：抑制 DA（功能待加）
    // ===== 代谢系统 =====
    MetabolicPool metabolicPool;
    // ===== 体感系统 =====
    std::vector<int> s1_3a_neurons;
    std::vector<int> s1_3b_neurons;
    std::vector<int> s1_1_neurons;
    std::vector<int> s1_2_neurons;

    // ===== 行动系统 =====
    std::vector<int> pfcNeurons;
    std::vector<int> bgNeurons;
    std::vector<int> thalNeurons;
    std::vector<int> m1Neurons;

    double touchSignal = 0.0;
    double painSignal = 0.0;
    double bodyIntegrity = 1.0;

    // ===== 外部输入 =====
    double externalTouch = -1.0;   // -1 表示无输入
    double externalPain = -1.0;
    double externalDamage = 0.0;   // 额外损伤

    // ===== 控制标志 =====
    bool daFromValence = false;    // DA 由效价驱动
    bool daFromCuriosity = false;  // DA 由好奇心驱动
    bool htFromCuriosity = false;  // 5-HT 由好奇心驱动



    // ===== 创建区域 =====
    void addRegion(const std::string& name,
                   double cx, double cy,
                   double radius, int layer) {
        regions[name] = Region(name, cx, cy, radius, layer);
    }

    // ===== 在区域里添加神经元 =====
    int addNeuronToRegion(const std::string& regionName,
                          const NeuronType& type) {
        auto it = regions.find(regionName);
        if (it == regions.end()) return -1;

        int id = addNeuron(type);
        it->second.neurons.push_back(id);
        return id;
    }

    // ===== 获取区域 =====
    const Region* getRegion(const std::string& name) const {
        auto it = regions.find(name);
        return (it != regions.end()) ? &it->second : nullptr;
    }

    Region* getRegion(const std::string& name) {
        auto it = regions.find(name);
        return (it != regions.end()) ? &it->second : nullptr;
    }


    // ===== 添加神经元 =====
    int addNeuron(const NeuronType& type) {
        int id = (int)neurons.size();
        neurons.emplace_back(id, type);

        neurons[id].onSpikeCallback =
            [this](int neuronId,
                   const std::vector<int>& outgoing,
                   const std::vector<int>& incoming) {
                for (int sid : outgoing) {
                    synapses.synapses[sid].onPreSpike(
                        currentTime, dopamine, acetylcholine);
                }
                for (int sid : incoming) {
                    synapses.synapses[sid].onPostSpike(
                        currentTime, dopamine, acetylcholine);
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
        // 1. 统计上一步总放电数
        int totalSpikes = 0;
        for (auto& n : neurons) {
            if (n.hasSpiked) totalSpikes++;
        }

        // 2. 更新代谢池
        metabolicPool.update(totalSpikes, externalFood, dt);
        // 3. 体感系统（在效价之前，因为它影响效价）
        updateBodySense(dt);
        // 4. 更新效价
        valenceSystem.update(metabolicPool.glucose, dt);
        // 5. 调质合成
        //    DA：基础 + 效价 + 好奇心
        if (daFromValence) {
            dopamine = valenceSystem.getDopamineModulation();
        }
        if (daFromCuriosity) {
            dopamine = std::min(1.0,
                                dopamine + curiosityInterface.getDopamineBoost());
        }

        //    6-HT：基础 + 好奇心（危险）
        if (htFromCuriosity) {
            serotonin = std::min(1.0,
                                 serotonin + curiosityInterface.getSerotoninBoost());
        }
        //7. 突触群更新：衰减电流 + 延迟投递
        synapses.step(currentTime, dt,
                      [this](int postId, int branch, int input, double amount) {
                          neurons[postId].deliver(branch, input, amount);
                      },
                      [this](int neuronId) {
                          return neurons[neuronId].soma.v;
                      });

        // 8. 神经元更新
        //    放电时 onSpikeCallback 自动触发 STDP + 释放
        for (auto& n : neurons) {
            n.step(dt);
        }

        // 9. 时间推进
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

    // ===== 从基因组构建网络 =====
    void buildFromGenome(const NetworkGenome& genome) {
        // 清空
        neurons.clear();
        synapses.clear();
        regions.clear();
        currentTime = 0.0;

        // 1. 创建区域 + 神经元
        for (auto& rg : genome.regions) {
            addRegion(rg.name, rg.centerX, rg.centerY,
                      rg.radius, rg.layer);

            for (int i = 0; i < rg.numPyramidal; ++i)
                addNeuronToRegion(rg.name, pyramidalType());
            for (int i = 0; i < rg.numBasket; ++i)
                addNeuronToRegion(rg.name, basketType());
            for (int i = 0; i < rg.numChandelier; ++i)
                addNeuronToRegion(rg.name, chandelierType());
        }

        // 2. 区域内连接
        for (auto& rg : genome.regions) {
            buildIntraRegion(rg);
        }

        // 3. 区域间连接
        for (auto& cg : genome.connections) {
            buildInterRegion(cg);
        }

        // 4. 给所有新神经元设置放电回调
        //    （因为 clear 会删掉回调）
        for (auto& n : neurons) {
            n.onSpikeCallback =
                [this](int neuronId,
                       const std::vector<int>& outgoing,
                       const std::vector<int>& incoming) {
                    for (int sid : outgoing) {
                        synapses.synapses[sid].onPreSpike(
                            currentTime, dopamine, acetylcholine);
                    }
                    for (int sid : incoming) {
                        synapses.synapses[sid].onPostSpike(
                            currentTime, dopamine, acetylcholine);
                    }
                    synapses.onPresynapticSpike(neuronId, currentTime);
                };
        }
        initS1();
        initActionSystem();
    }
    // ===== 喂食 =====
    void feed(double amount) {
        externalFood = amount;
    }

    void clearFood() {
        externalFood = 0.0;
    }

    // ===== 初始化 S1 =====
    void initS1() {
        auto collect = [&](const char* name, std::vector<int>& out) {
            auto* r = getRegion(name);
            if (!r) return;
            for (int id : r->neurons) {
                if (neurons[id].type.name == "Pyramidal") {
                    out.push_back(id);
                }
            }
        };
        collect("S1_3a", s1_3a_neurons);
        collect("S1_3b", s1_3b_neurons);
        collect("S1_1", s1_1_neurons);
        collect("S1_2", s1_2_neurons);
    }

    // ===== 更新体感 =====
    void updateBodySense(double dt) {
        // 1. 身体完整性：损伤后恢复
        bodyIntegrity += (-externalDamage + 0.01 * (1.0 - bodyIntegrity)) * dt;
        if (bodyIntegrity < 0.0) bodyIntegrity = 0.0;
        if (bodyIntegrity > 1.0) bodyIntegrity = 1.0;
        externalDamage = 0.0;   // 损伤只在一步生效

        // 2. 触觉信号
        if (externalTouch >= 0.0) touchSignal = externalTouch;
        else touchSignal *= 0.9;   // 衰减

        // 3. 痛觉信号
        double intrinsicPain = (1.0 - bodyIntegrity) * 2.0;
        if (externalPain >= 0.0) painSignal = externalPain + intrinsicPain;
        else painSignal = intrinsicPain;

        if (painSignal > 1.0) painSignal = 1.0;

        // 4. 痛觉门控触觉
        double gatedTouch = touchSignal;
        if (painSignal > 0.3) {
            gatedTouch *= (1.0 - painSignal);
        }

        // 5. 注入 S1_3b（触觉）
        for (int id : s1_3b_neurons) {
            for (size_t b = 0; b < neurons[id].branches.size(); ++b) {
                for (size_t i = 0; i < neurons[id].branches[b].inputs.size(); ++i) {
                    neurons[id].deliver((int)b, (int)i, gatedTouch * 50.0 / 32.0);
                }
            }
        }

        // 6. 注入 S1_3a（痛觉）
        for (int id : s1_3a_neurons) {
            for (size_t b = 0; b < neurons[id].branches.size(); ++b) {
                for (size_t i = 0; i < neurons[id].branches[b].inputs.size(); ++i) {
                    neurons[id].deliver((int)b, (int)i, painSignal * 50.0 / 32.0);
                }
            }
        }

        // 7. 体感影响效价（痛觉 → 负效价，触觉 → 正效价）
        if (painSignal > 0.2) {
            valenceSystem.integrator -= painSignal * 0.5 * dt;
            if (valenceSystem.integrator < -1.0) valenceSystem.integrator = -1.0;
        }
        if (gatedTouch > 0.2 && painSignal < 0.1) {
            valenceSystem.integrator += gatedTouch * 0.05 * dt;
            if (valenceSystem.integrator > 1.0) valenceSystem.integrator = 1.0;
        }
    }

    // ===== 初始化行动系统 =====
    void initActionSystem() {
        auto collect = [&](const char* name, std::vector<int>& out) {
            auto* r = getRegion(name);
            if (!r) return;
            for (int id : r->neurons) {
                if (neurons[id].type.name == "Pyramidal") {
                    out.push_back(id);
                }
            }
        };
        collect("PFC", pfcNeurons);
        collect("BG", bgNeurons);
        collect("THAL", thalNeurons);
        collect("M1", m1Neurons);
    }

    // ===== 计算 actionOutput =====
    // 从 M1 的平均放电率计算行动标量
    // 返回 [-1, +1]
    // ===== 计算 actionOutput =====
    double getActionOutput() const {
        if (m1Neurons.empty()) return 0.0;

        double sum = 0.0;
        for (int id : m1Neurons) {
            sum += neurons[id].firingRate;
        }
        double avgM1 = sum / m1Neurons.size();

        // 死区：低于 5 Hz 视为安静
        double deadZone = 0.005;
        if (avgM1 <= deadZone) return 0.0;

        // 增益：20 Hz 时接近饱和
        return std::tanh((avgM1 - deadZone) * 30.0);
    }

    // ===== 计算 PFC 注意力 =====
    double getPFCAttention() const {
        if (pfcNeurons.empty()) return 0.0;

        double sum = 0.0;
        for (int id : pfcNeurons) {
            sum += neurons[id].firingRate;
        }
        double avgPFC = sum / pfcNeurons.size();

        // 死区：低于 3 Hz 视为无注意
        double deadZone = 0.003;
        if (avgPFC <= deadZone) return 0.0;

        double attention = (avgPFC - deadZone) * 30.0;
        if (attention > 1.0) attention = 1.0;
        return attention;
    }

    // ===== 外部接口 =====
    void applyTouch(double amount) { externalTouch = amount; }
    void applyPain(double amount) { externalPain = amount; }
    void applyDamage(double amount) { externalDamage += amount; }
private:
    // ===== 区域内连接 =====
    void buildIntraRegion(const RegionGenome& rg) {
        auto* region = getRegion(rg.name);
        if (!region) return;

        bool isH = (rg.name == "H");

        std::vector<int> pyramidal, basket, chandelier;
        for (int id : region->neurons) {
            const std::string& t = neurons[id].type.name;
            if (t == "Pyramidal") pyramidal.push_back(id);
            else if (t == "Basket") basket.push_back(id);
            else chandelier.push_back(id);
        }

        // ===== P → P =====
        for (size_t i = 0; i < pyramidal.size(); ++i) {
            for (size_t j = i + 1; j < pyramidal.size(); ++j) {
                if (RandomUtils::uniform() < rg.p2pProb) {
                    // H 的递归突触：低权重，纯 AMPA
                    double w = isH ? 0.05 : rg.excWeight;
                    int id1 = connect(pyramidal[i], pyramidal[j], 0, 0,
                                      w, 1.0, true);
                    int id2 = connect(pyramidal[j], pyramidal[i], 0, 0,
                                      w, 1.0, true);

                    if (isH) {
                        synapses.synapses[id1].ampaFraction = 1.0;
                        synapses.synapses[id1].nmdaFraction = 0.0;
                        synapses.synapses[id1].tau_ampa = 5.0;
                        synapses.synapses[id2].ampaFraction = 1.0;
                        synapses.synapses[id2].nmdaFraction = 0.0;
                        synapses.synapses[id2].tau_ampa = 5.0;
                    }
                }
            }
        }

        // ===== P → B =====
        for (int p : pyramidal) {
            for (int b : basket) {
                double w = isH ? 1.0 : rg.excWeight * 2.0;
                connect(p, b, 0, 0, w, 1.0, true);
            }
        }

        // ===== B → P（H 的抑制要强）=====
        for (int b : basket) {
            for (int p : pyramidal) {
                double w = isH ? 30.0 : std::abs(rg.inhWeight);
                int id = connect(b, p, 0, 0, w, 1.0, false);
                synapses.synapses[id].isPlastic = false;
            }
        }

        // ===== C → P =====
        for (int c : chandelier) {
            for (int p : pyramidal) {
                double w = isH ? 15.0 : std::abs(rg.inhWeight) * 0.7;
                int id = connect(c, p, 0, 0, w, 1.0, false);
                synapses.synapses[id].isPlastic = false;
            }
        }
    }
    // ===== 区域间连接 =====
    void buildInterRegion(const ConnectionGenome& cg) {
        auto* from = getRegion(cg.fromRegion);
        auto* to = getRegion(cg.toRegion);
        if (!from || !to) return;

        // 收集 pyramidal 神经元
        std::vector<int> fromP, toP;
        for (int id : from->neurons) {
            if (neurons[id].type.name == "Pyramidal") fromP.push_back(id);
        }
        for (int id : to->neurons) {
            if (neurons[id].type.name == "Pyramidal") toP.push_back(id);
        }

        if (fromP.empty() || toP.empty()) return;

        // 显式映射
        if (!cg.explicitMapping.empty()) {
            for (auto& [srcIdx, tgtIdx] : cg.explicitMapping) {
                if (srcIdx < (int)fromP.size() &&
                    tgtIdx < (int)toP.size()) {
                    int id = connect(fromP[srcIdx], toP[tgtIdx], 0, 0,
                                     cg.weight, cg.delayMs,
                                     cg.isExcitatory);
                    synapses.synapses[id].isPlastic = cg.isPlastic;
                }
            }
        } else {
            // 自动连接
            for (int srcId : fromP) {
                for (int k = 0; k < cg.connectionsPerNeuron; ++k) {
                    int tgtId = toP[k % toP.size()];
                    int id = connect(srcId, tgtId, 0, 0,
                                     cg.weight, cg.delayMs,
                                     cg.isExcitatory);
                    synapses.synapses[id].isPlastic = cg.isPlastic;
                }
            }
        }
    }
};

#endif
