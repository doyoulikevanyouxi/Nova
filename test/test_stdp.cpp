#include "../core/Network.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

// ============================================================
// 测试 1：因果（pre → post）→ LTP
// 直接调用 Synapse 的 onPreSpike/onPostSpike
// ============================================================
void testCausalLTP() {
    std::cout << "=== Test 1: Causal (pre -> post) -> LTP ===\n\n";

    Synapse s;
    s.weight = 1.0;
    s.nReleaseSites = 0;   // 关闭释放

    double w0 = s.weight;

    // pre 在 t=0，post 在 t=5ms
    s.onPreSpike(0.0, 1.0);
    s.onPostSpike(5.0, 1.0);

    double w1 = s.weight;

    std::cout << "Weight: " << std::fixed << std::setprecision(6)
              << w0 << " -> " << w1 << "\n";
    std::cout << "Delta: " << (w1 - w0) << "\n";
    std::cout << (w1 > w0 ? "[PASS] LTP" : "[FAIL]") << "\n\n";
}

// ============================================================
// 测试 2：反因果（post → pre）→ LTD
// ============================================================
void testAntiCausalLTD() {
    std::cout << "=== Test 2: Anti-causal (post -> pre) -> LTD ===\n\n";

    Synapse s;
    s.weight = 1.0;
    s.nReleaseSites = 0;

    double w0 = s.weight;

    // post 在 t=0，pre 在 t=5ms
    s.onPostSpike(0.0, 1.0);
    s.onPreSpike(5.0, 1.0);

    double w1 = s.weight;

    std::cout << "Weight: " << w0 << " -> " << w1 << "\n";
    std::cout << "Delta: " << (w1 - w0) << "\n";
    std::cout << (w1 < w0 ? "[PASS] LTD" : "[FAIL]") << "\n\n";
}

// ============================================================
// 测试 3：时间窗口
// ============================================================
void testTimeWindow() {
    std::cout << "=== Test 3: Time window ===\n\n";
    std::cout << std::setw(10) << "dt(ms)"
              << std::setw(15) << "delta_w"
              << "\n";
    std::cout << std::string(25, '-') << "\n";

    for (double dt : {1.0, 5.0, 10.0, 20.0, 50.0, 100.0}) {
        Synapse s;
        s.weight = 1.0;
        s.nReleaseSites = 0;

        double w0 = s.weight;
        s.onPreSpike(0.0, 1.0);
        s.onPostSpike(dt, 1.0);
        double dw = s.weight - w0;

        std::cout << std::setw(10) << std::fixed << std::setprecision(1)
                  << dt
                  << std::setw(15) << std::setprecision(6) << dw
                  << "\n";
    }
    std::cout << "\nExpected: dw decreases with dt (exponential decay)\n\n";
}

// ============================================================
// 测试 4：多巴胺门控
// ============================================================
void testDopamineGating() {
    std::cout << "=== Test 4: Dopamine gating ===\n\n";
    std::cout << std::setw(12) << "dopamine"
              << std::setw(15) << "delta_w"
              << "\n";
    std::cout << std::string(27, '-') << "\n";

    for (double da : {0.0, 0.3, 0.5, 0.6, 0.7, 0.9, 1.0}) {
        Synapse s;
        s.weight = 1.0;
        s.nReleaseSites = 0;

        double w0 = s.weight;
        s.onPreSpike(0.0, da);
        s.onPostSpike(5.0, da);
        double dw = s.weight - w0;

        std::cout << std::setw(12) << std::fixed << std::setprecision(2)
                  << da
                  << std::setw(15) << std::setprecision(6) << dw
                  << "\n";
    }
    std::cout << "\nExpected:\n";
    std::cout << "  da < 0.6: no change\n";
    std::cout << "  da >= 0.6: increasing LTP\n\n";
}

// ============================================================
// 测试 5：神经元自主 STDP（通过 Network）
// ============================================================
void testAutonomousSTDP() {
    std::cout << "=== Test 5: Autonomous STDP via Network ===\n\n";

    Network net;
    int pre = net.addNeuron(pyramidalType());
    int post = net.addNeuron(pyramidalType());

    int synId = net.connect(pre, post, 0, 0, 1.0, 1.0, true);
    net.synapses.synapses[synId].nReleaseSites = 0;  // 关闭释放
    net.dopamine = 1.0;

    // 验证突触列表已填充
    std::cout << "Neuron " << pre << " outgoing synapses: "
              << net.neurons[pre].outgoingSynapses.size() << "\n";
    std::cout << "Neuron " << post << " incoming synapses: "
              << net.neurons[post].incomingSynapses.size() << "\n\n";

    double w0 = net.getWeight(synId);

    // 手动模拟：pre 放电，然后 post 放电
    // 用回调直接触发（不实际跑 step，因为自发时间不可控）
    double t = 0.0;
    net.synapses.synapses[synId].onPreSpike(t, net.dopamine);
    net.synapses.synapses[synId].onPostSpike(t + 5.0, net.dopamine);

    double w1 = net.getWeight(synId);

    std::cout << "Weight: " << std::fixed << std::setprecision(6)
              << w0 << " -> " << w1 << "\n";
    std::cout << "Delta: " << (w1 - w0) << "\n";
    std::cout << (w1 > w0 ? "[PASS] LTP via Network" : "[FAIL]") << "\n\n";
}

// ============================================================
// 测试 6：实际运行网络，看自发状态下 STDP 是否改变权重
// ============================================================
void testSpontaneousSTDP() {
    std::cout << "=== Test 6: Spontaneous STDP ===\n\n";

    Network net;
    int pre = net.addNeuron(pyramidalType());
    int post = net.addNeuron(pyramidalType());

    int synId = net.connect(pre, post, 0, 0, 1.0, 1.0, true);
    net.dopamine = 1.0;   // 打开 STDP

    double w0 = net.getWeight(synId);

    // 跑 2 秒
    const double dt = 0.05;
    for (int step = 0; step < (int)(2000.0 / dt); ++step) {
        net.step(dt);
    }

    double w1 = net.getWeight(synId);
    double dw = w1 - w0;

    std::cout << "After 2s spontaneous activity:\n";
    std::cout << "  Weight: " << std::fixed << std::setprecision(6)
              << w0 << " -> " << w1 << "\n";
    std::cout << "  Delta: " << dw << "\n";

    if (std::abs(dw) > 1e-6) {
        std::cout << "[PASS] Weight changed by spontaneous activity\n";
    } else {
        std::cout << "[FAIL] Weight unchanged\n";
    }
    std::cout << "\nNote: change direction depends on pre/post spike timing\n\n";
}

int main() {
    std::cout << "=== STDP Tests ===\n\n";

    testCausalLTP();
    testAntiCausalLTD();
    testTimeWindow();
    testDopamineGating();
    testAutonomousSTDP();
    testSpontaneousSTDP();

    std::cout << "=== All Tests Complete ===\n";
    return 0;
}
