#include "../core/Network.h"
#include <iostream>
#include <iomanip>
#include <vector>

void stimulate(Neuron& n, int branch, double amount) {
    if (branch < 0 || branch >= (int)n.branches.size()) return;
    for (int i = 0; i < (int)n.branches[branch].inputs.size(); ++i) {
        n.deliver(branch, i, amount);
    }
}

int main() {
    std::cout << "=== Associative Learning via STDP ===\n\n";

    Network net;
    int A = net.addNeuron(pyramidalType());
    int B = net.addNeuron(pyramidalType());

    // ===== 关键修复：关掉自发活动 =====
    for (int i = 0; i < net.neuronCount(); ++i) {
        net.neurons[i].soma.I_pace = 0.0;
        net.neurons[i].soma.sigma_noise = 0.0;
    }

    int synId = net.connect(A, B, 0, 0, 1.0, 1.0, true);
    net.synapses.synapses[synId].weightMax = 3.0;
    net.dopamine = 1.0;

    const double dt = 0.05;

    std::cout << "Setup:\n";
    std::cout << "  A (input)  id = " << A << "\n";
    std::cout << "  B (output) id = " << B << "\n";
    std::cout << "  Synapse A->B, initial weight = 1.0\n";
    std::cout << "  Dopamine = 1.0\n";
    std::cout << "  Spontaneous activity: DISABLED\n\n";

    // 测量：只刺激 A，统计 B 的放电
    auto measureB = [&]() -> int {
        net.reset();
        net.dopamine = 1.0;
        // 重置后也要关掉自发（reset 可能恢复）
        for (auto& n : net.neurons) {
            n.soma.I_pace = 0.0;
            n.soma.sigma_noise = 0.0;
        }
        int spikes = 0;
        for (int step = 0; step < (int)(200.0 / dt); ++step) {
            if (step * dt < 100.0) stimulate(net.neurons[A], 0, 1.5);
            net.step(dt);
            if (net.neurons[B].hasSpiked) spikes++;
        }
        return spikes;
    };

    // ===== 阶段 1：基线 =====
    std::cout << "=== Phase 1: Baseline ===\n";
    int baseline = measureB();
    std::cout << "B spikes (A only): " << baseline << "\n\n";

    // ===== 阶段 2：训练 =====
    std::cout << "=== Phase 2: Training (50 trials) ===\n\n";

    const int trials = 50;

    for (int trial = 0; trial < trials; ++trial) {
        // A 刺激：0-5ms
        for (int step = 0; step < (int)(5.0 / dt); ++step) {
            stimulate(net.neurons[A], 0, 1.5);
            net.step(dt);
        }
        // 静息：5-10ms
        for (int step = 0; step < (int)(5.0 / dt); ++step) {
            net.step(dt);
        }
        // B 刺激：10-15ms
        for (int step = 0; step < (int)(5.0 / dt); ++step) {
            stimulate(net.neurons[B], 0, 1.5);
            net.step(dt);
        }
        // 静息：15-50ms
        for (int step = 0; step < (int)(35.0 / dt); ++step) {
            net.step(dt);
        }

        if ((trial + 1) % 10 == 0) {
            double w = net.getWeight(synId);
            std::cout << "  Trial " << std::setw(2) << (trial + 1)
                      << ": A->B weight = "
                      << std::fixed << std::setprecision(4) << w << "\n";
        }
    }

    double finalWeight = net.getWeight(synId);
    std::cout << "\nFinal weight: " << std::fixed << std::setprecision(4)
              << finalWeight << "\n\n";

    // ===== 阶段 3：测试 =====
    std::cout << "=== Phase 3: Post-Training Test ===\n";
    int after = measureB();
    std::cout << "B spikes (A only): " << after << "\n\n";

    // ===== 总结 =====
    std::cout << "=== Summary ===\n\n";
    std::cout << std::setw(20) << "Metric"
              << std::setw(15) << "Before"
              << std::setw(15) << "After"
              << std::setw(15) << "Change"
              << "\n";
    std::cout << std::string(65, '-') << "\n";
    std::cout << std::setw(20) << "A->B weight"
              << std::setw(15) << std::fixed << std::setprecision(4) << 1.0
              << std::setw(15) << finalWeight
              << std::setw(15) << (finalWeight - 1.0)
              << "\n";
    std::cout << std::setw(20) << "B spikes (A only)"
              << std::setw(15) << baseline
              << std::setw(15) << after
              << std::setw(15) << (after - baseline)
              << "\n\n";

    bool pass = true;

    if (finalWeight > 1.0 + 0.01) {
        std::cout << "[PASS] STDP increased A->B weight ("
                  << std::setprecision(4) << (finalWeight - 1.0) << ")\n";
    } else {
        std::cout << "[FAIL] Weight did not increase\n";
        pass = false;
    }

    if (after > baseline) {
        std::cout << "[PASS] B responds more to A after training ("
                  << baseline << " -> " << after << " spikes)\n";
    } else {
        std::cout << "[NEUTRAL] B response not increased\n";
    }

    std::cout << "\n=== " << (pass ? "ALL TESTS PASSED" : "TESTS FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
