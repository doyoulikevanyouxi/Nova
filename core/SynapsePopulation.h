#ifndef SYNAPSE_POPULATION_H
#define SYNAPSE_POPULATION_H

#include "Synapse.h"
#include <vector>
#include <functional>

// ============================================================
// SynapsePopulation - 突触群
// ============================================================
//
// 每个时间步做三件事：
//   1. 衰减所有突触的电流
//   2. 处理延迟队列（到达的释放触发突触电流）
//   3. 投递当前电流到突触后
//
// 投递回调：(postId, branchIdx, inputIdx, currentAmount)
//
// ============================================================

struct PendingRelease {
    int synapseIndex;
    double arrivalTime;
    double amount;
};

using DeliveryCallback = std::function<void(int, int, int, double)>;

class SynapsePopulation {
public:
    std::vector<Synapse> synapses;
    std::vector<PendingRelease> pending;

    // ===== 添加突触 =====
    int addSynapse(const Synapse& syn) {
        synapses.push_back(syn);
        return (int)synapses.size() - 1;
    }

    // ===== 突触前放电通知 =====
    void onPresynapticSpike(int preId, double t) {
        for (size_t i = 0; i < synapses.size(); ++i) {
            if (synapses[i].preId != preId) continue;

            double amount = synapses[i].release(true);
            if (amount <= 0.0) continue;

            PendingRelease pr;
            pr.synapseIndex = (int)i;
            pr.arrivalTime = t + synapses[i].delayMs;
            pr.amount = amount;
            pending.push_back(pr);
        }
    }

    using PostVoltageCallback = std::function<double(int)>;

    void step(double t, double dt,
              const DeliveryCallback& deliver,
              const PostVoltageCallback& getPostV) {
        // 1. 衰减所有突触电流
        for (auto& s : synapses) {
            s.decayCurrents(dt);
        }

        // 2. 处理延迟队列
        for (auto it = pending.begin(); it != pending.end(); ) {
            if (t >= it->arrivalTime) {
                synapses[it->synapseIndex].trigger(it->amount);
                it = pending.erase(it);
            } else {
                ++it;
            }
        }

        // 3. 投递当前电流（NMDA 带 Mg 阻断）
        for (auto& s : synapses) {
            double vPost = getPostV(s.postId);

            // Mg 阻断
            double mg_block = 1.0 / (1.0 + std::exp(-(vPost + 25.0) / 12.5));

            // AMPA 不受阻断，NMDA 受阻断
            double I = s.I_ampa + s.I_nmda * mg_block;

            if (std::abs(I) > 1e-9) {
                deliver(s.postId, s.postBranch, s.postInput, I);
            }
        }
    }
    int size() const { return (int)synapses.size(); }
    int pendingCount() const { return (int)pending.size(); }

    void clear() {
        synapses.clear();
        pending.clear();
    }
};

#endif
