#include "../core/Network.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

// ============================================================
// 多模式学习测试
// ============================================================
//
// 网络结构：
//   In_A, In_B, In_C （三个输入）
//   E1, E2, E3 （三个竞争神经元）
//   I1 （共享侧抑制）
//
// 预期：
//   训练后，每个输入对应不同的 E
//   In_A → E1
//   In_B → E2
//   In_C → E3
//
// ============================================================

void stimulate(Neuron& n, double amount) {
    for (size_t b = 0; b < n.branches.size(); ++b) {
        for (size_t i = 0; i < n.branches[b].inputs.size(); ++i) {
            n.deliver((int)b, (int)i, amount / 32.0);
        }
    }
}

struct EResponse {
    int e1, e2, e3;
    int winner() const {
        if (e1 >= e2 && e1 >= e3) return 1;
        if (e2 >= e1 && e2 >= e3) return 2;
        return 3;
    }
};

// 测量某个输入刺激下，三个 E 的响应
EResponse testInput(Network& net, int inputId, int E1, int E2, int E3) {
    net.reset();
    net.dopamine = 0.5;   // STDP 关闭

    // 恢复参数
    for (auto& n : net.neurons) {
        // reset 后恢复默认
    }
    net.neurons[E1].soma.I_pace = 3.5;
    net.neurons[E1].soma.sigma_noise = 0.03;
    net.neurons[E2].soma.I_pace = 3.5;
    net.neurons[E2].soma.sigma_noise = 0.03;
    net.neurons[E3].soma.I_pace = 3.5;
    net.neurons[E3].soma.sigma_noise = 0.03;

    const double dt = 0.05;
    int e1s = 0, e2s = 0, e3s = 0;

    // 测试：持续 3ms 强驱动 + 100ms 静息
    for (int step = 0; step < (int)(3.0 / dt); ++step) {
        stimulate(net.neurons[inputId], 5.0);
        net.step(dt);
        if (net.neurons[E1].hasSpiked) e1s++;
        if (net.neurons[E2].hasSpiked) e2s++;
        if (net.neurons[E3].hasSpiked) e3s++;
    }
    for (int step = 0; step < (int)(100.0 / dt); ++step) {
        net.step(dt);
        if (net.neurons[E1].hasSpiked) e1s++;
        if (net.neurons[E2].hasSpiked) e2s++;
        if (net.neurons[E3].hasSpiked) e3s++;
    }

    return {e1s, e2s, e3s};
}

