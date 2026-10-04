#include "../core/Synapse.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

int main() {
    std::cout << "=== Synapse Quantum Release Test ===\n\n";

    // 创建一个突触
    Synapse syn;
    syn.nReleaseSites = 5;
    syn.releaseProb = 0.3;
    syn.quantalSize = 0.5;
    syn.weight = 1.0;

    // 理论值
    double theoryMean = syn.meanRelease();
    double theoryVar = syn.releaseVariance();
    double theoryStd = std::sqrt(theoryVar);

    std::cout << "Parameters:\n";
    std::cout << "  Release sites: " << syn.nReleaseSites << "\n";
    std::cout << "  Release prob:  " << syn.releaseProb << "\n";
    std::cout << "  Quantal size:  " << syn.quantalSize << "\n";
    std::cout << "  Weight:        " << syn.weight << "\n\n";

    std::cout << "Theory (Binomial distribution):\n";
    std::cout << "  Mean:     " << std::fixed << std::setprecision(4)
              << theoryMean << "\n";
    std::cout << "  Variance: " << theoryVar << "\n";
    std::cout << "  Std:      " << theoryStd << "\n\n";

    // 模拟：给突触前 10000 个脉冲，统计释放分布
    const int nTrials = 10000;
    std::vector<double> releaseAmounts;
    releaseAmounts.reserve(nTrials);

    // 统计每个释放量子数的出现次数
    std::vector<int> histogram(syn.nReleaseSites + 1, 0);

    for (int i = 0; i < nTrials; ++i) {
        double amount = syn.release(true);
        releaseAmounts.push_back(amount);

        // 反推释放的量子数
        int nQuantal = (int)std::round(amount / (syn.quantalSize * syn.weight));
        if (nQuantal >= 0 && nQuantal <= syn.nReleaseSites) {
            histogram[nQuantal]++;
        }
    }

    // 统计实测均值
    double sum = 0.0;
    for (double x : releaseAmounts) sum += x;
    double measuredMean = sum / nTrials;

    // 统计实测方差
    double varSum = 0.0;
    for (double x : releaseAmounts) {
        double d = x - measuredMean;
        varSum += d * d;
    }
    double measuredVar = varSum / nTrials;
    double measuredStd = std::sqrt(measuredVar);

    std::cout << "Measured (n=" << nTrials << "):\n";
    std::cout << "  Mean:     " << measuredMean << "\n";
    std::cout << "  Variance: " << measuredVar << "\n";
    std::cout << "  Std:      " << measuredStd << "\n\n";

    // 释放量子数的分布
    std::cout << "Quantal release distribution:\n";
    std::cout << std::setw(10) << "Quanta"
              << std::setw(12) << "Count"
              << std::setw(12) << "Frequency"
              << std::setw(12) << "Theory"
              << "\n";
    std::cout << std::string(46, '-') << "\n";

    // 理论二项分布
    auto binom = [](int n, int k, double p) {
        // C(n,k) * p^k * (1-p)^(n-k)
        double c = 1.0;
        for (int i = 0; i < k; ++i) {
            c *= (double)(n - i) / (i + 1);
        }
        return c * std::pow(p, k) * std::pow(1.0 - p, n - k);
    };

    for (int k = 0; k <= syn.nReleaseSites; ++k) {
        double freq = (double)histogram[k] / nTrials;
        double theory = binom(syn.nReleaseSites, k, syn.releaseProb);
        std::cout << std::setw(10) << k
                  << std::setw(12) << histogram[k]
                  << std::setw(12) << std::setprecision(4) << freq
                  << std::setw(12) << theory
                  << "\n";
    }

    // ===== 验证 =====
    std::cout << "\n=== Verification ===\n\n";

    bool pass = true;

    // 均值接近
    double meanError = std::abs(measuredMean - theoryMean) / theoryMean;
    if (meanError < 0.05) {
        std::cout << "[PASS] Mean matches theory (error = "
                  << std::setprecision(2) << (meanError * 100) << "%)\n";
    } else {
        std::cout << "[FAIL] Mean does not match (error = "
                  << (meanError * 100) << "%)\n";
        pass = false;
    }

    // 方差接近
    double varError = std::abs(measuredVar - theoryVar) / theoryVar;
    if (varError < 0.1) {
        std::cout << "[PASS] Variance matches theory (error = "
                  << (varError * 100) << "%)\n";
    } else {
        std::cout << "[FAIL] Variance does not match (error = "
                  << (varError * 100) << "%)\n";
        pass = false;
    }

    // ===== 测试不同释放概率 =====
    std::cout << "\n=== Release Probability Sweep ===\n\n";
    std::cout << std::setw(12) << "releaseProb"
              << std::setw(12) << "mean"
              << std::setw(12) << "std"
              << std::setw(12) << "CV"
              << "\n";
    std::cout << std::string(48, '-') << "\n";

    for (double p : {0.1, 0.3, 0.5, 0.7, 0.9}) {
        Synapse s;
        s.nReleaseSites = 5;
        s.releaseProb = p;
        s.quantalSize = 0.5;

        std::vector<double> amounts;
        for (int i = 0; i < nTrials; ++i) {
            amounts.push_back(s.release(true));
        }

        double m = 0.0;
        for (double x : amounts) m += x;
        m /= nTrials;

        double v = 0.0;
        for (double x : amounts) v += (x - m) * (x - m);
        v /= nTrials;
        double sd = std::sqrt(v);

        double cv = (m > 1e-9) ? (sd / m) : 0.0;

        std::cout << std::setw(12) << std::setprecision(2) << p
                  << std::setw(12) << std::setprecision(4) << m
                  << std::setw(12) << sd
                  << std::setw(12) << std::setprecision(3) << cv
                  << "\n";
    }

    std::cout << "\nNote: CV = coefficient of variation = std/mean\n";
    std::cout << "Low p -> high CV (unreliable synapse)\n";
    std::cout << "High p -> low CV (reliable synapse)\n";

    std::cout << "\n=== " << (pass ? "ALL TESTS PASSED" : "TESTS FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
