#include "../core/Network.h"
#include <iostream>
#include <iomanip>
#include <vector>

int main() {
    std::cout << "=== Network Test ===\n";

    // ===== 创建网络 =====
    Network net;

    // 4 个神经元
    int pyr0 = net.addNeuron(pyramidalType());
    int bas1 = net.addNeuron(basketType());
    int pyr2 = net.addNeuron(pyramidalType());
    int chn3 = net.addNeuron(chandelierType());

    std::cout << "\nNeurons created: " << net.neuronCount() << "\n";
    std::cout << "  Pyramidal-0: id=" << pyr0 << "\n";
    std::cout << "  Basket-1:    id=" << bas1 << "\n";
    std::cout << "  Pyramidal-2: id=" << pyr2 << "\n";
    std::cout << "  Chandelier-3:id=" << chn3 << "\n";

    // ===== 连接 =====
    net.connect(pyr0, bas1, 0, 0, 0.15, 1.0, true);
    net.connect(pyr0, pyr2, 0, 0, 0.10, 1.0, true);
    net.connect(pyr0, chn3, 0, 0, 0.15, 1.0, true);
    net.connect(bas1, pyr0, 0, 0, 0.25, 1.5, false);
    net.connect(bas1, pyr2, 0, 0, 0.25, 1.5, false);
    net.connect(chn3, pyr0, 0, 0, 0.20, 2.0, false);
    net.connect(chn3, pyr2, 0, 0, 0.20, 2.0, false);
    std::cout << "\nSynapses: " << net.synapseCount() << "\n";

    // ===== 测试 1：静息自发 =====
    std::cout << "\n=== Test 1: Spontaneous Activity (2 seconds) ===\n\n";
    std::cout << std::setw(6) << "time"
              << std::setw(15) << "pyr0"
              << std::setw(15) << "bas1"
              << std::setw(15) << "pyr2"
              << std::setw(15) << "chn3"
              << "\n";
    std::cout << std::string(70, '-') << "\n";

    const double dt = 0.05;
    std::vector<int> spikeCounts(4, 0);

    for (int step = 0; step < (int)(2000.0 / dt); ++step) {
        net.step(dt);

        for (int i = 0; i < 4; ++i) {
            if (net.neurons[i].hasSpiked) spikeCounts[i]++;
        }

        // 每 500ms 打印一次
        if (step % (int)(500.0 / dt) == 0 && step > 0) {
            std::cout << std::setw(6) << std::fixed << std::setprecision(0)
            << (step * dt)
            << std::setw(15) << std::setprecision(2)
            << net.neurons[0].soma.v
            << std::setw(15) << net.neurons[1].soma.v
            << std::setw(15) << net.neurons[2].soma.v
            << std::setw(15) << net.neurons[3].soma.v
            << "\n";
        }
    }

    std::cout << "\nSpike counts:\n";
    const char* names[4] = {"Pyr0", "Bas1", "Pyr2", "Chn3"};
    for (int i = 0; i < 4; ++i) {
        std::cout << "  " << names[i] << ": " << spikeCounts[i]
                  << " spikes (" << std::setprecision(2)
                  << (spikeCounts[i] / 2.0) << " Hz)\n";
    }

    // ===== 测试 2：刺激 Pyramidal-0 =====
    // ===== 测试 2：刺激 Pyramidal-0 =====
    std::cout << "\n=== Test 2: Stimulate Pyramidal-0 (500ms) ===\n\n";

    net.reset();
    std::fill(spikeCounts.begin(), spikeCounts.end(), 0);

    // 计算每个神经元的放电率（分早/晚两段）
    std::vector<int> earlyCounts(4, 0);
    std::vector<int> lateCounts(4, 0);
    const int stimSteps = (int)(500.0 / dt);

    for (int step = 0; step < stimSteps; ++step) {
        // 每 10ms 给 Pyr0 所有分支所有通道一个强输入
        if (step % (int)(10.0 / dt) == 0) {
            Neuron& target = net.neurons[0];
            for (size_t b = 0; b < target.branches.size(); ++b) {
                for (size_t i = 0; i < target.branches[b].inputs.size(); ++i) {
                    target.deliver((int)b, (int)i, 0.5);
                }
            }
        }

        net.step(dt);

        for (int i = 0; i < 4; ++i) {
            if (net.neurons[i].hasSpiked) {
                spikeCounts[i]++;
                if (step < stimSteps / 2) earlyCounts[i]++;
                else lateCounts[i]++;
            }
        }
    }

    std::cout << "Spike counts after stimulation:\n";
    std::cout << std::setw(12) << "neuron"
              << std::setw(15) << "total"
              << std::setw(15) << "early Hz"
              << std::setw(15) << "late Hz"
              << "\n";
    std::cout << std::string(57, '-') << "\n";

    for (int i = 0; i < 4; ++i) {
        double earlyHz = earlyCounts[i] / 0.25;   // 前 250ms
        double lateHz = lateCounts[i] / 0.25;     // 后 250ms
        std::cout << std::setw(12) << names[i]
                  << std::setw(15) << spikeCounts[i]
                  << std::setw(15) << std::fixed << std::setprecision(2)
                  << earlyHz
                  << std::setw(15) << lateHz
                  << "\n";
    }

    std::cout << "\nExpected:\n";
    std::cout << "  Pyr0 driven by strong input -> high firing rate\n";
    std::cout << "  Bas1 and Chn3 receive Pyr0 -> fire\n";
    std::cout << "  Pyr2 receives excitation + strong inhibition\n";
    std::cout << "  Late Hz should be lower than Early Hz (adaptation)\n";

    std::cout << "\n=== All Tests Complete ===\n";
    return 0;
}
