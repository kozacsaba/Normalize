#include "FilterProcessor.h"
#include <juce_core/juce_core.h>
#include <complex>

namespace norm
{
    KWFilter::KWFilter() {}
    KWFilter::~KWFilter() {}

    void KWFilter::reset(double sampleRate)
    {
        for (int i = 0; i < 4; i++)
            {
                Mhp[i] = 0;
                Mhs[i] = 0;
            }

        // float comparison
        const bool sampleRateActuallyChanged = 
            std::fabs(fs - sampleRate) > 1.f;
        if (sampleRateActuallyChanged)
        {
            fs = sampleRate;
            recalibrate();
        }
    }
    void KWFilter::process(float* data, int size)
    {
        jassert(fs > 0);

        for (int s = 0; s < size; s++)
        {
            // high-shelf filtering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            float u = data[s];
            float u1 = Mhs[0];
            float u2 = Mhs[1];
            float y1 = Mhs[2];
            float y2 = Mhs[3];

            data[s] =
                HS_B[0] * u + HS_B[1] * u1 + HS_B[2] * u2 -
                HS_A[0] * y1 - HS_A[1] * y2;

            Mhs[0] = u;
            Mhs[1] = u1;
            Mhs[2] = data[s];
            Mhs[3] = y1;

            // high-pass filtering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            u = data[s];
            u1 = Mhp[0];
            u2 = Mhp[1];
            y1 = Mhp[2];
            y2 = Mhp[3];

            data[s] =
                HP_B[0] * u + HP_B[1] * u1 + HP_B[2] * u2 -
                HP_A[0] * y1 - HP_A[1] * y2;

            Mhp[0] = u;
            Mhp[1] = u1;
            Mhp[2] = data[s];
            Mhp[3] = y1;
        }
    }

    void KWFilter::recalibrate()
    {
        jassert(fs > 0);

        // high-shelf coeffs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        float Khs = tan(defines::pi * Fhs / (float)fs);
        float khs2 = Khs * Khs;
        float a0 = 1.0f + Khs / Qhs + khs2;

        HS_B[0] = (Vh + Vb * Khs / Qhs + khs2) / a0;
        HS_B[1] = 2.0f * (khs2 - Vh) / a0;
        HS_B[2] = (Vh - Vb * Khs / Qhs + khs2) / a0;
        HS_A[0] = 2.0f * (khs2 - 1.0f) / a0;
        HS_A[1] = (1.0f - Khs / Qhs + khs2) / a0;

        // high-pass coeffs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        float Khp = tan(defines::pi * Fhp / (float)fs);
        float khp2 = Khp * Khp;

        HP_B[0] = 1.f;
        HP_B[1] = -2.f;
        HP_B[2] = 1.f;
        HP_A[0] = 2.f * (khp2 - 1.f) / (1.f + Khp / Qhp + khp2);
        HP_A[1] = (1.f - Khp / Qhp + khp2) / (1.f + Khp / Qhp + khp2);

        // attenuation @ 997Hz ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        using c = std::complex<float>;
        c j(0.f, 1.f);
        c z = std::exp(c(997.f * 2.f * defines::pi / (float)fs) * -j);

        c h =  c(HS_B[0], 0) * z * z + c(HS_B[1], 0) * z + c(HS_B[2], 0);
        h  *= (c(HP_B[0], 0) * z * z + c(HP_B[1], 0) * z + c(HP_B[2], 0));
        h  /= (c(1.0, 0)     * z * z + c(HS_A[0], 0) * z + c(HS_A[1], 0));
        h  /= (c(1.0, 0)     * z * z + c(HP_A[0], 0) * z + c(HP_A[1], 0));

        att = std::abs(h);
    }
}

#undef pi