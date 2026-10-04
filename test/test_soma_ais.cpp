#include "../core/Soma.h"
#include "../core/AxonInitialSegment.h"
#include "../core/NeuronType.h"
#include <iostream>
#include <iomanip>
#include <vector>

// 测试：胞体 + AIS 组合，测量真正的自发率
void testSpontaneous(const char* name, const NeuronType& type,
                     double duration_ms) {
    std::cout << "\n=== " << name << " ===\n\n";

    Soma soma;
    soma.I_pace = type.soma_I_pace;
    soma.g_M = type.soma_g_M;
    soma.sigma_noise = type.soma_sigma_noise;

    AxonInitialSegment ais(type);

    const double dt = 0.05;
    const int totalSteps = (int)(duration_ms / dt);

    int spikes = 0;
    double vSum = 0.0;
    double vMin = 0.0, vMax = -100.0;

    for (int step = 0; step < totalSteps; ++step) {
        // 胞体步骤（无外部输入）
        soma.step(0.0, dt);

        // AIS 步骤，接收胞体电位
        ais.step(soma.v, dt);

        if (ais.hasSpiked) spikes++;
        vSum += soma.v;
        if (soma.v < vMin) vMin = soma.v;
        if (soma.v > vMax) vMax = soma.v;
    }

    double vMean = vSum / totalSteps;
    double durationSec = duration_ms / 1000.0;
    double rate = spikes / durationSec;

    std::cout << "  Soma mean V:      " << std::fixed << std::setprecision(2)
              << vMean << " mV\n";
    std::cout << "  Soma V range:     [" << vMin << ", " << vMax << "] mV\n";
    std::cout << "  AIS threshold:    " << type.ais_threshold << " mV\n";
    std::cout << "  AIS spikes:       " << spikes << "\n";
    std::cout << "  Spontaneous rate: " << std::setprecision(2)
              << rate << " Hz\n";
}

int main() {
    std::cout << "=== Soma + AIS Spontaneous Test ===\n";

    testSpontaneous("Pyramidal", pyramidalType(), 5000.0);
    testSpontaneous("Basket", basketType(), 5000.0);
    testSpontaneous("Chandelier", chandelierType(), 5000.0);

    std::cout << "\nTarget rates:\n";
    std::cout << "  Pyramidal:  0.5 - 2 Hz\n";
    std::cout << "  Basket:     5 - 15 Hz\n";
    std::cout << "  Chandelier: 2 - 5 Hz\n";

    std::cout << "\n=== All Tests Complete ===\n";
    return 0;
}


// #include "../core/Soma.h"
// #include "../core/AxonInitialSegment.h"
// #include "../core/NeuronType.h"
// #include <iostream>
// #include <iomanip>
// #include <vector>

// struct TestResult {
//     double ipace;
//     double noise;
//     double vRest;
//     double rate;
// };

// TestResult measureRate(const NeuronType& type,
//                        double ipace, double noise,
//                        double duration_ms) {
//     Soma soma;
//     soma.I_pace = ipace;
//     soma.g_M = type.soma_g_M;
//     soma.sigma_noise = noise;

//     AxonInitialSegment ais(type);

//     const double dt = 0.05;
//     const int totalSteps = (int)(duration_ms / dt);

//     int spikes = 0;
//     double vSum = 0.0;
//     int vCount = 0;

//     // 前 500ms 用于稳定，之后开始统计
//     const int settleSteps = (int)(500.0 / dt);

//     for (int step = 0; step < totalSteps; ++step) {
//         soma.step(0.0, dt);
//         ais.step(soma.v, dt);

//         if (ais.hasSpiked) spikes++;

//         if (step > settleSteps) {
//             vSum += soma.v;
//             vCount++;
//         }
//     }

//     TestResult r;
//     r.ipace = ipace;
//     r.noise = noise;
//     r.vRest = vSum / vCount;
//     r.rate = spikes / (duration_ms / 1000.0);
//     return r;
// }

// void sweepType(const char* name, const NeuronType& type,
//                double targetMin, double targetMax) {
//     std::cout << "\n=== " << name << " ===\n\n";
//     std::cout << "Target rate: " << targetMin << " - " << targetMax << " Hz\n\n";

//     std::cout << std::setw(10) << "I_pace"
//               << std::setw(10) << "noise"
//               << std::setw(12) << "V_rest"
//               << std::setw(12) << "rate"
//               << std::setw(10) << "match"
//               << "\n";
//     std::cout << std::string(54, '-') << "\n";

//     std::vector<double> ipaces = {2.0, 2.5, 3.0, 3.5, 4.0};
//     std::vector<double> noises = {0.01, 0.02, 0.03, 0.05};

//     for (double ipace : ipaces) {
//         for (double noise : noises) {
//             TestResult r = measureRate(type, ipace, noise, 2000.0);

//             bool match = (r.rate >= targetMin && r.rate <= targetMax);

//             std::cout << std::setw(10) << std::fixed << std::setprecision(1)
//                       << ipace
//                       << std::setw(10) << std::setprecision(2) << noise
//                       << std::setw(12) << std::setprecision(2) << r.vRest
//                       << std::setw(12) << std::setprecision(2) << r.rate
//                       << std::setw(10) << (match ? "YES" : "")
//                       << "\n";
//         }
//     }
// }

// int main() {
//     std::cout << "=== Soma Parameter Sweep ===\n";
//     std::cout << "Each cell: 2 seconds simulation\n";

//     sweepType("Pyramidal", pyramidalType(), 0.5, 2.0);
//     sweepType("Basket", basketType(), 5.0, 15.0);
//     sweepType("Chandelier", chandelierType(), 2.0, 5.0);

//     std::cout << "\n=== Sweep Complete ===\n";
//     std::cout << "Pick parameters where 'match' = YES\n";
//     return 0;
// }
