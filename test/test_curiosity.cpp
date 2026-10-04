#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include "../core/CuriosityInterface.h"
#include <iostream>
#include <iomanip>
#include <vector>

// ============================================================
// 好奇心接口测试
// ============================================================

int main() {
    std::cout << "=== Curiosity Interface Test ===\n\n";

    // ===== 测试 1：三个场景的调质输出 =====
    std::cout << "=== Test 1: Curiosity scenarios ===\n\n";
    std::cout << std::setw(15) << "Scenario"
              << std::setw(12) << "Novelty"
              << std::setw(12) << "Safety"
              << std::setw(12) << "DA boost"
              << std::setw(12) << "5-HT boost"
              << std::setw(12) << "State"
              << "\n";
    std::cout << std::string(75, '-') << "\n";

    auto testScenario = [&](const char* name,
                            double novelty, double safety) {
        CuriosityInterface ci;
        ci.noveltySignal = novelty;
        ci.safetySignal = safety;
        std::cout << std::setw(15) << name
                  << std::setw(12) << std::fixed << std::setprecision(2)
                  << novelty
                  << std::setw(12) << safety
                  << std::setw(12) << std::setprecision(3)
                  << ci.getDopamineBoost()
                  << std::setw(12) << ci.getSerotoninBoost()
                  << std::setw(12) << ci.state()
                  << "\n";
    };

    testScenario("Familiar", 0.1, 1.0);
    testScenario("Curious",  0.9, 1.0);
    testScenario("Fearful",  0.9, 0.1);
    testScenario("Neutral",  0.5, 0.5);
    std::cout << "\n";

    // ===== 测试 2：好奇心驱动 DA =====
    std::cout << "=== Test 2: DA driven by curiosity ===\n\n";

    Network net;
    net.buildFromGenome(defaultVisualGenome());
    net.daFromCuriosity = true;

    std::cout << std::setw(12) << "Novelty"
              << std::setw(12) << "Safety"
              << std::setw(15) << "DA"
              << "\n";
    std::cout << std::string(40, '-') << "\n";

    for (auto& [nov, saf] : std::vector<std::pair<double,double>>{
             {0.0, 1.0}, {0.5, 1.0}, {1.0, 1.0}, {1.0, 0.5}, {1.0, 0.0}
         }) {
        net.dopamine = 0.5;
        net.curiosityInterface.noveltySignal = nov;
        net.curiosityInterface.safetySignal = saf;

        net.step(0.05);

        std::cout << std::setw(12) << std::fixed << std::setprecision(2)
                  << nov
                  << std::setw(12) << saf
                  << std::setw(15) << std::setprecision(3)
                  << net.dopamine
                  << "\n";
    }
    std::cout << "\n";

    // ===== 测试 3：危险驱动 5-HT =====
    std::cout << "=== Test 3: 5-HT driven by danger ===\n\n";

    Network net2;
    net2.buildFromGenome(defaultVisualGenome());
    net2.htFromCuriosity = true;

    std::cout << std::setw(12) << "Safety"
              << std::setw(15) << "5-HT"
              << "\n";
    std::cout << std::string(28, '-') << "\n";

    for (double saf : {0.0, 0.3, 0.5, 0.7, 1.0}) {
        net2.serotonin = 0.5;
        net2.curiosityInterface.safetySignal = saf;
        net2.curiosityInterface.noveltySignal = 0.8;

        net2.step(0.05);

        std::cout << std::setw(12) << std::fixed << std::setprecision(2)
                  << saf
                  << std::setw(15) << std::setprecision(3)
                  << net2.serotonin
                  << "\n";
    }
    std::cout << "\n";

    // ===== 判定 =====
    std::cout << "=== Criteria ===\n\n";
    bool pass = true;

    // 好奇心应该提升 DA
    CuriosityInterface ciHigh, ciLow;
    ciHigh.noveltySignal = 0.9; ciHigh.safetySignal = 1.0;
    ciLow.noveltySignal = 0.1;  ciLow.safetySignal = 1.0;

    if (ciHigh.getDopamineBoost() > ciLow.getDopamineBoost()) {
        std::cout << "[PASS] High novelty increases DA boost ("
                  << ciHigh.getDopamineBoost() << " vs "
                  << ciLow.getDopamineBoost() << ")\n";
    } else {
        std::cout << "[FAIL] Curiosity-DA mapping wrong\n";
        pass = false;
    }

    // 危险应该提升 5-HT
    CuriosityInterface ciSafe, ciDanger;
    ciSafe.noveltySignal = 0.5; ciSafe.safetySignal = 1.0;
    ciDanger.noveltySignal = 0.5; ciDanger.safetySignal = 0.1;

    if (ciDanger.getSerotoninBoost() > ciSafe.getSerotoninBoost()) {
        std::cout << "[PASS] Danger increases 5-HT boost ("
                  << ciDanger.getSerotoninBoost() << " vs "
                  << ciSafe.getSerotoninBoost() << ")\n";
    } else {
        std::cout << "[FAIL] Safety-5HT mapping wrong\n";
        pass = false;
    }

    // 好奇心提升感知增益
    if (ciHigh.getPerceptualGain() > 1.0) {
        std::cout << "[PASS] Curiosity increases perceptual gain ("
                  << ciHigh.getPerceptualGain() << ")\n";
    } else {
        std::cout << "[FAIL] Perceptual gain wrong\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "CURIOSITY WORKS" : "CURIOSITY FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
