#ifndef NEURONPARAMETERS_H
#define NEURONPARAMETERS_H

struct IzhikevichParams {
    double a, b, c, d;
};

inline IzhikevichParams regularSpiking() {
    return {0.02, 0.2, -65.0, 8.0};
}

inline IzhikevichParams bursting() {
    return {0.02, 0.2, -50.0, 2.0};
}

inline IzhikevichParams fastSpiking() {
    return {0.1, 0.2, -65.0, 2.0};
}

struct GlobalNeuronParams {
    double dt              = 0.5;
    double tau_dendrite    = 20.0;
    double tau_plateau     = 100.0;
    double theta_plateau   = -60.0;
    double A_plateau       = 1.0;
    double tau_calcium     = 80.0;
    double tau_energy      = 5000.0;
    double kappa           = 0.002;
    double tau_novelty     = 5000.0;
    double tau_firing_rate = 100.0;
    double eta_base        = 0.0005;
    double lambda_tag      = 0.3;
    double tau_tag         = 200.0;
    double eta_gain        = 0.0005;
    double theta_gain      = 0.3;
    double gain_min        = 0.1;
    double gain_max        = 5.0;
    double weight_min      = -1.0;
    double weight_max      = 1.0;
    double tau_eta_noise   = 50.0;
    double sigma_eta_noise = 1.0;
    double alpha_spont     = 0.02;
    double gamma_da        = 0.5;

    double weight_decay_rate = 0.00002;
    double ltd_rate          = 0.0003;

    // 生存压力
    double healthDecay    = 0.00005;
    double spikeCost      = 0.001;
    double recoveryReward = 0.005;

    // 慢适应
    bool   adaptationEnabled    = false;
    double tauAdaptation        = 200.0;
    double adaptationIncrement  = 0.005;
    double adaptationGain       = 3.0;

    // 树突输入增益
    double R_input = 50.0;

    // ===== 突触衰减（记忆遗忘）=====
    double synapseDecayActive   = 0.000002;   // 活跃突触衰减率 (每ms)
    double synapseDecayInactive = 0.0002;     // 不活跃突触衰减率 (每ms)
    double synapseInactiveTime  = 3000.0;     // 视为"不活跃"的时间阈值 (ms)
};

#endif
