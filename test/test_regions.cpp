#include "../core/Network.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Region Test ===\n\n";

    Network net;

    // ===== 创建 3 个区域 =====
    net.addRegion("V1",  50.0,  20.0, 12.0, 1);
    net.addRegion("V2",  50.0,  50.0, 12.0, 2);
    net.addRegion("IT",  50.0, 110.0, 18.0, 4);

    // ===== 每个区域加神经元 =====
    for (int i = 0; i < 4; ++i)
        net.addNeuronToRegion("V1", pyramidalType());
    for (int i = 0; i < 2; ++i)
        net.addNeuronToRegion("V1", basketType());

    for (int i = 0; i < 4; ++i)
        net.addNeuronToRegion("V2", pyramidalType());
    for (int i = 0; i < 2; ++i)
        net.addNeuronToRegion("V2", basketType());

    for (int i = 0; i < 6; ++i)
        net.addNeuronToRegion("IT", pyramidalType());
    for (int i = 0; i < 3; ++i)
        net.addNeuronToRegion("IT", basketType());

    // ===== 打印区域信息 =====
    std::cout << std::setw(8) << "Region"
              << std::setw(10) << "Layer"
              << std::setw(10) << "Neurons"
              << std::setw(15) << "Position"
              << "\n";
    std::cout << std::string(43, '-') << "\n";

    for (auto& [name, region] : net.regions) {
        std::cout << std::setw(8) << name
                  << std::setw(10) << region.layer
                  << std::setw(10) << region.size()
                  << std::setw(15)
                  << ("(" + std::to_string((int)region.centerX)
                      + "," + std::to_string((int)region.centerY) + ")")
                  << "\n";
    }

    std::cout << "\nTotal neurons: " << net.neuronCount() << "\n";
    std::cout << "Total regions: " << net.regions.size() << "\n\n";

    // ===== 区域内连接 =====
    // V1 内部：P → B（兴奋）
    auto* v1 = net.getRegion("V1");
    if (v1) {
        for (size_t i = 0; i < v1->neurons.size(); ++i) {
            for (size_t j = 0; j < v1->neurons.size(); ++j) {
                if (i == j) continue;
                int pre = v1->neurons[i];
                int post = v1->neurons[j];
                // 只连 P→B
                if (net.neurons[pre].type.name == "Pyramidal" &&
                    net.neurons[post].type.name == "Basket") {
                    net.connect(pre, post, 0, 0, 0.5, 1.0, true);
                }
            }
        }
    }

    std::cout << "V1 internal synapses: " << net.synapseCount() << "\n\n";

    // ===== 跑 500ms =====
    std::cout << "Running 500ms...\n";
    const double dt = 0.05;
    for (int step = 0; step < (int)(500.0 / dt); ++step) {
        net.step(dt);
    }

    std::cout << "Done.\n\n";

    // ===== 判定 =====
    bool pass = true;

    if (net.regions.size() == 3) {
        std::cout << "[PASS] 3 regions created\n";
    } else {
        std::cout << "[FAIL] Expected 3 regions\n";
        pass = false;
    }

    if (net.neuronCount() == 21) {
        std::cout << "[PASS] 21 neurons created (6+6+9)\n";
    } else {
        std::cout << "[FAIL] Expected 21 neurons, got "
                  << net.neuronCount() << "\n";
        pass = false;
    }

    if (net.synapseCount() > 0) {
        std::cout << "[PASS] Regional synapses created\n";
    } else {
        std::cout << "[FAIL] No synapses\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "REGIONS WORK" : "REGIONS FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
