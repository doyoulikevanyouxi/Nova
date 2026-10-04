#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include <iostream>
#include <iomanip>
#include <vector>

void stimulateRegion(Network& net, const std::string& name, double amount) {
    auto* region = net.getRegion(name);
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

void printRegionActivity(Network& net, const char* name,
                         const std::vector<int>& ids) {
    int spikes = 0;
    double rate = 0.0;
    for (int id : ids) {
        if (net.neurons[id].hasSpiked) spikes++;
        rate += net.neurons[id].firingRate;
    }
    double avgRate = ids.empty() ? 0.0 : rate / ids.size();
    std::cout << "  " << std::setw(6) << name
              << " spikes=" << std::setw(3) << spikes
              << "  avgRate=" << std::fixed << std::setprecision(4)
              << avgRate << " (" << (avgRate * 1000.0) << " Hz)\n";
}

int main() {
    std::cout << "=== Action System Test ===\n\n";

    Network net;
    net.buildFromGenome(defaultVisualGenome());

    std::cout << "Action system regions:\n";
    std::cout << "  PFC:  " << net.pfcNeurons.size() << " P\n";
    std::cout << "  BG:   " << net.bgNeurons.size() << " P\n";
    std::cout << "  THAL: " << net.thalNeurons.size() << " P\n";
    std::cout << "  M1:   " << net.m1Neurons.size() << " P\n\n";

    // 关闭自发（让基线干净）
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    const double dt = 0.05;

    // ===== 测试 1：基线 =====
    std::cout << "=== Test 1: Baseline ===\n\n";
    net.reset();
    net.initS1();
    net.initActionSystem();
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    for (int step = 0; step < (int)(200.0 / dt); ++step) {
        net.step(dt);
    }

    std::cout << "  Baseline actionOutput: "
              << std::fixed << std::setprecision(4)
              << net.getActionOutput() << "\n";
    std::cout << "  Baseline PFC attention: "
              << net.getPFCAttention() << "\n\n";

    // ===== 测试 2：刺激 IT（视觉驱动行动）=====
    std::cout << "=== Test 2: Stimulate IT ===\n\n";
    net.reset();
    net.initS1();
    net.initActionSystem();
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    // 刺激 IT 30ms
    for (int step = 0; step < (int)(30.0 / dt); ++step) {
        stimulateRegion(net, "IT", 50.0);
        net.step(dt);
    }
    // 之后 300ms
    for (int step = 0; step < (int)(300.0 / dt); ++step) {
        net.step(dt);
    }

    printRegionActivity(net, "PFC", net.pfcNeurons);
    printRegionActivity(net, "BG", net.bgNeurons);
    printRegionActivity(net, "THAL", net.thalNeurons);
    printRegionActivity(net, "M1", net.m1Neurons);
    std::cout << "  actionOutput: " << net.getActionOutput() << "\n";
    std::cout << "  PFC attention: " << net.getPFCAttention() << "\n\n";

    // ===== 测试 3：刺激 S1_2（痛觉驱动行动）=====
    std::cout << "=== Test 3: Stimulate S1_2 ===\n\n";
    net.reset();
    net.initS1();
    net.initActionSystem();
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    // 刺激 S1_2 30ms
    for (int step = 0; step < (int)(30.0 / dt); ++step) {
        stimulateRegion(net, "S1_2", 50.0);
        net.step(dt);
    }
    for (int step = 0; step < (int)(300.0 / dt); ++step) {
        net.step(dt);
    }

    printRegionActivity(net, "PFC", net.pfcNeurons);
    printRegionActivity(net, "BG", net.bgNeurons);
    printRegionActivity(net, "THAL", net.thalNeurons);
    printRegionActivity(net, "M1", net.m1Neurons);
    std::cout << "  actionOutput: " << net.getActionOutput() << "\n";
    std::cout << "  PFC attention: " << net.getPFCAttention() << "\n\n";

    // ===== 判定 =====
    std::cout << "=== Criteria ===\n\n";
    bool pass = true;

    double baselineAction = 0.0;
    double itAction = 0.0;
    double s1Action = 0.0;

    // 重新跑并记录
    {
        net.reset();
        net.initS1();
        net.initActionSystem();
        for (auto& n : net.neurons) {
            n.soma.I_pace = 0.0;
            n.soma.sigma_noise = 0.0;
        }
        for (int step = 0; step < (int)(200.0 / dt); ++step) net.step(dt);
        baselineAction = net.getActionOutput();
    }
    {
        net.reset();
        net.initS1();
        net.initActionSystem();
        for (auto& n : net.neurons) {
            n.soma.I_pace = 0.0;
            n.soma.sigma_noise = 0.0;
        }
        for (int step = 0; step < (int)(30.0 / dt); ++step) {
            stimulateRegion(net, "IT", 50.0);
            net.step(dt);
        }
        for (int step = 0; step < (int)(300.0 / dt); ++step) net.step(dt);
        itAction = net.getActionOutput();
    }
    {
        net.reset();
        net.initS1();
        net.initActionSystem();
        for (auto& n : net.neurons) {
            n.soma.I_pace = 0.0;
            n.soma.sigma_noise = 0.0;
        }
        for (int step = 0; step < (int)(30.0 / dt); ++step) {
            stimulateRegion(net, "S1_2", 50.0);
            net.step(dt);
        }
        for (int step = 0; step < (int)(300.0 / dt); ++step) net.step(dt);
        s1Action = net.getActionOutput();
    }

    std::cout << "  Baseline action: " << std::fixed << std::setprecision(4)
              << baselineAction << "\n";
    std::cout << "  IT-driven:       " << itAction << "\n";
    std::cout << "  S1_2-driven:     " << s1Action << "\n\n";

    if (std::abs(baselineAction) < 0.2) {
        std::cout << "[PASS] Baseline action near zero\n";
    } else {
        std::cout << "[FAIL] Baseline action too large\n";
        pass = false;
    }

    if (itAction > baselineAction + 0.05) {
        std::cout << "[PASS] IT drives action ("
                  << baselineAction << " -> " << itAction << ")\n";
    } else {
        std::cout << "[FAIL] IT does not drive action\n";
        pass = false;
    }

    if (s1Action > baselineAction + 0.05) {
        std::cout << "[PASS] S1_2 drives action ("
                  << baselineAction << " -> " << s1Action << ")\n";
    } else {
        std::cout << "[FAIL] S1_2 does not drive action\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "ACTION SYSTEM WORKS"
                                   : "ACTION SYSTEM FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
