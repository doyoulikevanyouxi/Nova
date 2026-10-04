#include "../core/Network.h"
#include "../core/NetworkGenome.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "=== Genome Test ===\n\n";

    Network net;
    NetworkGenome genome = defaultVisualGenome();

    std::cout << "Building network from genome...\n";
    net.buildFromGenome(genome);

    std::cout << "Done.\n\n";

    // ===== 打印区域 =====
    std::cout << std::setw(8) << "Region"
              << std::setw(10) << "Layer"
              << std::setw(10) << "Neurons"
              << "\n";
    std::cout << std::string(28, '-') << "\n";
    for (auto& [name, region] : net.regions) {
        std::cout << std::setw(8) << name
                  << std::setw(10) << region.layer
                  << std::setw(10) << region.size()
                  << "\n";
    }

    std::cout << "\nTotal neurons: " << net.neuronCount() << "\n";
    std::cout << "Total synapses: " << net.synapseCount() << "\n\n";

    // ===== 统计突触类型 =====
    int intra = 0, inter = 0;
    for (auto& s : net.synapses.synapses) {
        // 判断是否是区域间
        bool sameRegion = false;
        for (auto& [name, region] : net.regions) {
            bool srcIn = false, tgtIn = false;
            for (int id : region.neurons) {
                if (id == s.preId) srcIn = true;
                if (id == s.postId) tgtIn = true;
            }
            if (srcIn && tgtIn) { sameRegion = true; break; }
        }
        if (sameRegion) intra++;
        else inter++;
    }

    std::cout << "Intra-region synapses: " << intra << "\n";
    std::cout << "Inter-region synapses: " << inter << "\n\n";

    // ===== 检查 V1→V2 =====
    int v1v2 = 0;
    auto* v1 = net.getRegion("V1");
    auto* v2 = net.getRegion("V2");
    if (v1 && v2) {
        for (auto& s : net.synapses.synapses) {
            bool srcInV1 = false, tgtInV2 = false;
            for (int id : v1->neurons) if (id == s.preId) srcInV1 = true;
            for (int id : v2->neurons) if (id == s.postId) tgtInV2 = true;
            if (srcInV1 && tgtInV2) v1v2++;
        }
    }
    std::cout << "V1 -> V2 synapses: " << v1v2 << "\n";

    // V2 → V4
    int v2v4Count = 0;
    auto* v4 = net.getRegion("V4");
    if (v2 && v4) {
        for (auto& s : net.synapses.synapses) {
            bool srcInV2 = false, tgtInV4 = false;
            for (int id : v2->neurons) if (id == s.preId) srcInV2 = true;
            for (int id : v4->neurons) if (id == s.postId) tgtInV4 = true;
            if (srcInV2 && tgtInV4) v2v4Count++;
        }
    }
    std::cout << "V2 -> V4 synapses: " << v2v4Count << " (expected 4)\n";



    // ===== 检查 V2→IT =====
    int v2it = 0;
    auto* it = net.getRegion("IT");
    // V4 → IT
    int v4itCount = 0;
    if (v4 && it) {
        for (auto& s : net.synapses.synapses) {
            bool srcInV4 = false, tgtInIT = false;
            for (int id : v4->neurons) if (id == s.preId) srcInV4 = true;
            for (int id : it->neurons) if (id == s.postId) tgtInIT = true;
            if (srcInV4 && tgtInIT) v4itCount++;
        }
    }
    std::cout << "V4 -> IT synapses: " << v4itCount << " (expected 10)\n";

    if (v2 && it) {
        for (auto& s : net.synapses.synapses) {
            bool srcInV2 = false, tgtInIT = false;
            for (int id : v2->neurons) if (id == s.preId) srcInV2 = true;
            for (int id : it->neurons) if (id == s.postId) tgtInIT = true;
            if (srcInV2 && tgtInIT) v2it++;
        }
    }
    std::cout << "V2 -> IT synapses: " << v2it << "\n";

    // IT → H
    int itHCount = 0;
    auto* h = net.getRegion("H");
    if (it && h) {
        for (auto& s : net.synapses.synapses) {
            bool srcInIT = false, tgtInH = false;
            for (int id : it->neurons) if (id == s.preId) srcInIT = true;
            for (int id : h->neurons) if (id == s.postId) tgtInH = true;
            if (srcInIT && tgtInH) itHCount++;
        }
    }
    std::cout << "IT -> H synapses: " << itHCount << " (expected 20)\n";

    // ===== 检查 IT→V4（反馈）=====
    int itv4 = 0;
    if (it && v4) {
        for (auto& s : net.synapses.synapses) {
            bool srcInIT = false, tgtInV4 = false;
            for (int id : it->neurons) if (id == s.preId) srcInIT = true;
            for (int id : v4->neurons) if (id == s.postId) tgtInV4 = true;
            if (srcInIT && tgtInV4) itv4++;
        }
    }
    std::cout << "IT -> V4 synapses: " << itv4 << " (expected 3)\n";

    // ===== 检查 V4→V2（反馈）=====
    int v4v2 = 0;
    if (v4 && v2) {
        for (auto& s : net.synapses.synapses) {
            bool srcInV4 = false, tgtInV2 = false;
            for (int id : v4->neurons) if (id == s.preId) srcInV4 = true;
            for (int id : v2->neurons) if (id == s.postId) tgtInV2 = true;
            if (srcInV4 && tgtInV2) v4v2++;
        }
    }
    std::cout << "V4 -> V2 synapses: " << v4v2 << " (expected 3)\n";

    // ===== 检查 V2→V1（反馈）=====
    int v2v1 = 0;
    if (v2 && v1) {
        for (auto& s : net.synapses.synapses) {
            bool srcInV2 = false, tgtInV1 = false;
            for (int id : v2->neurons) if (id == s.preId) srcInV2 = true;
            for (int id : v1->neurons) if (id == s.postId) tgtInV1 = true;
            if (srcInV2 && tgtInV1) v2v1++;
        }
    }
    std::cout << "V2 -> V1 synapses: " << v2v1 << " (expected 4)\n";

    // ===== 检查 H→IT（反馈）=====
    int hit = 0;
    if (h && it) {
        for (auto& s : net.synapses.synapses) {
            bool srcInH = false, tgtInIT = false;
            for (int id : h->neurons) if (id == s.preId) srcInH = true;
            for (int id : it->neurons) if (id == s.postId) tgtInIT = true;
            if (srcInH && tgtInIT) hit++;
        }
    }
    std::cout << "H -> IT synapses: " << hit << " (expected 8)\n";



    // ===== 跑 500ms =====
    std::cout << "Running 500ms...\n";
    const double dt = 0.05;
    for (int step = 0; step < (int)(500.0 / dt); ++step) {
        net.step(dt);
    }
    std::cout << "Done.\n\n";

    // ===== 判定 =====
    bool pass = true;

    if (net.regions.size() == 5) {
        std::cout << "[PASS] 5 regions\n";
    } else { pass = false; }

    if (net.neuronCount() == 72) {
        std::cout << "[PASS] 72 neurons (7+7+5+13+40)\n";
    } else {
        std::cout << "[FAIL] Expected 72 neurons, got "
                  << net.neuronCount() << "\n";
        pass = false;
    }

    if (v1v2 == 6) {
        std::cout << "[PASS] V1->V2 has 6 synapses (explicit mapping)\n";
    } else { pass = false; }

    if (v2it == 8) {
        std::cout << "[PASS] V2->IT has 8 synapses (explicit mapping)\n";
    } else { pass = false; }

    if (itv4 == 3) {
        std::cout << "[PASS] IT->V4 has 3 synapses (feedback)\n";
    } else { pass = false; }

    if (v4v2 == 3) {
        std::cout << "[PASS] V4->V2 has 3 synapses (feedback)\n";
    } else { pass = false; }

    if (v2v1 == 4) {
        std::cout << "[PASS] V2->V1 has 4 synapses (feedback)\n";
    } else { pass = false; }

    std::cout << "\n=== " << (pass ? "GENOME WORKS" : "GENOME FAILED")
              << " ===\n";

    return pass ? 0 : 1;
}
