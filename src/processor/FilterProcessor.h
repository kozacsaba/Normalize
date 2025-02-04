#pragma once

/*  K-Weighted Filter for one channel. Create multiple instances to work on
    multi-channel audio
*/

#include <cmath>
#include "util/Defines.h"

namespace norm
{

class KWFilter
{
public:
    KWFilter();
    ~KWFilter();

    void reset(float sampleRate);
    void process(float* data, int size);

private:
    void recalibrate();

    double fs = -1.0;
    global att = 0.f;

    // High-Shelf Filter
    global HS_A[2] = {};
    global HS_B[3] = {};
    float Mhs[4] = {};

    // High-Pass Filter
    global HP_A[2] = {};
    global HP_B[3] = {};
    float Mhp[4] = {};

    // High-Shelf Filter Contants
    globalc Fhs = 1681.9745f;
    globalc Qhs = 0.70717525f;
    globalc Vh = pow(10.0f, 3.9998438f / 20.0f);
    globalc Vb = pow(10.0f, 3.9998438f / 20.0f * 0.49966678f);

    // High-Pass Filter Constants
    globalc Fhp = 38.13547f;
    globalc Qhp = 0.50032705f;
    
};

} // namespace norm
