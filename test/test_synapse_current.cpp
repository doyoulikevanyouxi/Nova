#include "../core/Synapse.h"
#include "../core/SynapsePopulation.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

int main() {
    std::cout << "=== Synapse Current Decay Test ===\n\n";

    // 创建一个突触
    Synapse syn;
    syn.preId = 0;
    syn.postId = 1;
    syn.weight = 1.0;
    syn.delayMs = 1.0;
    syn.nReleaseSites = 100;      // 高释放位点，降低随机性
    syn.releaseProb = 1.0;        // 必释放
    syn.quantalSize = 1.0;
    syn.tau_ampa = 5.0;
    syn.tau_nmda = 100.0;
    syn.ampaFraction = 0.7;
    syn.nmdaFraction = 0.3;

    SynapsePopulation pop;
    pop.addSynapse(syn);

    std::cout << "Parameters:\n";
    std::cout << "  Release amount:  " << syn.meanRelease() << "\n";
    std::cout << "  AMPA fraction:   " << syn.ampaFraction << "\n";
    std::cout << "  NMDA fraction:   " << syn.nmdaFraction << "\n";
    std::cout << "  tau_AMPA:        " << syn.tau_ampa << " ms\n";
    std::cout << "  tau_NMDA:        " << syn.tau_nmda << " ms\n";
    std::cout << "  Delay:           " << syn.delayMs << " ms\n\n";

    // 记录轨迹（每 0.5ms 记录一次）
    struct Sample {
        double t;
        double i_ampa;
        double i_nmda;
        double i_total;
    };
    std::vector<Sample> trace;

    const double dt = 0.05;
    const double duration = 200.0;
    const int totalSteps = (int)(duration / dt);
    const int recordEvery = 10;   // 每 10 步记录 = 0.5ms

    auto deliver = [](int, int, int, double) {};

    // t=0 触发突触前放电
    pop.onPresynapticSpike(0, 0.0);

    for (int step = 0; step < totalSteps; ++step) {
        double t = step * dt;
        pop.step(t, dt, deliver);

        if (step % recordEvery == 0) {
            trace.push_back({
                t,
                pop.synapses[0].I_ampa,
                pop.synapses[0].I_nmda,
                pop.synapses[0].current()
            });
        }
    }

    // ===== 打印关键时间点 =====
    std::cout << std::setw(10) << "time(ms)"
              << std::setw(15) << "I_AMPA"
              << std::setw(15) << "I_NMDA"
              << std::setw(15) << "I_total"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    // 选择要打印的时间点
    std::vector<double> printTimes = {
        0.0, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0,
        10.0, 20.0, 50.0, 100.0
    };

    for (double pt : printTimes) {
        // 找最近的采样点
        int bestIdx = -1;
        double bestDiff = 1e9;
        for (size_t i = 0; i < trace.size(); ++i) {
            double diff = std::abs(trace[i].t - pt);
            if (diff < bestDiff) {
                bestDiff = diff;
                bestIdx = (int)i;
            }
        }
        if (bestIdx >= 0 && bestDiff < 0.3) {
            auto& s = trace[bestIdx];
            std::cout << std::setw(10) << std::fixed << std::setprecision(2)
                      << s.t
                      << std::setw(15) << std::setprecision(4) << s.i_ampa
                      << std::setw(15) << s.i_nmda
                      << std::setw(15) << s.i_total
                      << "\n";
        }
    }

    // ===== 分析 =====
    std::cout << "\n=== Analysis ===\n\n";

    // 峰值
    double peakTotal = 0.0;
    double peakTime = 0.0;
    for (auto& s : trace) {
        if (s.i_total > peakTotal) {
            peakTotal = s.i_total;
            peakTime = s.t;
        }
    }

    double theoreticalPeak = syn.meanRelease();

    std::cout << "Peak total current:  " << std::fixed << std::setprecision(4)
              << peakTotal << " at t=" << std::setprecision(2)
              << peakTime << " ms\n";
    std::cout << "Theoretical peak:    " << std::setprecision(4)
              << theoreticalPeak << "\n\n";

    // AMPA 在 t = delay + 5ms = 6ms 的值
    double targetTime = syn.delayMs + syn.tau_ampa;
    double ampaAt6ms = 0.0;
    for (auto& s : trace) {
        if (std::abs(s.t - targetTime) < 0.3) {
            ampaAt6ms = s.i_ampa;
            break;
        }
    }

    double expectedAmpaAt6ms = theoreticalPeak * syn.ampaFraction
                               * std::exp(-1.0);

    std::cout << "AMPA at t=" << targetTime << "ms:\n";
    std::cout << "  Measured:  " << std::setprecision(4)
              << ampaAt6ms << "\n";
    std::cout << "  Expected:  " << expectedAmpaAt6ms
              << "  (peak * 0.7 * 1/e)\n\n";

    // NMDA 在 t = delay + 100ms = 101ms 的值
    double nmdaTargetTime = syn.delayMs + syn.tau_nmda;
    double nmdaAt101ms = 0.0;
    for (auto& s : trace) {
        if (std::abs(s.t - nmdaTargetTime) < 0.3) {
            nmdaAt101ms = s.i_nmda;
            break;
        }
    }

    double expectedNmdaAt101ms = theoreticalPeak * syn.nmdaFraction
                                 * std::exp(-1.0);

    std::cout << "NMDA at t=" << nmdaTargetTime << "ms:\n";
    std::cout << "  Measured:  " << std::setprecision(4)
              << nmdaAt101ms << "\n";
    std::cout << "  Expected:  " << expectedNmdaAt101ms
              << "  (peak * 0.3 * 1/e)\n\n";

    // ===== 判定 =====
    bool pass = true;

    // 峰值
    if (std::abs(peakTotal - theoreticalPeak) / theoreticalPeak < 0.01) {
        std::cout << "[PASS] Peak current matches theory\n";
    } else {
        std::cout << "[FAIL] Peak current wrong\n";
        pass = false;
    }

    // AMPA 衰减
    if (expectedAmpaAt6ms > 0
        && std::abs(ampaAt6ms - expectedAmpaAt6ms) / expectedAmpaAt6ms < 0.1) {
        std::cout << "[PASS] AMPA decays with tau=5ms\n";
    } else {
        std::cout << "[FAIL] AMPA decay wrong\n";
        pass = false;
    }

    // NMDA 衰减
    if (expectedNmdaAt101ms > 0
        && std::abs(nmdaAt101ms - expectedNmdaAt101ms) / expectedNmdaAt101ms < 0.1) {
        std::cout << "[PASS] NMDA decays with tau=100ms\n";
    } else {
        std::cout << "[FAIL] NMDA decay wrong\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "ALL TESTS PASSED" : "TESTS FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
