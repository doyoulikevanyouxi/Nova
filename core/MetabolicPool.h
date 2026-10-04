#ifndef METABOLIC_POOL_H
#define METABOLIC_POOL_H

#include <algorithm>

// ============================================================
// MetabolicPool - 代谢池
// ============================================================
//
// 模拟全局血糖水平。
// 神经元放电消耗能量，恢复机制补充能量。
//
// 参数：
//   basalConsumption：基础消耗
//   spikeConsumption：每次放电的额外消耗
//   recoveryRate：恢复速率
//
// ============================================================

struct MetabolicPool {
    double glucose = 1.0;              // 血糖水平（0-1）
    double basalConsumption = 0.00005; // 基础消耗
    double spikeConsumption = 0.000005;// 每次放电消耗
    double recoveryRate = 0.0002;      // 恢复速率

    double satiety = 0.0;              // 饱腹感（变化率）
    double prevGlucose = 1.0;

    // ===== 更新 =====
    // totalSpikes: 上一步总放电数
    // externalFood: 外部食物注入
    // dt: 时间步
    void update(int totalSpikes, double externalFood, double dt) {
        double consumption = basalConsumption + spikeConsumption * totalSpikes;
        glucose -= consumption * dt;
        glucose += recoveryRate * (1.0 - glucose) * dt;
        glucose += externalFood * dt;

        if (glucose < 0.0) glucose = 0.0;
        if (glucose > 1.0) glucose = 1.0;

        satiety = (glucose - prevGlucose) / dt;
        if (satiety > 1.0) satiety = 1.0;
        if (satiety < -1.0) satiety = -1.0;

        prevGlucose = glucose;
    }

    double getHungerLevel() const {
        return 1.0 - glucose;
    }

    double getDopamineBoost() const {
        return std::max(0.0, satiety) * 0.5;
    }
};

#endif
