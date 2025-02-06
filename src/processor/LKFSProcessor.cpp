#include "LKFSProcessor.h"
#include <util/Logger.h>
#include <juce_core/juce_core.h>

namespace norm
{

LKFS::LKFS() 
    : mState(State::invalid)
    , mCircularBuffer(4)
{}
LKFS::~LKFS() {}

void LKFS::reset(float sampleRate, int numberOfChannels)
{
    setNumberOfChannels(numberOfChannels);
    setSampleRate(sampleRate);

    mSamplePeak = 0;

    mBlockLoudnessValues.clear();
    mCircularBuffer.reset();

    mState = State::ready;
}
void LKFS::processNext100ms(const juce::AudioBuffer<float>& buffer)
{
    EXPECT_OR_RETURN (mState == State::invalid,
                      void(), 
                      "You need to reset the LKFS Processor before use.");

    int incomingBufferSize = buffer.getNumSamples();

    EXPECT_OR_THROW (incomingBufferSize != mExpectedBufferSize,
                     std::exception{},
                     "Wrong buffer size fed into LKFS unit");

    //==========================================================================

    juce::AudioBuffer<float> workBuffer = buffer;

    float localMax = workBuffer.getMagnitude(0, mExpectedBufferSize);
    mSamplePeak = juce::jmax(mSamplePeak, localMax);

    float blockValue = 0;
    for (int ch = 0; ch < workBuffer.getNumChannels(); ch++)
    {
        auto pChannelData = workBuffer.getWritePointer(ch);
        mFilters[ch].process(pChannelData, mExpectedBufferSize);

        for (int s = 0; s < workBuffer.getNumSamples(); s++)
        {
            blockValue += pChannelData[s] * pChannelData[s];
        }
    }

    mCircularBuffer.push(blockValue);
    float momentary = mCircularBuffer.getSum();
    momentary = momentary / 0.4f / (float)fs;
    momentary = 10.f * std::log10(momentary) - mAttenuation;

    if (momentary > mAbsoluteGate)
    {
        mBlockLoudnessValues.emplace_back(momentary);
    }

    mState = State::in_use;
}
float LKFS::getIntegratedLoudness()
{
    EXPECT_OR_THROW (mState == State::in_use && mBlockLoudnessValues.size() > 3,
                     std::exception{},
                     "You need to feed the processor some audio before querying loudness");

    // The first 3 entries are actually invalid and should be discarded, because
    // according to ITU-R BS.1770, the first window measured should be one
    // completely filled with data. In case of 75% overlap, that window is the
    // 4th one.
    mBlockLoudnessValues.erase (mBlockLoudnessValues.begin(), 
                                mBlockLoudnessValues.begin() + 2);

    float blockValueSum = 0;
    for (const auto& blockLoudness : mBlockLoudnessValues)
    {
        blockValueSum += blockLoudness;
    }
    float blockValueAverage = blockValueSum / mBlockLoudnessValues.size();
    float blockAverageDB = 10.0f * log10(blockValueAverage) - mAttenuation;

    float relativeGate = juce::jmax(blockAverageDB, -70.0f);
    float gatedSum = 0;
    int gatedValueCount = 0;

    for (const auto& blockLoudness : mBlockLoudnessValues)
    {
        if (blockLoudness > relativeGate)
        {
            gatedSum += blockLoudness;
            gatedValueCount++;
        }
    }

    mState = State::invalid;

    float gatedAverage = gatedSum / (float)gatedValueCount;
    float integratedLoudness = 10.f * log10(gatedAverage) - mAttenuation;
    return integratedLoudness;
}
float LKFS::getSamplePeak()
{
    return mSamplePeak;
}

void LKFS::setSampleRate(float sampleRate)
{
    EXPECT_OR_THROW (sampleRate > 0,
                     std::exception{},
                     "Sample Rate is {}", sampleRate);

    fs = sampleRate;
    for (int ch = 0; ch < chnum; ch++)
    {
        mFilters[ch].reset(fs);
    }

    mExpectedBufferSize = (int)(fs / 10);

    mAttenuation = KWFilter::getAttenuation();
    mAbsoluteGate = pow(10.f, -7.f + mAttenuation / 10.f);
}
void LKFS::setNumberOfChannels(int numberOfChannels)
{
    EXPECT_OR_THROW (numberOfChannels > 0,
                     std::exception{},
                     "Number of channels is {}", numberOfChannels);

    if (chnum != numberOfChannels)
    {
        chnum = numberOfChannels;
        mFilters = std::make_unique<KWFilter[]>(chnum);
    }
}

} // namespace norm