/*  Tests for the LKFS analyzing unit. Files and target values are provided by
    ITU-R BS.2217. All tests must pass to ensure correct implementation.
*/

#pragma once

#include <gtest/gtest.h>
#include <processor/LKFSProcessor.h>
#include <processor/FileHandler.h>

class LKFSTest : public testing::Test
{
protected:
    norm::FileHandler mFileHandler;
    norm::LKFS mLKFSProcessor;
    const float eps = 0.05f;

    float measureAudioFile(juce::String fileName)
    {
        juce::File file = juce::File(TEST_AUDIO_DIR).getChildFile(fileName);
        EXPECT_TRUE(file.existsAsFile());

        mFileHandler.openFile(file);
        double sampleRate = mFileHandler.getSampleRate();
        int numberOfChannels = (int)(mFileHandler.getNumberOfChannels());
        int samplesPerBlock = (int)(sampleRate * 0.1);

        mLKFSProcessor.reset((float)sampleRate, numberOfChannels);
        juce::AudioBuffer<float> buffer(numberOfChannels, samplesPerBlock);

        while (mFileHandler.readNextBlock(&buffer))
        {
            mLKFSProcessor.processNext100ms(buffer);
        }

        return mLKFSProcessor.getIntegratedLoudness();
    }
};

//==============================================================================

TEST_F(LKFSTest, HomeMade_997Hz_20LKFS)
{
    const float loudness = measureAudioFile(
        "HomeMade_997Hz_20LKFS.wav");
    const float target = -20.f + 20.f * log10(sqrt(2.f));
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_AbsGateTest)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_AbsGateTest.wav");
    const float target = -69.5f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_RelGateTest)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_RelGateTest.wav");
    const float target = -10;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_24LKFS_25Hz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_24LKFS_25Hz_2ch.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_24LKFS_100Hz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_24LKFS_100Hz_2ch.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_24LKFS_500Hz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_24LKFS_500Hz_2ch.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_24LKFS_1kHz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_24LKFS_1000Hz_2ch.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_24LKFS_2kHz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_24LKFS_2000Hz_2ch.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_24LKFS_10kHz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_24LKFS_10000Hz_2ch.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_23LKFS_25Hz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_23LKFS_25Hz_2ch.wav");
    const float target = -23.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_23LKFS_100Hz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_23LKFS_100Hz_2ch.wav");
    const float target = -23.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_23LKFS_500Hz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_23LKFS_500Hz_2ch.wav");
    const float target = -23.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_23LKFS_1kHz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_23LKFS_1000Hz_2ch.wav");
    const float target = -23.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_23LKFS_2kHz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_23LKFS_2000Hz_2ch.wav");
    const float target = -23.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_23LKFS_10kHz_2ch)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_23LKFS_10000Hz_2ch.wav");
    const float target = -23.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Comp_18LKFS_FrequencySweep)
{
    const float loudness = measureAudioFile(
        "1770-2_Comp_18LKFS_FrequencySweep.wav");
    const float target = -18.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Conf_Stereo_VinLR_24LKFS)
{
    const float loudness = measureAudioFile(
        "1770-2 Conf Stereo VinL+R-24LKFS.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Conf_Stereo_VinLR_23LKFS)
{
    const float loudness = measureAudioFile(
        "1770-2 Conf Stereo VinL+R-23LKFS.wav");
    const float target = -23;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Conf_Mono_Voice_Music_24LKFS)
{
    const float loudness = measureAudioFile(
        "1770-2 Conf Mono Voice+Music-24LKFS.wav");
    const float target = -24.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}

TEST_F(LKFSTest, Conf_Mono_Voice_Music_23LKFS)
{
    const float loudness = measureAudioFile(
        "1770-2 Conf Mono Voice+Music-23LKFS.wav");
    const float target = -23.f;
    EXPECT_GT(loudness, target - eps);
    EXPECT_LT(loudness, target + eps);
}
