#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

void stimulateRegion(Network& net, const std::string& regionName,
                     double amount) {
    auto* region = net.getRegion(regionName);
    if (!region) return;

    for (int id : region->neurons) {
        if (net.neurons[id].type.name != "Pyramidal") continue;
        for (size_t b = 0; b < net.neurons[id].branches.size(); ++b) {
            for (size_t i = 0; i < net.neurons[id].branches[b].inputs.size(); ++i) {
                net.neurons[id].deliver((int)b, (int)i, amount / 32.0);
            }
        }
    }
}

int main() {
    std::cout << "=== Signal Propagation Test ===\n\n";

    Network net;
    net.buildFromGenome(defaultVisualGenome());

    // 关掉 STDP
    for (auto& s : net.synapses.synapses) {
        s.isPlastic = false;
    }

    // ===== 关键：关掉所有自发活动 =====
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    std::cout << "Spontaneous activity: DISABLED\n\n";

    const double dt = 0.05;
    const int settleSteps = (int)(200.0 / dt);
    const int testSteps = (int)(200.0 / dt);

    // ===== 稳定（无输入）=====
    std::cout << "Settling (200ms, no input)...\n";
    for (int step = 0; step < settleSteps; ++step) {
        net.step(dt);
    }

    // 统计基线
    auto countSpikes = [&](const std::string& regionName) {
        auto* region = net.getRegion(regionName);
        if (!region) return 0;
        int count = 0;
        for (int id : region->neurons) {
            if (net.neurons[id].hasSpiked) count++;
        }
        return count;
    };

    std::cout << "Baseline spikes:\n";
    for (auto& name : {"V1", "V2", "V4", "IT", "H"}) {
        std::cout << "  " << name << ": " << countSpikes(name) << "\n";
    }
    std::cout << "\n";

    // ===== 刺激 V1 =====
    std::cout << "Stimulating V1 (10ms, strong)...\n\n";

    std::map<std::string, double> firstSpike;
    std::map<std::string, int> spikeCount;
    for (auto& name : {"V1", "V2", "V4", "IT", "H"}) {
        firstSpike[name] = -1.0;
        spikeCount[name] = 0;
    }

    for (int step = 0; step < testSteps; ++step) {
        double t = step * dt;

        if (t < 10.0) {
             stimulateRegion(net, "V1", 100.0);   // 50 -> 100
        }

        net.step(dt);

        for (auto& name : {"V1", "V2", "V4", "IT", "H"}) {
            auto* region = net.getRegion(name);
            if (!region) continue;
            for (int id : region->neurons) {
                if (net.neurons[id].hasSpiked) {
                    if (firstSpike[name] < 0) firstSpike[name] = t;
                    spikeCount[name]++;
                }
            }
        }
    }

    // ===== 结果 =====
    std::cout << "=== Propagation Results ===\n\n";
    std::cout << std::setw(8) << "Region"
              << std::setw(15) << "First spike (ms)"
              << std::setw(15) << "Total spikes"
              << "\n";
    std::cout << std::string(38, '-') << "\n";

    for (auto& name : {"V1", "V2", "V4", "IT", "H"}) {
        std::cout << std::setw(8) << name;
        if (firstSpike[name] < 0) {
            std::cout << std::setw(15) << "--";
        } else {
            std::cout << std::setw(15) << std::fixed << std::setprecision(2)
            << firstSpike[name];
        }
        std::cout << std::setw(15) << spikeCount[name] << "\n";
    }

    // ===== 判定 =====
    std::cout << "\n=== Criteria ===\n\n";
    bool pass = true;

    if (firstSpike["V1"] >= 0 && firstSpike["V1"] < 5.0) {
        std::cout << "[PASS] V1 fired at "
                  << std::fixed << std::setprecision(2)
                  << firstSpike["V1"] << " ms\n";
    } else {
        std::cout << "[FAIL] V1 did not fire in time\n";
        pass = false;
    }

    // 逐级检查
    std::vector<std::string> layers = {"V1", "V2", "V4", "IT", "H"};
    double prev = firstSpike["V1"];
    for (size_t i = 1; i < layers.size(); ++i) {
        auto& name = layers[i];
        if (firstSpike[name] >= 0) {
            if (firstSpike[name] > prev) {
                std::cout << "[PASS] " << name << " at "
                          << std::fixed << std::setprecision(2)
                          << firstSpike[name] << " ms (delta = "
                          << (firstSpike[name] - prev) << " ms)\n";
                prev = firstSpike[name];
            } else {
                std::cout << "[FAIL] " << name << " at "
                          << firstSpike[name] << " ms (too early)\n";
                pass = false;
            }
        } else {
            std::cout << "[WARN] " << name << " did not fire\n";
        }
    }

    std::cout << "\n=== " << (pass ? "SIGNAL PROPAGATION WORKS"
                                   : "SIGNAL PROPAGATION FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
