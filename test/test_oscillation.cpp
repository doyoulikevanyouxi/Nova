#include "../core/Network.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

void stimulate(Neuron& n, double amount) {
    for (size_t b = 0; b < n.branches.size(); ++b) {
        for (size_t i = 0; i < n.branches[b].inputs.size(); ++i) {
            n.deliver((int)b, (int)i, amount / 32.0);   // 分摊到 32 个通道
        }
    }
}

int main() {
    std::cout << "=== E/I Oscillation Test (driven) ===\n\n";

    Network net;

    int E1 = net.addNeuron(pyramidalType());
    int I1 = net.addNeuron(basketType());

    // E：低自发，长不应期（每个驱动周期只放 1 个 spike）
    net.neurons[E1].soma.I_pace = 3.0;
    net.neurons[E1].soma.sigma_noise = 0.02;
    net.neurons[E1].ais.refractoryPeriod = 30.0;   // ← 新增

    // I：极低自发
    net.neurons[I1].soma.I_pace = 0.5;
    net.neurons[I1].soma.sigma_noise = 0.02;

    std::cout << "Network: 1 E + 1 I\n";
    std::cout << "E: I_pace=3.0, noise=0.02 (driven by external)\n";
    std::cout << "I: I_pace=0.5, noise=0.02 (silent unless driven)\n\n";

    // E -> I
    int synEI = net.connect(E1, I1, 0, 0, 1.5, 1.0, true);   // 0.8 -> 1.5
    net.synapses.synapses[synEI].ampaFraction = 1.0;
    net.synapses.synapses[synEI].nmdaFraction = 0.0;
    net.synapses.synapses[synEI].tau_ampa = 3.0;

    // I -> E（加强抑制）
    int synIE = net.connect(I1, E1, 0, 0, 2.0, 1.0, false);   // 1.0 -> 2.0

    for (auto& s : net.synapses.synapses) s.isPlastic = false;

    std::cout << "E->I: AMPA, tau=3ms, w=0.8\n";
    std::cout << "I->E: GABA, w=1.0\n\n";

    const double dt = 0.05;
    const double duration = 2000.0;
    const int totalSteps = (int)(duration / dt);
    const int settleSteps = (int)(500.0 / dt);

    std::vector<double> eSpikes, iSpikes;

    // 外部驱动：每 100ms 给 E 一个短暂强输入
    const double driveInterval = 100.0;
    const double driveDuration = 5.0;

    for (int step = 0; step < totalSteps; ++step) {
        double t = step * dt;

        // 周期性外部驱动（降低强度）
        double phaseT = std::fmod(t, driveInterval);
        if (phaseT < 20.0) {
            stimulate(net.neurons[E1], 1.0);
        }

        net.step(dt);

        if (step > settleSteps) {
            if (net.neurons[E1].hasSpiked) eSpikes.push_back(t);
            if (net.neurons[I1].hasSpiked) iSpikes.push_back(t);
        }
    }

    std::cout << "=== Results ===\n\n";
    std::cout << "E spikes: " << eSpikes.size() << "\n";
    std::cout << "I spikes: " << iSpikes.size() << "\n";

    double windowSec = (duration - 500.0) / 1000.0;
    std::cout << "E rate: " << std::fixed << std::setprecision(2)
              << (eSpikes.size() / windowSec) << " Hz\n";
    std::cout << "I rate: " << (iSpikes.size() / windowSec) << " Hz\n\n";

    if (eSpikes.size() < 20) {
        std::cout << "[FAIL] Too few E spikes\n";
        return 1;
    }

    // ISI 分析
    std::vector<double> isis;
    for (size_t i = 1; i < eSpikes.size(); ++i) {
        double isi = eSpikes[i] - eSpikes[i-1];
        if (isi > 1.0) isis.push_back(isi);
    }

    double isiMean = 0;
    for (double x : isis) isiMean += x;
    isiMean /= isis.size();

    double varSum = 0;
    for (double x : isis) varSum += (x - isiMean) * (x - isiMean);
    double isiStd = std::sqrt(varSum / isis.size());
    double cv = isiStd / isiMean;

    std::cout << "=== ISI Analysis ===\n\n";
    std::cout << "  Count:  " << isis.size() << "\n";
    std::cout << "  Mean:   " << std::fixed << std::setprecision(2)
              << isiMean << " ms\n";
    std::cout << "  Std:    " << isiStd << " ms\n";
    std::cout << "  CV:     " << std::setprecision(3) << cv << "\n";
    std::cout << "  Freq:   " << std::setprecision(2)
              << (1000.0 / isiMean) << " Hz\n\n";

    // 关键：看 I 是否在 E 之后放电
    std::cout << "=== E/I Timing ===\n\n";
    if (!eSpikes.empty() && !iSpikes.empty()) {
        std::cout << "First 5 E-I pairs:\n";
        int shown = 0;
        for (double eT : eSpikes) {
            // 找这个 E spike 之后的第一个 I spike
            for (double iT : iSpikes) {
                if (iT > eT && iT < eT + 10.0) {
                    std::cout << "  E at " << std::fixed << std::setprecision(2)
                    << eT << " ms, I at " << iT
                    << " ms (delay = " << (iT - eT) << " ms)\n";
                    shown++;
                    break;
                }
            }
            if (shown >= 5) break;
        }
    }

    std::cout << "\n=== Criteria ===\n\n";

    bool pass = true;

    if (isiMean > 10.0 && isiMean < 50.0) {
        std::cout << "[PASS] ISI in gamma range\n";
    } else {
        std::cout << "[FAIL] ISI out of range: " << isiMean << " ms\n";
        pass = false;
    }

    if (cv < 0.5) {
        std::cout << "[PASS] Regular oscillation (CV = "
                  << std::setprecision(3) << cv << ")\n";
    } else {
        std::cout << "[FAIL] Irregular (CV = " << cv << ")\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "OSCILLATION DETECTED" : "NO OSCILLATION")
              << " ===\n";

    return pass ? 0 : 1;
}
