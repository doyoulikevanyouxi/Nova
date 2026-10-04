#include "../core/AxonInitialSegment.h"
#include "../core/NeuronType.h"
#include <iostream>
#include <iomanip>
#include <vector>

void testType(const char* name, const NeuronType& type,
              const std::vector<double>& somaVoltages) {
    std::cout << "\n=== " << name << " ===\n\n";
    std::cout << std::setw(15) << "somaV(mV)"
              << std::setw(15) << "spikes"
              << std::setw(15) << "Hz"
              << "\n";
    std::cout << std::string(45, '-') << "\n";

    const double dt = 0.01;
    const double duration = 100.0;
    const int totalSteps = (int)(duration / dt);

    for (double somaV : somaVoltages) {
        AxonInitialSegment ais(type);
        int spikes = 0;

        for (int step = 0; step < totalSteps; ++step) {
            ais.step(somaV, dt);
            if (ais.hasSpiked) spikes++;
        }

        double hz = spikes / (duration / 1000.0);

        std::cout << std::setw(15) << std::fixed << std::setprecision(1)
                  << somaV
                  << std::setw(15) << spikes
                  << std::setw(15) << std::setprecision(1) << hz
                  << "\n";
    }
}

int main() {
    std::cout << "=== AIS Type-Dependent Test ===\n";

    std::vector<double> somaVoltages = {
        -70.0, -65.0, -60.0, -55.0, -52.0, -50.0, -45.0
    };

    testType("Pyramidal", pyramidalType(), somaVoltages);
    testType("Basket", basketType(), somaVoltages);
    testType("Chandelier", chandelierType(), somaVoltages);

    return 0;
}
