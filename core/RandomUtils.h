#ifndef RANDOMUTILS_H
#define RANDOMUTILS_H

#include <random>

class RandomUtils {
public:
    static double uniform() {
        static std::mt19937 gen(std::random_device{}());
        static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(gen);
    }

    static double gaussian() {
        static std::mt19937 gen(std::random_device{}());
        static std::normal_distribution<double> dist(0.0, 1.0);
        return dist(gen);
    }

    static double uniformRange(double lo, double hi) {
        return lo + (hi - lo) * uniform();
    }
};
#endif // RANDOMUTILS_H
