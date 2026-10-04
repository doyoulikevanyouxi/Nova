#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include <iostream>
#include <iomanip>
#include <vector>

int main() {
    std::cout << "=== Valence System Test ===\n\n";

    Network net;
    net.buildFromGenome(defaultVisualGenome());

    // 关闭所有自发（让代谢只受基础消耗影响）
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    const double dt = 0.05;

    // ===== 测试 1：不同血糖水平的效价 =====
    std::cout << "=== Test 1: Valence vs glucose ===\n\n";
    std::cout << std::setw(12) << "Glucose"
              << std::setw(12) << "Valence"
              << std::setw(15) << "DA Mod"
              << std::setw(15) << "MemWrite"
              << std::setw(15) << "ForgetRate"
              << "\n";
    std::cout << std::string(70, '-') << "\n";

    std::vector<double> glucoseLevels = {0.0, 0.2, 0.5, 0.8, 1.0};

    for (double g : glucoseLevels) {
        ValenceSystem vs;
        vs.integrator = (g - 0.5) * 2.0;
        vs.valence = vs.integrator;
        vs.valence = std::max(-1.0, std::min(1.0, vs.valence));

        std::cout << std::setw(12) << std::fixed << std::setprecision(2)
                  << g
                  << std::setw(12) << vs.valence
                  << std::setw(15) << vs.getDopamineModulation()
                  << std::setw(15) << vs.getMemoryWriteStrength()
                  << std::setw(15) << vs.getForgettingRate()
                  << "\n";
    }

    // ===== 测试 2：效价随时间变化 =====
    std::cout << "\n=== Test 2: Valence evolution ===\n\n";
    std::cout << std::setw(10) << "Time(s)"
              << std::setw(15) << "Glucose"
              << std::setw(15) << "Valence"
              << std::setw(15) << "State"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    net.metabolicPool.glucose = 1.0;
    net.valenceSystem.integrator = 1.0;
    net.valenceSystem.valence = 1.0;

    for (int step = 0; step < (int)(10000.0 / dt); ++step) {
        double t = step * dt;
        net.step(dt);

        if (step % 1000 == 0) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(1)
            << (t / 1000.0)
            << std::setw(15) << std::setprecision(4)
            << net.metabolicPool.glucose
            << std::setw(15) << std::setprecision(4)
            << net.valenceSystem.valence
            << std::setw(15) << net.valenceSystem.state()
            << "\n";
        }
    }

    // ===== 测试 3：DA 跟随效价 =====
    std::cout << "\n=== Test 3: DA driven by valence ===\n\n";

    Network net2;
    net2.buildFromGenome(defaultVisualGenome());
    for (auto& n : net2.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }
    net2.daFromValence = true;

    std::cout << std::setw(15) << "Glucose"
              << std::setw(15) << "Valence"
              << std::setw(15) << "DA"
              << "\n";
    std::cout << std::string(45, '-') << "\n";

    for (double g : {0.0, 0.3, 0.5, 0.7, 1.0}) {
        net2.metabolicPool.glucose = g;
        net2.valenceSystem.integrator = (g - 0.5) * 2.0;
        net2.valenceSystem.valence = net2.valenceSystem.integrator;

        // 跑一步更新 DA
        net2.step(dt);

        std::cout << std::setw(15) << std::fixed << std::setprecision(2)
                  << g
                  << std::setw(15) << net2.valenceSystem.valence
                  << std::setw(15) << net2.dopamine
                  << "\n";
    }

    // ===== 判定 =====
    std::cout << "\n=== Criteria ===\n\n";
    bool pass = true;

    // 检查效价映射
    ValenceSystem vsHigh, vsLow;
    vsHigh.integrator = 1.0; vsHigh.valence = 1.0;
    vsLow.integrator = -1.0; vsLow.valence = -1.0;

    if (vsHigh.getDopamineModulation() > vsLow.getDopamineModulation()) {
        std::cout << "[PASS] High valence -> high DA ("
                  << vsHigh.getDopamineModulation() << " vs "
                  << vsLow.getDopamineModulation() << ")\n";
    } else {
        std::cout << "[FAIL] Valence-DA mapping wrong\n";
        pass = false;
    }

    if (vsHigh.getMemoryWriteStrength() > 0 &&
        vsLow.getMemoryWriteStrength() == 0) {
        std::cout << "[PASS] Memory write gated by positive valence\n";
    } else {
        std::cout << "[FAIL] Memory write gating wrong\n";
        pass = false;
    }

    if (vsLow.getForgettingRate() > 0 &&
        vsHigh.getForgettingRate() == 0) {
        std::cout << "[PASS] Negative valence accelerates forgetting\n";
    } else {
        std::cout << "[FAIL] Forgetting rate wrong\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "VALENCE WORKS" : "VALENCE FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
