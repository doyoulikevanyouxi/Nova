#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include <iostream>
#include <iomanip>
#include <vector>

int main() {
    std::cout << "=== Metabolic Pool Test ===\n\n";

    Network net;
    net.buildFromGenome(defaultVisualGenome());

    const double dt = 0.05;

    // ===== 测试 1：无食物，血糖下降 =====
    std::cout << "=== Test 1: Starvation ===\n\n";
    std::cout << std::setw(10) << "Time(s)"
              << std::setw(15) << "Glucose"
              << std::setw(15) << "Hunger"
              << std::setw(15) << "Spikes"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    // 关闭所有自发（让网络安静，只看代谢消耗）
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    net.metabolicPool.glucose = 1.0;

    for (int step = 0; step < (int)(10000.0 / dt); ++step) {
        double t = step * dt;
        net.step(dt);

        if (step % 1000 == 0) {
            int spikes = 0;
            for (auto& n : net.neurons) {
                if (n.hasSpiked) spikes++;
            }
            std::cout << std::setw(10) << std::fixed << std::setprecision(1)
                      << (t / 1000.0)
                      << std::setw(15) << std::setprecision(4)
                      << net.metabolicPool.glucose
                      << std::setw(15) << net.metabolicPool.getHungerLevel()
                      << std::setw(15) << spikes
                      << "\n";
        }
    }

    double glucoseAfterStarve = net.metabolicPool.glucose;

    // ===== 测试 2：喂食后血糖上升 =====
    std::cout << "\n=== Test 2: Feeding ===\n\n";

    double glucoseBeforeFeed = net.metabolicPool.glucose;
    std::cout << "Glucose before feeding: " << std::fixed
              << std::setprecision(4) << glucoseBeforeFeed << "\n";

    // 喂食 5 秒
    std::cout << "Feeding for 5s...\n";
    net.feed(0.002);

    for (int step = 0; step < (int)(5000.0 / dt); ++step) {
        net.step(dt);
    }
    net.clearFood();

    double glucoseAfterFeed = net.metabolicPool.glucose;
    std::cout << "Glucose after feeding:  " << glucoseAfterFeed << "\n\n";

    // ===== 测试 3：自发活动的代谢消耗 =====
    std::cout << "=== Test 3: Metabolic cost of spikes ===\n\n";

    // 场景 A：无自发，跑 5s
    Network netA;
    netA.buildFromGenome(defaultVisualGenome());
    for (auto& n : netA.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }
    netA.metabolicPool.glucose = 1.0;

    for (int step = 0; step < (int)(5000.0 / dt); ++step) {
        netA.step(dt);
    }
    double glucoseQuiet = netA.metabolicPool.glucose;

    // 场景 B：有自发，跑 5s
    Network netB;
    netB.buildFromGenome(defaultVisualGenome());
    for (auto& n : netB.neurons) {
        n.soma.I_pace = 3.0;
        n.soma.sigma_noise = 0.03;
    }
    netB.metabolicPool.glucose = 1.0;

    for (int step = 0; step < (int)(5000.0 / dt); ++step) {
        netB.step(dt);
    }
    double glucoseSpikes = netB.metabolicPool.glucose;

    std::cout << "5s quiet:    " << std::fixed << std::setprecision(4)
              << glucoseQuiet << " (consumed "
              << (1.0 - glucoseQuiet) << ")\n";
    std::cout << "5s active:   " << glucoseSpikes << " (consumed "
              << (1.0 - glucoseSpikes) << ")\n";
    std::cout << "Extra cost:  "
              << ((1.0 - glucoseSpikes) - (1.0 - glucoseQuiet)) << "\n\n";

    // ===== 判定 =====
    std::cout << "=== Criteria ===\n\n";
    bool pass = true;

    if (glucoseAfterStarve < 1.0) {
        std::cout << "[PASS] Glucose decreased without food ("
                  << std::fixed << std::setprecision(4)
                  << glucoseAfterStarve << ")\n";
    } else {
        std::cout << "[FAIL] Glucose did not decrease\n";
        pass = false;
    }

    if (glucoseAfterFeed > glucoseBeforeFeed) {
        std::cout << "[PASS] Feeding restored glucose ("
                  << glucoseBeforeFeed << " -> " << glucoseAfterFeed << ")\n";
    } else {
        std::cout << "[FAIL] Feeding did not restore glucose\n";
        pass = false;
    }

    // 判定
    if (glucoseSpikes < glucoseQuiet) {
        std::cout << "[PASS] Spontaneous activity costs more glucose\n";
    } else {
        std::cout << "[FAIL] Spike cost not visible\n";
        pass = false;
    }
    std::cout << "\n=== " << (pass ? "METABOLISM WORKS" : "METABOLISM FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
