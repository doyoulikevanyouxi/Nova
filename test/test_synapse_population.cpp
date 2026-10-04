#include "../core/SynapsePopulation.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

// ============================================================
// 模拟一个最小神经元：只有 spike 发射
// ============================================================
struct MockNeuron {
    int id;
    bool hasSpiked = false;
    double current = 0.0;           // 接收到的总电流
    std::vector<std::pair<int,double>> receivedAmounts;  // 记录每次接收

    void fire() { hasSpiked = true; }
    void clearSpike() { hasSpiked = false; }

    void receive(int branch, int input, double amount) {
        current += amount;
        receivedAmounts.push_back({input, amount});
    }
};

int main() {
    std::cout << "=== Synapse Population Test ===\n\n";

    // ===== 创建两个神经元 =====
    MockNeuron pre;  pre.id = 0;
    MockNeuron post; post.id = 1;

    // ===== 创建突触群 =====
    SynapsePopulation pop;

    // 添加 3 个突触：
    //   突触 0: pre -> post, delay=1ms, weight=1.0
    //   突触 1: pre -> post, delay=2ms, weight=0.5
    //   突触 2: pre -> post, delay=5ms, weight=2.0

    auto makeSyn = [](int pre, int post, double delay,
                      double weight, int branch, int input) {
        Synapse s;
        s.preId = pre;
        s.postId = post;
        s.delayMs = delay;
        s.weight = weight;
        s.postBranch = branch;
        s.postInput = input;
        s.nReleaseSites = 5;
        s.releaseProb = 0.9;   // 从 0.5 改为 0.9
        s.quantalSize = 0.5;
        return s;
    };

    pop.addSynapse(makeSyn(0, 1, 1.0, 1.0, 0, 0));
    pop.addSynapse(makeSyn(0, 1, 2.0, 0.5, 0, 1));
    pop.addSynapse(makeSyn(0, 1, 5.0, 2.0, 0, 2));

    std::cout << "Created " << pop.size() << " synapses\n\n";

    // ===== 投递回调 =====
    auto deliver = [&post](int postId, int branch, int input, double amount) {
        if (postId == post.id) {
            post.receive(branch, input, amount);
        }
    };

    // ===== 模拟 =====
    const double dt = 0.01;   // ms
    const double duration = 20.0;
    const int totalSteps = (int)(duration / dt);

    std::cout << "Simulation: pre fires at t=0\n";
    std::cout << "Expected deliveries at t=1ms, 2ms, 5ms\n\n";

    std::cout << std::setw(10) << "time(ms)"
              << std::setw(15) << "post.current"
              << std::setw(15) << "received"
              << "\n";
    std::cout << std::string(40, '-') << "\n";

    // t=0 时放电
    pre.fire();

    for (int step = 0; step < totalSteps; ++step) {
        double t = step * dt;

        // 突触前放电通知
        if (pre.hasSpiked) {
            pop.onPresynapticSpike(pre.id, t);
            pre.clearSpike();
        }

        // 投递到达的释放
        pop.step(t, deliver);

        // 打印关键时刻
        if (step % 100 == 0 ||
            (t > 0.9 && t < 1.1) ||
            (t > 1.9 && t < 2.1) ||
            (t > 4.9 && t < 5.1)) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(2)
            << t
            << std::setw(15) << std::setprecision(4) << post.current
            << std::setw(15) << post.receivedAmounts.size()
            << "\n";
        }
    }

    // ===== 分析结果 =====
    std::cout << "\n=== Analysis ===\n\n";

    std::cout << "Total deliveries: " << post.receivedAmounts.size() << "\n";
    std::cout << "Total current: " << std::fixed << std::setprecision(4)
              << post.current << "\n\n";

    // 每个突触的期望释放量
    double expected1 = 5 * 0.9 * 0.5 * 1.0;   // 2.25
    double expected2 = 5 * 0.9 * 0.5 * 0.5;   // 1.125
    double expected3 = 5 * 0.9 * 0.5 * 2.0;   // 4.5
    double totalExpected = expected1 + expected2 + expected3;  // 7.875

    std::cout << "Expected release amounts:\n";
    std::cout << "  Synapse 0 (delay=1ms, w=1.0): " << expected1 << "\n";
    std::cout << "  Synapse 1 (delay=2ms, w=0.5): " << expected2 << "\n";
    std::cout << "  Synapse 2 (delay=5ms, w=2.0): " << expected3 << "\n";
    std::cout << "  Total: " << totalExpected << "\n\n";

    // ===== 验证 =====
    bool pass = true;

    // 检查是否收到 3 次投递
    if (post.receivedAmounts.size() == 3) {
        std::cout << "[PASS] Received 3 deliveries (one per synapse)\n";
    } else {
        std::cout << "[FAIL] Expected 3 deliveries, got "
                  << post.receivedAmounts.size() << "\n";
        pass = false;
    }

    // 检查总电流是否接近期望
    double error = std::abs(post.current - totalExpected) / totalExpected;
    if (error < 0.3) {   // 量子释放有随机性，放宽到 30%
        std::cout << "[PASS] Total current close to expected (error = "
                  << std::setprecision(1) << (error * 100) << "%)\n";
    } else {
        std::cout << "[FAIL] Total current far from expected\n";
        pass = false;
    }

    // 检查延迟队列已清空
    if (pop.pendingCount() == 0) {
        std::cout << "[PASS] Pending queue empty\n";
    } else {
        std::cout << "[FAIL] Pending queue has "
                  << pop.pendingCount() << " items\n";
        pass = false;
    }

    std::cout << "\n=== " << (pass ? "ALL TESTS PASSED" : "TESTS FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
