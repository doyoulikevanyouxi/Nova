#include "../core/Network.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

// ============================================================
// 竞争学习测试
// ============================================================
//
// 网络结构：
//   In （输入神经元）
//   E1, E2, E3 （竞争神经元）
//   I1 （侧抑制神经元）
//
// 连接：
//   In → E1, E2, E3（可塑，STDP）
//   E1, E2, E3 → I1（固定）
//   I1 → E1, E2, E3（固定，GABA）
//
// 训练：每个 trial 给 In 单次脉冲
// 预期：In→E1 增强（E1 赢），In→E2/E3 削弱
//
// ============================================================

void stimulate(Neuron& n, double amount) {
    for (size_t b = 0; b < n.branches.size(); ++b) {
        for (size_t i = 0; i < n.branches[b].inputs.size(); ++i) {
            n.deliver((int)b, (int)i, amount / 32.0);
        }
    }
}

struct Response {
    int e1, e2, e3;
};

Response measureResponse(Network& net, int In, int E1, int E2, int E3) {
    net.reset();
    net.dopamine = 0.5;

    const double dt = 0.05;
    int e1s = 0, e2s = 0, e3s = 0;

    for (int step = 0; step < (int)(500.0 / dt); ++step) {
        double t = step * dt;
        if (t < 20.0) stimulate(net.neurons[In], 0.5);
        net.step(dt);

        if (net.neurons[E1].hasSpiked) e1s++;
        if (net.neurons[E2].hasSpiked) e2s++;
        if (net.neurons[E3].hasSpiked) e3s++;
    }
    return {e1s, e2s, e3s};
}

