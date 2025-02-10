#pragma once

#include <gtest/gtest.h>
#include <processor/FilterProcessor.h>

class FilterTest : public testing::Test
{
protected:
    norm::KWFilter mFilter;
    const float eps = 0.0001f;

    void hsCoeffsTest()
    {
        mFilter.reset(fs);

        EXPECT_FLOAT_EQ(mFilter.HS_A[0], HS_A[0]);
        EXPECT_FLOAT_EQ(mFilter.HS_A[1], HS_A[1]);

        EXPECT_FLOAT_EQ(mFilter.HS_B[0], HS_B[0]);
        EXPECT_FLOAT_EQ(mFilter.HS_B[1], HS_B[1]);
        EXPECT_FLOAT_EQ(mFilter.HS_B[2], HS_B[2]);

        EXPECT_FLOAT_EQ(mFilter.HP_A[0], HP_A[0]);
        EXPECT_FLOAT_EQ(mFilter.HP_A[1], HP_A[1]);
        
        EXPECT_FLOAT_EQ(mFilter.HP_B[0], HP_B[0]);
        EXPECT_FLOAT_EQ(mFilter.HP_B[1], HP_B[1]);
        EXPECT_FLOAT_EQ(mFilter.HP_B[2], HP_B[2]);
    }

    void attTest()
    {
        mFilter.reset(fs);

        const float filterAtt = mFilter.getLinearAttenuation();
        const float filterAttDB = 20.f * log10(filterAtt);
        EXPECT_GT(filterAttDB, att - eps);
        EXPECT_LT(filterAttDB, att + eps);
    }

private:
    const float fs = 48000.f;

    const float HS_A[2] = {-1.69065929318241f, 
                            0.73248077421585f};
    const float HS_B[3] = { 1.53512485958697f,
                           -2.69169618940638f,
                            1.19839281085285f};
    const float HP_A[2] = {-1.99004745483398f,
                            0.99007225036621f};
    const float HP_B[3] = { 1.0f,
                           -2.0f,
                            1.0f};

    const float att = 0.691f;
};

TEST_F(FilterTest, CoeffTest)
{
    hsCoeffsTest();
}

TEST_F(FilterTest, AttenuationTest)
{
    attTest();
}
