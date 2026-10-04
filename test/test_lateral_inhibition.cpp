#include "../core/Network.h"
#include <iostream>
#include <iomanip>
#include <vector>

void stimulate(Neuron& n, double amount) {
    for (size_t b = 0; b < n.branches.size(); ++b) {
        for (size_t i = 0; i < n.branches[b].inputs.size(); ++i) {
            n.deliver((int)b, (int)i, amount / 32.0);
        }
    }
}

struct Rates {
    double E1, E2, E3, I1;
};

// 测量指定时间内的放电率（Hz）
Rates measureRates(Network& net,
                   int E1, int E2, int E3, int I1,
                   double durationMs,
                   int drivenNeuron = -1,
                   double driveAmount = 0.0) {

    const double dt = 0.05;
    const int totalSteps = (int)(durationMs / dt);

    int e1Spikes = 0, e2Spikes = 0, e3Spikes = 0, i1Spikes = 0;

    for (int step = 0; step < totalSteps; ++step) {
        // 持续驱动指定神经元
        if (drivenNeuron >= 0) {
            stimulate(net.neurons[drivenNeuron], driveAmount);
        }

        net.step(dt);

        if (net.neurons[E1].hasSpiked) e1Spikes++;
        if (net.neurons[E2].hasSpiked) e2Spikes++;
        if (net.neurons[E3].hasSpiked) e3Spikes++;
        if (net.neurons[I1].hasSpiked) i1Spikes++;
    }

    double sec = durationMs / 1000.0;
    Rates r;
    r.E1 = e1Spikes / sec;
    r.E2 = e2Spikes / sec;
    r.E3 = e3Spikes / sec;
    r.I1 = i1Spikes / sec;
    return r;
}

