#include "../core/DendriteCluster.h"
#include "../core/DendriteBranch.h"
#include "../core/NeuronType.h"
#include <iostream>
#include <iomanip>
#include <vector>

// ============================================================
// 创建一个锥体细胞的树突结构
// 2 簇（基树突 + 顶树突），每簇 2 分支
// ============================================================
void buildPyramidalDendrites(
    std::vector<DendriteBranch>& branches,
    std::vector<DendriteCluster>& clusters) {

    NeuronType type = pyramidalType();

    // 创建 4 个分支（每个分支 2 个输入通道）
    for (int i = 0; i < 4; ++i) {
        DendriteBranch b(2);
        //b.R_input = type.defaultR_input;
        branches.push_back(b);
    }

    // 簇 0：基树突（下方）
    DendriteCluster basal(0, ClusterType::Basal);
    basal.x = 0.0;
    basal.y = -5.0;          // 在胞体下方 5 单位
    basal.diameter = 1.0;
    basal.addBranch(0);
    basal.addBranch(1);
    clusters.push_back(basal);

    // 分支 0、1 相对基簇的位置
    branches[0].x = -3.0;
    branches[0].y =  0.0;
    branches[1].x =  3.0;
    branches[1].y =  0.0;

    // 簇 1：顶树突（上方）
    DendriteCluster apical(1, ClusterType::Apical);
    apical.x = 0.0;
    apical.y = 5.0;          // 在胞体上方 5 单位
    apical.diameter = 1.0;
    apical.addBranch(2);
    apical.addBranch(3);
    clusters.push_back(apical);

    // 分支 2、3 相对顶簇的位置
    branches[2].x = -3.0;
    branches[2].y =  0.0;
    branches[3].x =  3.0;
    branches[3].y =  0.0;
}

int main() {
    std::cout << "=== Dendrite Cluster Test ===\n\n";

    // 构建锥体细胞的树突结构
    std::vector<DendriteBranch> branches;
    std::vector<DendriteCluster> clusters;
    buildPyramidalDendrites(branches, clusters);

    std::cout << "Structure:\n";
    std::cout << "  Branches: " << branches.size() << "\n";
    std::cout << "  Clusters: " << clusters.size() << "\n\n";

    for (auto& c : clusters) {
        std::cout << "  Cluster " << c.id
                  << " (" << (c.type == ClusterType::Basal ? "Basal" : "Apical") << ")"
                  << ": " << c.branchIndices.size() << " branches"
                  << ", pos=(" << c.x << "," << c.y << ")"
                  << ", attToSoma=" << std::fixed << std::setprecision(3)
                  << c.attenuationToSoma()
                  << "\n";
    }
    std::cout << "\n";

    // ===== 测试 1：只激活基树突 =====
    std::cout << "=== Test 1: Only Basal Active ===\n\n";

    const double dt = 0.1;
    const int totalSteps = 500;   // 50ms

    for (int step = 0; step < totalSteps; ++step) {
        // 给基树突分支强输入
        branches[0].inputs[0] = 1.0;
        branches[1].inputs[0] = 1.0;

        // 更新分支
        for (auto& b : branches) b.update(dt);

        // 更新簇
        for (auto& c : clusters) c.update(branches, dt);
    }

    std::cout << std::setw(10) << "Cluster"
              << std::setw(15) << "d(mV)"
              << std::setw(15) << "p(mV)"
              << std::setw(15) << "output"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    for (auto& c : clusters) {
        std::cout << std::setw(10) << c.id
                  << std::setw(15) << std::fixed << std::setprecision(3) << c.d
                  << std::setw(15) << c.p
                  << std::setw(15) << c.outputToSoma()
                  << "\n";
    }

    std::cout << "\nExpected: Basal cluster active, Apical silent\n\n";

    // ===== 测试 2：只激活顶树突 =====
    std::cout << "=== Test 2: Only Apical Active ===\n\n";

    // 重置
    for (auto& b : branches) { b.d = -65.0; b.p = 0.0; b.clearInputs(); }
    for (auto& c : clusters) { c.d = -65.0; c.p = 0.0; }

    for (int step = 0; step < totalSteps; ++step) {
        // 给顶树突分支强输入
        branches[2].inputs[0] = 1.0;
        branches[3].inputs[0] = 1.0;

        for (auto& b : branches) b.update(dt);
        for (auto& c : clusters) c.update(branches, dt);
    }

    std::cout << std::setw(10) << "Cluster"
              << std::setw(15) << "d(mV)"
              << std::setw(15) << "p(mV)"
              << std::setw(15) << "output"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    for (auto& c : clusters) {
        std::cout << std::setw(10) << c.id
                  << std::setw(15) << std::fixed << std::setprecision(3) << c.d
                  << std::setw(15) << c.p
                  << std::setw(15) << c.outputToSoma()
                  << "\n";
    }

    std::cout << "\nExpected: Apical cluster active, Basal silent\n\n";

    // ===== 测试 3：两者同时激活 =====
    std::cout << "=== Test 3: Both Active ===\n\n";

    for (auto& b : branches) { b.d = -65.0; b.p = 0.0; b.clearInputs(); }
    for (auto& c : clusters) { c.d = -65.0; c.p = 0.0; }

    for (int step = 0; step < totalSteps; ++step) {
        branches[0].inputs[0] = 1.0;
        branches[1].inputs[0] = 1.0;
        branches[2].inputs[0] = 1.0;
        branches[3].inputs[0] = 1.0;

        for (auto& b : branches) b.update(dt);
        for (auto& c : clusters) c.update(branches, dt);
    }

    std::cout << std::setw(10) << "Cluster"
              << std::setw(15) << "d(mV)"
              << std::setw(15) << "p(mV)"
              << std::setw(15) << "output"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    double totalOutput = 0.0;
    for (auto& c : clusters) {
        totalOutput += c.outputToSoma();
        std::cout << std::setw(10) << c.id
                  << std::setw(15) << std::fixed << std::setprecision(3) << c.d
                  << std::setw(15) << c.p
                  << std::setw(15) << c.outputToSoma()
                  << "\n";
    }

    std::cout << "\nTotal output to soma: " << totalOutput << "\n";
    std::cout << "Expected: Both clusters active, total > either alone\n";

    std::cout << "\n=== All Tests Complete ===\n";
    return 0;
}
