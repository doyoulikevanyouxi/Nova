#include "../core/Neuron.h"
#include <iostream>
#include <iomanip>
#include <vector>

// ============================================================
// 测试 1：结构验证
// ============================================================
void testStructure() {
    std::cout << "\n=== Test 1: Structure ===\n\n";

    Neuron pyr(0, pyramidalType());
    std::cout << "Pyramidal neuron:\n";
    std::cout << "  Branches:      " << pyr.branchCount() << "\n";
    std::cout << "  Clusters:      " << pyr.clusterCount() << "\n";
    std::cout << "  Input channels:" << pyr.totalInputChannels() << "\n";
    for (auto& c : pyr.clusters) {
        std::cout << "  Cluster " << c.id
                  << " (" << (c.type == ClusterType::Basal ? "Basal" : "Apical") << ")"
                  << ": " << c.branchIndices.size() << " branches"
                  << ", pos=(" << c.x << "," << c.y << ")"
                  << ", attToSoma=" << std::fixed << std::setprecision(3)
                  << c.attenuationToSoma() << "\n";
    }

    Neuron bas(1, basketType());
    std::cout << "\nBasket neuron:\n";
    std::cout << "  Branches:      " << bas.branchCount() << "\n";
    std::cout << "  Clusters:      " << bas.clusterCount() << "\n";
    std::cout << "  Input channels:" << bas.totalInputChannels() << "\n";

    Neuron chn(2, chandelierType());
    std::cout << "\nChandelier neuron:\n";
    std::cout << "  Branches:      " << chn.branchCount() << "\n";
    std::cout << "  Clusters:      " << chn.clusterCount() << "\n";
    std::cout << "  Input channels:" << chn.totalInputChannels() << "\n";
}

// ============================================================
// 测试 2：自发活动（无输入）
// ============================================================
void testSpontaneous(const char* name, NeuronType type,
                     double duration_ms) {
    std::cout << "\n=== Test 2: " << name << " Spontaneous ===\n\n";

    Neuron n(0, type);

    const double dt = 0.05;
    const int totalSteps = (int)(duration_ms / dt);

    int spikes = 0;
    double vSum = 0.0;

    // 前 500ms 忽略（稳定期）
    const int settleSteps = (int)(500.0 / dt);

    for (int step = 0; step < totalSteps; ++step) {
        n.step(dt);
        if (n.hasSpiked) spikes++;
        if (step > settleSteps) vSum += n.soma.v;
    }

    double rate = spikes / (duration_ms / 1000.0);
    double vMean = vSum / (totalSteps - settleSteps);

    std::cout << "  Soma V mean: " << std::fixed << std::setprecision(2)
              << vMean << " mV\n";
    std::cout << "  Spikes:      " << spikes << "\n";
    std::cout << "  Rate:        " << std::setprecision(2)
              << rate << " Hz\n";
}

// ============================================================
// 测试 3：突触输入驱动
// ============================================================
void testDrivenFiring() {
    std::cout << "\n=== Test 3: Driven Firing ===\n\n";

    Neuron n(0, pyramidalType());
    const double dt = 0.05;

    // 稳定 500ms
    for (int step = 0; step < (int)(500.0 / dt); ++step) {
        n.step(dt);
    }

    // 持续投递强输入到所有分支
    std::cout << "Delivering sustained input to all branches...\n";
    double vBefore = n.soma.v;

    // 1000ms 持续输入
    int spikes = 0;
    for (int step = 0; step < (int)(1000.0 / dt); ++step) {
        // 每个分支的每个通道都收到 0.5
        for (size_t bi = 0; bi < n.branches.size(); ++bi) {
            for (size_t ii = 0; ii < n.branches[bi].inputs.size(); ++ii) {
                n.deliver((int)bi, (int)ii, 0.5);
            }
        }
        n.step(dt);
        if (n.hasSpiked) spikes++;
    }

    double vAfter = n.soma.v;

    std::cout << "  V before:  " << std::fixed << std::setprecision(2)
              << vBefore << " mV\n";
    std::cout << "  V after:   " << vAfter << " mV\n";
    std::cout << "  Spikes:    " << spikes << "\n";
    std::cout << "  Rate:      " << std::setprecision(2)
              << (spikes / 1.0) << " Hz\n";

    std::cout << "\nExpected:\n";
    std::cout << "  Strong input should depolarize soma\n";
    std::cout << "  AIS should fire at high rate\n";
}

int main() {
    std::cout << "=== Neuron Integration Tests ===\n";

    testStructure();
    testSpontaneous("Pyramidal", pyramidalType(), 5000.0);
    testSpontaneous("Basket", basketType(), 5000.0);
    testSpontaneous("Chandelier", chandelierType(), 5000.0);
    testDrivenFiring();

    std::cout << "\n=== All Tests Complete ===\n";
    return 0;
}