int main() {
    std::cout << "=== Multi-Modal Learning Test ===\n\n";

    Network net;

    // 神经元
    int In_A = net.addNeuron(pyramidalType());
    int In_B = net.addNeuron(pyramidalType());
    int In_C = net.addNeuron(pyramidalType());
    int E1 = net.addNeuron(pyramidalType());
    int E2 = net.addNeuron(pyramidalType());
    int E3 = net.addNeuron(pyramidalType());
    int I1 = net.addNeuron(basketType());

    // ===== 神经元参数 =====
    // 输入神经元：低自发
    for (int id : {In_A, In_B, In_C}) {
        net.neurons[id].soma.I_pace = 2.0;
        net.neurons[id].soma.sigma_noise = 0.02;
        net.neurons[id].ais.refractoryPeriod = 5.0;
    }

    // 竞争神经元：接近阈值，有自发
    for (int id : {E1, E2, E3}) {
        net.neurons[id].soma.I_pace = 3.5;
        net.neurons[id].soma.sigma_noise = 0.03;
        net.neurons[id].ais.refractoryPeriod = 5.0;
    }

    // 抑制神经元：静息低
    net.neurons[I1].soma.I_pace = 0.5;
    net.neurons[I1].soma.sigma_noise = 0.02;

    // ===== In → E（可塑）=====
    auto connectPlastic = [&](int pre, int post, double w) {
        int id = net.connect(pre, post, 0, 0, w, 1.0, true);
        net.synapses.synapses[id].isPlastic = true;
        net.synapses.synapses[id].weightMin = 0.1;
        net.synapses.synapses[id].weightMax = 10.0;
        net.synapses.synapses[id].ampaFraction = 0.9;
        net.synapses.synapses[id].nmdaFraction = 0.1;
        net.synapses.synapses[id].tau_ampa = 5.0;
        return id;
    };

    // 每个 E 对一个输入有偏好
    int wA1 = connectPlastic(In_A, E1, 6.0);
    int wA2 = connectPlastic(In_A, E2, 1.0);
    int wA3 = connectPlastic(In_A, E3, 1.0);

    int wB1 = connectPlastic(In_B, E1, 1.0);
    int wB2 = connectPlastic(In_B, E2, 6.0);
    int wB3 = connectPlastic(In_B, E3, 1.0);

    int wC1 = connectPlastic(In_C, E1, 1.0);
    int wC2 = connectPlastic(In_C, E2, 1.0);
    int wC3 = connectPlastic(In_C, E3, 6.0);

    // ===== E → I（固定）=====
    auto connectE2I = [&](int e, int i, double w) {
        int id = net.connect(e, i, 0, 0, w, 1.0, true);
        net.synapses.synapses[id].ampaFraction = 1.0;
        net.synapses.synapses[id].nmdaFraction = 0.0;
        net.synapses.synapses[id].tau_ampa = 3.0;
        net.synapses.synapses[id].isPlastic = false;
    };
    connectE2I(E1, I1, 1.0);
    connectE2I(E2, I1, 1.0);
    connectE2I(E3, I1, 1.0);

    // ===== I → E（固定）=====
    for (int e : {E1, E2, E3}) {
        int id = net.connect(I1, e, 0, 0, 2.0, 1.0, false);
        net.synapses.synapses[id].isPlastic = false;
    }

    std::cout << "Network: 3 inputs, 3 competitive E, 1 I\n";
    std::cout << "Initial weight preference:\n";
    std::cout << "  In_A -> E1 = 6.0\n";
    std::cout << "  In_B -> E2 = 6.0\n";
    std::cout << "  In_C -> E3 = 6.0\n\n";

    // ===== 训练前测试 =====
    std::cout << "=== Before Training ===\n\n";
    auto beforeA = testInput(net, In_A, E1, E2, E3);
    auto beforeB = testInput(net, In_B, E1, E2, E3);
    auto beforeC = testInput(net, In_C, E1, E2, E3);

    std::cout << std::setw(10) << "Input"
              << std::setw(10) << "E1"
              << std::setw(10) << "E2"
              << std::setw(10) << "E3"
              << std::setw(10) << "Winner"
              << "\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << std::setw(10) << "In_A"
              << std::setw(10) << beforeA.e1
              << std::setw(10) << beforeA.e2
              << std::setw(10) << beforeA.e3
              << std::setw(10) << beforeA.winner() << "\n";
    std::cout << std::setw(10) << "In_B"
              << std::setw(10) << beforeB.e1
              << std::setw(10) << beforeB.e2
              << std::setw(10) << beforeB.e3
              << std::setw(10) << beforeB.winner() << "\n";
    std::cout << std::setw(10) << "In_C"
              << std::setw(10) << beforeC.e1
              << std::setw(10) << beforeC.e2
              << std::setw(10) << beforeC.e3
              << std::setw(10) << beforeC.winner() << "\n\n";

    // ===== 训练 =====
    std::cout << "=== Training ===\n\n";

    const double dt = 0.05;
    const int trialsPerPhase = 50;

    auto trainPhase = [&](int inputId, const char* name) {
        std::cout << "Phase: train " << name << " (" << trialsPerPhase << " trials)\n";

        for (int trial = 0; trial < trialsPerPhase; ++trial) {
            net.dopamine = 1.0;

            // 3ms 强驱动
            for (int step = 0; step < (int)(3.0 / dt); ++step) {
                stimulate(net.neurons[inputId], 5.0);
                net.step(dt);
            }
            // 100ms 静息
            for (int step = 0; step < (int)(100.0 / dt); ++step) {
                net.step(dt);
            }
        }
    };

    // reset 一次
    net.reset();
    net.neurons[In_A].soma.I_pace = 2.0;
    net.neurons[In_B].soma.I_pace = 2.0;
    net.neurons[In_C].soma.I_pace = 2.0;
    for (int id : {E1, E2, E3}) {
        net.neurons[id].soma.I_pace = 3.5;
        net.neurons[id].soma.sigma_noise = 0.03;
    }
    net.neurons[I1].soma.I_pace = 0.5;

    trainPhase(In_A, "In_A");
    trainPhase(In_B, "In_B");
    trainPhase(In_C, "In_C");

    std::cout << "\n";

    // ===== 训练后测试 =====
    std::cout << "=== After Training ===\n\n";
    auto afterA = testInput(net, In_A, E1, E2, E3);
    auto afterB = testInput(net, In_B, E1, E2, E3);
    auto afterC = testInput(net, In_C, E1, E2, E3);

    std::cout << std::setw(10) << "Input"
              << std::setw(10) << "E1"
              << std::setw(10) << "E2"
              << std::setw(10) << "E3"
              << std::setw(10) << "Winner"
              << "\n";
    std::cout << std::string(50, '-') << "\n";
    std::cout << std::setw(10) << "In_A"
              << std::setw(10) << afterA.e1
              << std::setw(10) << afterA.e2
              << std::setw(10) << afterA.e3
              << std::setw(10) << afterA.winner() << "\n";
    std::cout << std::setw(10) << "In_B"
              << std::setw(10) << afterB.e1
              << std::setw(10) << afterB.e2
              << std::setw(10) << afterB.e3
              << std::setw(10) << afterB.winner() << "\n";
    std::cout << std::setw(10) << "In_C"
              << std::setw(10) << afterC.e1
              << std::setw(10) << afterC.e2
              << std::setw(10) << afterC.e3
              << std::setw(10) << afterC.winner() << "\n\n";

    // ===== 权重总结 =====
    std::cout << "=== Weight Summary ===\n\n";
    std::cout << std::setw(12) << "Synapse"
              << std::setw(12) << "Initial"
              << std::setw(12) << "Final"
              << std::setw(12) << "Change"
              << "\n";
    std::cout << std::string(48, '-') << "\n";

    auto showSyn = [&](const char* name, int id, double init) {
        double fin = net.getWeight(id);
        std::cout << std::setw(12) << name
                  << std::setw(12) << std::fixed << std::setprecision(3)
                  << init
                  << std::setw(12) << fin
                  << std::setw(12) << (fin - init)
                  << "\n";
    };

    showSyn("In_A->E1", wA1, 6.0);
    showSyn("In_A->E2", wA2, 1.0);
    showSyn("In_A->E3", wA3, 1.0);
    std::cout << "\n";
    showSyn("In_B->E1", wB1, 1.0);
    showSyn("In_B->E2", wB2, 6.0);
    showSyn("In_B->E3", wB3, 1.0);
    std::cout << "\n";
    showSyn("In_C->E1", wC1, 1.0);
    showSyn("In_C->E2", wC2, 1.0);
    showSyn("In_C->E3", wC3, 6.0);

    // ===== 判定 =====
    std::cout << "\n=== Criteria ===\n\n";
    bool pass = true;

    if (afterA.winner() == 1) {
        std::cout << "[PASS] In_A -> E1\n";
    } else {
        std::cout << "[FAIL] In_A winner is E" << afterA.winner() << "\n";
        pass = false;
    }

    if (afterB.winner() == 2) {
        std::cout << "[PASS] In_B -> E2\n";
    } else {
        std::cout << "[FAIL] In_B winner is E" << afterB.winner() << "\n";
        pass = false;
    }

    if (afterC.winner() == 3) {
        std::cout << "[PASS] In_C -> E3\n";
    } else {
        std::cout << "[FAIL] In_C winner is E" << afterC.winner() << "\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "MULTI-MODAL LEARNING WORKS"
                                   : "MULTI-MODAL LEARNING FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
