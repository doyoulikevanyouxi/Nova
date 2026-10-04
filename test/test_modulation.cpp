#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include "../core/Synapse.h"
#include <iostream>
#include <iomanip>
#include <vector>

// ============================================================
// 调质系统测试
// ============================================================
//
// 验证三个调质标量：
//   dopamine：STDP 门控（已有）
//   acetylcholine：学习率调制（新）
//   serotonin：存在但暂无功能（后面加）
//
// ============================================================

int main() {
    std::cout << "=== Modulation System Test ===\n\n";

    // ===== 测试 1：DA 门控 =====
    std::cout << "=== Test 1: DA gating ===\n\n";
    std::cout << std::setw(10) << "DA"
              << std::setw(15) << "delta_w"
              << "\n";
    std::cout << std::string(25, '-') << "\n";

    for (double da : {0.0, 0.3, 0.5, 0.6, 0.7, 1.0}) {
        Synapse s;
        s.weight = 1.0;
        s.nReleaseSites = 0;
        double w0 = s.weight;
        s.onPreSpike(0.0, da, 0.5);
        s.onPostSpike(5.0, da, 0.5);
        double dw = s.weight - w0;

        std::cout << std::setw(10) << std::fixed << std::setprecision(2)
                  << da
                  << std::setw(15) << std::setprecision(6) << dw
                  << "\n";
    }

    // ===== 测试 2：ACh 调制 =====
    std::cout << "\n=== Test 2: ACh modulation (DA=1.0) ===\n\n";
    std::cout << std::setw(10) << "ACh"
              << std::setw(15) << "delta_w"
              << "\n";
    std::cout << std::string(25, '-') << "\n";

    for (double ach : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        Synapse s;
        s.weight = 1.0;
        s.nReleaseSites = 0;
        double w0 = s.weight;
        s.onPreSpike(0.0, 1.0, ach);
        s.onPostSpike(5.0, 1.0, ach);
        double dw = s.weight - w0;

        std::cout << std::setw(10) << std::fixed << std::setprecision(2)
                  << ach
                  << std::setw(15) << std::setprecision(6) << dw
                  << "\n";
    }
    std::cout << "\nExpected: ACh higher -> larger LTP\n";

    // ===== 测试 3：Network 层调质 =====
    std::cout << "\n=== Test 3: Network modulation levels ===\n\n";

    Network net;
    net.buildFromGenome(defaultVisualGenome());

    std::cout << "Default levels:\n";
    std::cout << "  Dopamine:       " << net.dopamine << "\n";
    std::cout << "  Acetylcholine:  " << net.acetylcholine << "\n";
    std::cout << "  Serotonin:      " << net.serotonin << "\n\n";

    // 修改调质
    net.dopamine = 0.8;
    net.acetylcholine = 0.9;
    net.serotonin = 0.3;

    std::cout << "After manual change:\n";
    std::cout << "  Dopamine:       " << net.dopamine << "\n";
    std::cout << "  Acetylcholine:  " << net.acetylcholine << "\n";
    std::cout << "  Serotonin:      " << net.serotonin << "\n\n";

    // 跑 100ms 确认不崩
    std::cout << "Running 100ms...\n";
    const double dt = 0.05;
    for (int step = 0; step < (int)(100.0 / dt); ++step) {
        net.step(dt);
    }
    std::cout << "Done.\n\n";

    // ===== 判定 =====
    bool pass = true;

    // 检查 ACh 调制方向
    Synapse sLow, sHigh;
    sLow.weight = 1.0;  sLow.nReleaseSites = 0;
    sHigh.weight = 1.0; sHigh.nReleaseSites = 0;

    sLow.onPreSpike(0.0, 1.0, 0.0);
    sLow.onPostSpike(5.0, 1.0, 0.0);
    sHigh.onPreSpike(0.0, 1.0, 1.0);
    sHigh.onPostSpike(5.0, 1.0, 1.0);

    if (sHigh.weight > sLow.weight) {
        std::cout << "[PASS] Higher ACh produces larger LTP ("
                  << std::fixed << std::setprecision(4)
                  << (sHigh.weight - 1.0) << " vs "
                  << (sLow.weight - 1.0) << ")\n";
    } else {
        std::cout << "[FAIL] ACh modulation direction wrong\n";
        pass = false;
    }

    // 检查三个调质都存在于 Network
    if (net.dopamine >= 0.0 && net.acetylcholine >= 0.0 && net.serotonin >= 0.0) {
        std::cout << "[PASS] All three modulators present in Network\n";
    } else {
        std::cout << "[FAIL] Missing modulators\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "MODULATION WORKS" : "MODULATION FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
