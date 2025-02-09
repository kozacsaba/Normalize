#pragma once

/*  K-Weighted Filter for one channel. Create multiple instances to work on
    multi-channel audio
*/

#include <cmath>
#include <numbers>

namespace norm
{

namespace defines
{
    static constexpr float pi = std::numbers::pi_v<float>;
}

class KWFilter
{
public:
    KWFilter();
    ~KWFilter();

    void reset(float sampleRate);
    void process(float* data, int size);
    static float getLinearAttenuation() { return att; }

private:
    static void recalibrate();

    inline static double fs = -1.0;
    inline static float att = 0.f;

    // High-Shelf Filter
    inline static float HS_A[2] = {};
    inline static float HS_B[3] = {};
    float Mhs[4] = {};

    // High-Pass Filter
    inline static float HP_A[2] = {};
    inline static float HP_B[3] = {};
    float Mhp[4] = {};

    // High-Shelf Filter Contants
    inline static const float Fhs = 1681.9745f;
    inline static const float Qhs = 0.70717525f;
    inline static const float Vh = pow(10.0f, 3.9998438f / 20.0f);
    inline static float Vb = pow(10.0f, 3.9998438f / 20.0f * 0.49966678f);

    // High-Pass Filter Constants
    inline static float Fhp = 38.13547f;
    inline static float Qhp = 0.50032705f;
};

} // namespace norm
