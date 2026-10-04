#include "../core/Soma.h"
#include "../core/NeuronType.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

void testSpontaneous(const char* name, const NeuronType& type,
                     double duration_ms) {
    std::cout << "\n=== " << name << " Spontaneous ===\n\n";

    Soma soma;
    soma.I_pace = type.soma_I_pace;
    soma.g_M = type.soma_g_M;
    soma.sigma_noise = type.soma_sigma_noise;

    const double dt = 0.05;
    const int totalSteps = (int)(duration_ms / dt);

    // 统计
    double vSum = 0.0;
    double vMin = 0.0, vMax = -100.0;
    int crossings = 0;
    bool wasBelow = true;
    double threshold = -50.0;

    // 记录前 500ms 的静息电位
    double vRestSum = 0.0;
    int vRestCount = 0;

    for (int step = 0; step < totalSteps; ++step) {
        soma.step(0.0, dt);

        vSum += soma.v;
        if (soma.v < vMin) vMin = soma.v;
        if (soma.v > vMax) vMax = soma.v;

        if (step < 500.0 / dt) {
            vRestSum += soma.v;
            vRestCount++;
        }

        if (soma.v > threshold && wasBelow) {
            crossings++;
            wasBelow = false;
        }
        if (soma.v < threshold - 3.0) wasBelow = true;
    }

    double vMean = vSum / totalSteps;
    double vRest = vRestSum / vRestCount;
    double durationSec = duration_ms / 1000.0;

    std::cout << "  Rest V (first 500ms): " << std::fixed << std::setprecision(2)
              << vRest << " mV\n";
    std::cout << "  Mean V:               " << vMean << " mV\n";
    std::cout << "  V range:              [" << vMin << ", " << vMax << "] mV\n";
    std::cout << "  Threshold crossings:  " << crossings << "\n";
    std::cout << "  Estimated rate:       " << std::setprecision(2)
              << (crossings / durationSec) << " Hz\n";
}

int main() {
    std::cout << "=== Soma Tests ===\n";

    testSpontaneous("Pyramidal", pyramidalType(), 5000.0);
    testSpontaneous("Basket", basketType(), 5000.0);
    testSpontaneous("Chandelier", chandelierType(), 5000.0);

    std::cout << "\n=== All Tests Complete ===\n";
    return 0;
}