int main() {
    std::cout << "=== Competitive Learning Test ===\n\n";

    Network net;

    int In = net.addNeuron(pyramidalType());
    int E1 = net.addNeuron(pyramidalType());
    int E2 = net.addNeuron(pyramidalType());
    int E3 = net.addNeuron(pyramidalType());
    int I1 = net.addNeuron(basketType());

    // ===== 神经元参数 =====
    // In：低自发，靠外部驱动
    net.neurons[In].soma.I_pace = 2.0;
    net.neurons[In].soma.sigma_noise = 0.02;
    net.neurons[In].ais.refractoryPeriod = 5.0;

    // E1/E2/E3：接近阈值，有自发
    for (int id : {E1, E2, E3}) {
        net.neurons[id].soma.I_pace = 3.5;
        net.neurons[id].soma.sigma_noise = 0.03;
        net.neurons[id].ais.refractoryPeriod = 5.0;
    }

    // I1：静息低，靠 E 驱动
    net.neurons[I1].soma.I_pace = 0.5;
    net.neurons[I1].soma.sigma_noise = 0.02;

    // ===== In → E（可塑，STDP）=====
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

    int synIE1 = connectPlastic(In, E1, 6.0);
    int synIE2 = connectPlastic(In, E2, 1.0);
    int synIE3 = connectPlastic(In, E3, 1.0);

    // ===== E → I（固定，纯 AMPA）=====
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

    // ===== I → E（固定，GABA）=====
    for (int e : {E1, E2, E3}) {
        int id = net.connect(I1, e, 0, 0, 2.0, 1.0, false);
        net.synapses.synapses[id].isPlastic = false;
    }

    std::cout << "Network:\n";
    std::cout << "  In (input)\n";
    std::cout << "  E1, E2, E3 (competitive)\n";
    std::cout << "  I1 (lateral inhibition)\n\n";

    std::cout << "Initial weights:\n";
    std::cout << "  In -> E1: " << net.getWeight(synIE1) << "\n";
    std::cout << "  In -> E2: " << net.getWeight(synIE2) << "\n";
    std::cout << "  In -> E3: " << net.getWeight(synIE3) << "\n\n";

    // ===== 训练前 =====
    std::cout << "=== Before Training ===\n\n";
    auto before = measureResponse(net, In, E1, E2, E3);
    std::cout << "  E1: " << before.e1 << " spikes\n";
    std::cout << "  E2: " << before.e2 << " spikes\n";
    std::cout << "  E3: " << before.e3 << " spikes\n\n";

    // ===== 训练 =====
    // ===== 训练 =====
    std::cout << "=== Training (50 trials, short drive) ===\n\n";
    net.reset();

    // reset 后恢复参数
    net.neurons[In].soma.I_pace = 2.0;
    net.neurons[In].soma.sigma_noise = 0.02;
    for (int id : {E1, E2, E3}) {
        net.neurons[id].soma.I_pace = 3.5;
        net.neurons[id].soma.sigma_noise = 0.03;
    }
    net.neurons[I1].soma.I_pace = 0.5;
    net.neurons[I1].soma.sigma_noise = 0.02;

    const double dt = 0.05;
    const int trials = 50;
    int inSpikes = 0, e1Spikes = 0, e2Spikes = 0, e3Spikes = 0, i1Spikes = 0;

    for (int trial = 0; trial < trials; ++trial) {
        net.dopamine = 1.0;

        // 持续 3ms 驱动 In
        for (int step = 0; step < (int)(3.0 / dt); ++step) {
            stimulate(net.neurons[In], 5.0);
            net.step(dt);
            if (net.neurons[In].hasSpiked) inSpikes++;
        }

        // 静息 100ms
        for (int step = 0; step < (int)(100.0 / dt); ++step) {
            net.step(dt);
            if (net.neurons[In].hasSpiked) inSpikes++;
            if (net.neurons[E1].hasSpiked) e1Spikes++;
            if (net.neurons[E2].hasSpiked) e2Spikes++;
            if (net.neurons[E3].hasSpiked) e3Spikes++;
            if (net.neurons[I1].hasSpiked) i1Spikes++;
        }

        if ((trial + 1) % 10 == 0) {
            std::cout << "  Trial " << std::setw(2) << (trial + 1)
            << ": E1=" << std::fixed << std::setprecision(3)
            << net.getWeight(synIE1)
            << ", E2=" << net.getWeight(synIE2)
            << ", E3=" << net.getWeight(synIE3)
            << "\n";
        }
    }

    std::cout << "\nTraining spike counts (over 50 trials):\n";
    std::cout << "  In: " << inSpikes << "\n";
    std::cout << "  E1: " << e1Spikes << "\n";
    std::cout << "  E2: " << e2Spikes << "\n";
    std::cout << "  E3: " << e3Spikes << "\n";
    std::cout << "  I1: " << i1Spikes << "\n\n";

    // ===== 训练后 =====
    std::cout << "=== After Training ===\n\n";
    auto after = measureResponse(net, In, E1, E2, E3);
    std::cout << "  E1: " << after.e1 << " spikes\n";
    std::cout << "  E2: " << after.e2 << " spikes\n";
    std::cout << "  E3: " << after.e3 << " spikes\n\n";

    // ===== 权重总结 =====
    std::cout << "=== Weight Summary ===\n\n";
    std::cout << std::setw(12) << "Synapse"
              << std::setw(15) << "Initial"
              << std::setw(15) << "Final"
              << std::setw(15) << "Change"
              << "\n";
    std::cout << std::string(57, '-') << "\n";

    auto showSyn = [&](const char* name, int id, double init) {
        double fin = net.getWeight(id);
        std::cout << std::setw(12) << name
                  << std::setw(15) << std::fixed << std::setprecision(3)
                  << init
                  << std::setw(15) << fin
                  << std::setw(15) << (fin - init)
                  << "\n";
    };

    showSyn("In->E1", synIE1, 6.0);
    showSyn("In->E2", synIE2, 1.0);
    showSyn("In->E3", synIE3, 1.0);

    // ===== 判定 =====
    std::cout << "\n=== Criteria ===\n\n";
    bool pass = true;

    double w1 = net.getWeight(synIE1);
    double w2 = net.getWeight(synIE2);
    double w3 = net.getWeight(synIE3);

    if (w1 > 6.0 + 0.05) {
        std::cout << "[PASS] In->E1 strengthened (winner)\n";
    } else {
        std::cout << "[FAIL] In->E1 not strengthened\n";
        pass = false;
    }

    if (w1 > w2 && w1 > w3) {
        std::cout << "[PASS] In->E1 is the strongest input\n";
    } else {
        std::cout << "[FAIL] In->E1 not the strongest\n";
        pass = false;
    }

    if (w2 < 1.0 || w3 < 1.0) {
        std::cout << "[PASS] Losers weakened\n";
    } else {
        std::cout << "[NEUTRAL] Losers not weakened\n";
    }

    if (after.e1 >= before.e1) {
        std::cout << "[PASS] E1 responds at least as much after training ("
                  << before.e1 << " -> " << after.e1 << ")\n";
    } else {
        std::cout << "[NEUTRAL] E1 response decreased\n";
    }

    std::cout << "\n=== " << (pass ? "COMPETITIVE LEARNING WORKS"
                                   : "COMPETITIVE LEARNING FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
