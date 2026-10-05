#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>

int main() {
    std::cout << "=== AHP Test (H isolated) ===\n\n";

    Network net;
    net.buildFromGenome(defaultVisualGenome());

    auto* h = net.getRegion("H");
    if (!h) { std::cout << "No H region\n"; return 1; }

    std::set<int> hSet(h->neurons.begin(), h->neurons.end());

    std::vector<int> hP;
    for (int id : h->neurons) {
        if (net.neurons[id].type.name == "Pyramidal") {
            hP.push_back(id);
        }
    }

    // 关掉所有自发
    for (auto& n : net.neurons) {
        n.soma.I_pace = 0.0;
        n.soma.sigma_noise = 0.0;
    }

    // 只恢复 H 的自发
    for (int id : hP) {
        net.neurons[id].soma.I_pace = 1.0;
        net.neurons[id].soma.sigma_noise = 0.03;
    }

    std::cout << "H pyramidal neurons: " << hP.size() << "\n";
    std::cout << "All non-H neurons forced to -90mV each step\n\n";

    const double dt = 0.05;

    std::cout << std::setw(10) << "Time(s)"
              << std::setw(15) << "H spikes"
              << std::setw(15) << "H rate (Hz)"
              << std::setw(15) << "Avg ca"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    for (int block = 0; block < 10; ++block) {
        int spikes = 0;
        double caSum = 0.0;

        for (int step = 0; step < (int)(500.0 / dt); ++step) {
            // 每步强制非 H 神经元超极化
            for (size_t i = 0; i < net.neurons.size(); ++i) {
                if (hSet.count((int)i)) continue;
                net.neurons[i].soma.v = -90.0;
                net.neurons[i].hasSpiked = false;
            }

            net.step(dt);

            for (int id : hP) {
                if (net.neurons[id].hasSpiked) spikes++;
                caSum += net.neurons[id].soma.ca_soma;
            }
        }

        double rate = spikes / (0.5 * hP.size());
        double avgCa = caSum / (hP.size() * 500.0 / dt);

        std::cout << std::setw(10) << std::fixed << std::setprecision(1)
                  << (block * 0.5 + 0.5)
                  << std::setw(15) << spikes
                  << std::setw(15) << std::setprecision(2) << rate
                  << std::setw(15) << std::setprecision(3) << avgCa
                  << "\n";
    }

    std::cout << "\n=== Criteria ===\n\n";
    std::cout << "If AHP works: H rate stable, not growing\n";
    std::cout << "If positive feedback: H rate grows over time\n";

    return 0;
}
