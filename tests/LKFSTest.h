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
    float loudness = measureAudioFile("HomeMade_997Hz_20LKFS.wav");
    EXPECT_EQ(loudness, -20);
}

TEST_F(LKFSTest, Comp_AbsGateTest)
{
    float loudness = measureAudioFile("1770-2_Comp_AbsGateTest.wav");
    EXPECT_FLOAT_EQ(loudness, -69.5);
}

TEST_F(LKFSTest, Comp_RelGateTest)
{
    float loudness = measureAudioFile("1770-2_Comp_RelGateTest.wav");
    EXPECT_FLOAT_EQ(loudness, -10);
}