int main() {
    std::cout << "=== Lateral Inhibition Test ===\n\n";

    Network net;

    int E1 = net.addNeuron(pyramidalType());
    int E2 = net.addNeuron(pyramidalType());
    int E3 = net.addNeuron(pyramidalType());
    int I1 = net.addNeuron(basketType());

    // E：低自发（不主动放电，被刺激才放）
    for (int id : {E1, E2, E3}) {
        net.neurons[id].soma.I_pace = 3.0;
        net.neurons[id].soma.sigma_noise = 0.02;
        net.neurons[id].ais.refractoryPeriod = 5.0;
    }

    // I：低自发，靠 E 驱动
    net.neurons[I1].soma.I_pace = 0.5;
    net.neurons[I1].soma.sigma_noise = 0.02;
    net.neurons[I1].ais.refractoryPeriod = 2.0;

    std::cout << "E neurons: I_pace=3.0, noise=0.02 (low spontaneous)\n";
    std::cout << "I neuron:  I_pace=0.5 (silent unless driven by E)\n\n";

    // ===== E -> I（兴奋性，纯 AMPA）=====
    auto connectE2I = [&](int e, int i, double w) {
        int id = net.connect(e, i, 0, 0, w, 1.0, true);
        net.synapses.synapses[id].ampaFraction = 1.0;
        net.synapses.synapses[id].nmdaFraction = 0.0;
        net.synapses.synapses[id].tau_ampa = 3.0;
    };

    connectE2I(E1, I1, 0.6);
    connectE2I(E2, I1, 0.6);
    connectE2I(E3, I1, 0.6);

    // ===== I -> E（抑制性，GABA）=====
    net.connect(I1, E1, 0, 0, 1.5, 1.0, false);
    net.connect(I1, E2, 0, 0, 1.5, 1.0, false);
    net.connect(I1, E3, 0, 0, 1.5, 1.0, false);

    // 关闭 STDP
    for (auto& s : net.synapses.synapses) s.isPlastic = false;

    std::cout << "Synapses:\n";
    std::cout << "  E1,E2,E3 -> I1: weight=0.6, AMPA\n";
    std::cout << "  I1 -> E1,E2,E3: weight=1.5, GABA\n\n";

    const double dt = 0.05;

    // ===== 基线（无刺激）=====
    std::cout << "=== Baseline (no stimulation) ===\n\n";
    net.reset();
    for (int step = 0; step < (int)(500.0 / dt); ++step) net.step(dt);

    auto baseline = measureRates(net, E1, E2, E3, I1, 1000.0);

    std::cout << std::setw(10) << "Neuron"
              << std::setw(15) << "Rate (Hz)"
              << "\n";
    std::cout << std::string(25, '-') << "\n";
    std::cout << std::setw(10) << "E1"
              << std::setw(15) << std::fixed << std::setprecision(2)
              << baseline.E1 << "\n";
    std::cout << std::setw(10) << "E2"
              << std::setw(15) << baseline.E2 << "\n";
    std::cout << std::setw(10) << "E3"
              << std::setw(15) << baseline.E3 << "\n";
    std::cout << std::setw(10) << "I1"
              << std::setw(15) << baseline.I1 << "\n\n";

    // ===== 刺激 E1 =====
    std::cout << "=== Stimulate E1 ===\n\n";
    net.reset();
    for (int step = 0; step < (int)(500.0 / dt); ++step) net.step(dt);

    auto driven1 = measureRates(net, E1, E2, E3, I1, 1000.0, E1, 0.5);

    std::cout << std::setw(10) << "Neuron"
              << std::setw(15) << "Rate (Hz)"
              << std::setw(15) << "Change"
              << "\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << std::setw(10) << "E1"
              << std::setw(15) << std::fixed << std::setprecision(2)
              << driven1.E1
              << std::setw(15) << (driven1.E1 - baseline.E1) << "\n";
    std::cout << std::setw(10) << "E2"
              << std::setw(15) << driven1.E2
              << std::setw(15) << (driven1.E2 - baseline.E2) << "\n";
    std::cout << std::setw(10) << "E3"
              << std::setw(15) << driven1.E3
              << std::setw(15) << (driven1.E3 - baseline.E3) << "\n";
    std::cout << std::setw(10) << "I1"
              << std::setw(15) << driven1.I1
              << std::setw(15) << (driven1.I1 - baseline.I1) << "\n\n";

    // ===== 刺激 E2 =====
    std::cout << "=== Stimulate E2 ===\n\n";
    net.reset();
    for (int step = 0; step < (int)(500.0 / dt); ++step) net.step(dt);

    auto driven2 = measureRates(net, E1, E2, E3, I1, 1000.0, E2, 0.5);

    std::cout << std::setw(10) << "Neuron"
              << std::setw(15) << "Rate (Hz)"
              << std::setw(15) << "Change"
              << "\n";
    std::cout << std::string(40, '-') << "\n";
    std::cout << std::setw(10) << "E1"
              << std::setw(15) << std::fixed << std::setprecision(2)
              << driven2.E1
              << std::setw(15) << (driven2.E1 - baseline.E1) << "\n";
    std::cout << std::setw(10) << "E2"
              << std::setw(15) << driven2.E2
              << std::setw(15) << (driven2.E2 - baseline.E2) << "\n";
    std::cout << std::setw(10) << "E3"
              << std::setw(15) << driven2.E3
              << std::setw(15) << (driven2.E3 - baseline.E3) << "\n";
    std::cout << std::setw(10) << "I1"
              << std::setw(15) << driven2.I1
              << std::setw(15) << (driven2.I1 - baseline.I1) << "\n\n";

    // ===== 判定 =====
    std::cout << "=== Criteria ===\n\n";
    bool pass = true;

    // 1. 刺激 E1 时，E1 活跃，E2/E3 被抑制
    if (driven1.E1 > baseline.E1 + 5.0) {
        std::cout << "[PASS] E1 driven higher when stimulated ("
                  << baseline.E1 << " -> " << driven1.E1 << " Hz)\n";
    } else {
        std::cout << "[FAIL] E1 not activated\n";
        pass = false;
    }

    if (driven1.E2 < driven1.E1 * 0.5) {
        std::cout << "[PASS] E2 suppressed by lateral inhibition ("
                  << driven1.E2 << " Hz vs E1 " << driven1.E1 << " Hz)\n";
    } else {
        std::cout << "[FAIL] E2 not suppressed\n";
        pass = false;
    }

    if (driven1.E3 < driven1.E1 * 0.5) {
        std::cout << "[PASS] E3 suppressed by lateral inhibition ("
                  << driven1.E3 << " Hz vs E1 " << driven1.E1 << " Hz)\n";
    } else {
        std::cout << "[FAIL] E3 not suppressed\n";
        pass = false;
    }

    // 2. 刺激 E2 时，E2 活跃，E1/E3 被抑制
    if (driven2.E2 > baseline.E2 + 5.0) {
        std::cout << "[PASS] E2 driven higher when stimulated ("
                  << baseline.E2 << " -> " << driven2.E2 << " Hz)\n";
    } else {
        std::cout << "[FAIL] E2 not activated\n";
        pass = false;
    }

    if (driven2.E1 < driven2.E2 * 0.5) {
        std::cout << "[PASS] E1 suppressed ("
                  << driven2.E1 << " Hz vs E2 " << driven2.E2 << " Hz)\n";
    } else {
        std::cout << "[FAIL] E1 not suppressed\n";
        pass = false;
    }

    // 3. I1 在刺激时放电率上升
    if (driven1.I1 > baseline.I1) {
        std::cout << "[PASS] I1 activated by E drive ("
                  << baseline.I1 << " -> " << driven1.I1 << " Hz)\n";
    } else {
        std::cout << "[FAIL] I1 not activated\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "LATERAL INHIBITION WORKS"
                                   : "LATERAL INHIBITION FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
