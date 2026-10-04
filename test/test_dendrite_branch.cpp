#include "../core/DendriteBranch.h"
#include <iostream>
#include <iomanip>
#include <vector>

// ============================================================
// 测试 1：基本泄漏积分
// ============================================================
void testBasicIntegration() {
    std::cout << "\n=== Test 1: Basic Integration ===\n\n";

    DendriteBranch branch(2);
    branch.R_input = 50.0;
    branch.weights[0] = 1.0;
    branch.weights[1] = 1.0;

    const double dt = 0.1;
    const int totalSteps = 500;

    std::cout << std::setw(10) << "time(ms)"
              << std::setw(15) << "d(mV)"
              << std::setw(15) << "p(mV)"
              << "\n";
    std::cout << std::string(40, '-') << "\n";

    for (int step = 0; step < totalSteps; ++step) {
        double t = step * dt;

        // 输入在 t=0 到 t=10ms 之间持续
        if (t < 10.0) {
            branch.inputs[0] = 1.0;
            branch.inputs[1] = 1.0;
        }

        branch.update(dt);

        if (step % 50 == 0) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(1)
            << t
            << std::setw(15) << std::setprecision(3) << branch.d
            << std::setw(15) << branch.p
            << "\n";
        }
    }

    std::cout << "\nExpected:\n";
    std::cout << "  d rises during input (0-10ms)\n";
    std::cout << "  d decays back to -65mV after input stops\n";
    std::cout << "  p rises during input, decays slowly\n";
}

// ============================================================
// 测试 2：不同距离的信号衰减
// ============================================================
void testAttenuation() {
    std::cout << "\n=== Test 2: Attenuation vs Distance ===\n\n";

    std::vector<double> distances = {1.0, 2.0, 5.0, 10.0};

    std::cout << std::setw(12) << "distance"
              << std::setw(12) << "lambda"
              << std::setw(15) << "attenuation"
              << "\n";
    std::cout << std::string(40, '-') << "\n";

    for (double dist : distances) {
        DendriteBranch branch(1);
        branch.setPosition(dist, 0.0);
        branch.diameter = 1.0;

        std::cout << std::setw(12) << std::fixed << std::setprecision(1)
                  << dist
                  << std::setw(12) << std::setprecision(3)
                  << branch.spaceConstant()
                  << std::setw(15) << branch.attenuationToSoma()
                  << "\n";
    }

    std::cout << "\nExpected:\n";
    std::cout << "  Closer branches attenuate less\n";
    std::cout << "  Farther branches attenuate more\n";
}

// ============================================================
// 测试 3：不同分支的输入增益
// ============================================================
void testDifferentInputGains() {
    std::cout << "\n=== Test 3: Different Input Gains ===\n\n";

    std::vector<double> gains = {10.0, 30.0, 50.0, 100.0};

    const double dt = 0.1;
    const double duration = 50.0;
    const int totalSteps = (int)(duration / dt);

    std::cout << std::setw(12) << "R_input"
              << std::setw(15) << "peak d(mV)"
              << std::setw(15) << "final d(mV)"
              << "\n";
    std::cout << std::string(45, '-') << "\n";

    for (double gain : gains) {
        DendriteBranch branch(1);
        branch.R_input = gain;
        branch.weights[0] = 1.0;

        double peakD = -100.0;

        for (int step = 0; step < totalSteps; ++step) {
            branch.inputs[0] = 1.0;
            branch.update(dt);
            if (branch.d > peakD) peakD = branch.d;
        }

        std::cout << std::setw(12) << std::fixed << std::setprecision(1)
                  << gain
                  << std::setw(15) << std::setprecision(3) << peakD
                  << std::setw(15) << branch.d
                  << "\n";
    }

    std::cout << "\nExpected:\n";
    std::cout << "  Higher R_input -> larger depolarization\n";
}

// ============================================================
// 测试 4：平台电位形成与衰减
// ============================================================
void testPlateauPotential() {
    std::cout << "\n=== Test 4: Plateau Potential ===\n\n";

    DendriteBranch branch(1);
    branch.R_input = 100.0;   // 高增益，容易触发平台
    branch.weights[0] = 1.0;
    branch.theta_plateau = -60.0;

    const double dt = 0.1;
    const double inputDuration = 20.0;   // ms
    const double totalDuration = 200.0;  // ms
    const int totalSteps = (int)(totalDuration / dt);

    std::cout << std::setw(10) << "time(ms)"
              << std::setw(15) << "d(mV)"
              << std::setw(15) << "p(mV)"
              << "\n";
    std::cout << std::string(40, '-') << "\n";

    for (int step = 0; step < totalSteps; ++step) {
        double t = step * dt;

        // 输入只在 0-20ms 之间
        if (t < inputDuration) {
            branch.inputs[0] = 1.0;
        }

        branch.update(dt);

        if (step % 100 == 0 || step == (int)(inputDuration / dt)) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(1)
            << t
            << std::setw(15) << std::setprecision(3) << branch.d
            << std::setw(15) << branch.p
            << "\n";
        }
    }

    std::cout << "\nExpected:\n";
    std::cout << "  d rises during input, falls after\n";
    std::cout << "  p rises during input, decays slowly after\n";
    std::cout << "  p is the branch's short-term memory\n";
}

int main() {
    std::cout << "=== Dendrite Branch Tests ===\n";

    testBasicIntegration();
    testAttenuation();
    testDifferentInputGains();
    testPlateauPotential();

    std::cout << "\n=== All Tests Complete ===\n";
    return 0;
}
