#pragma once

/*  Should be able to calculate the Integrated LKFS of an audio file. Input
    should be either an audio file or the whole audio in an array or 100ms
    sized audio buffers, haven't decided yet. Output is obviously a float.
    Don't forget to reset / initialise before starting to process a new file.
*/

#include "FilterProcessor.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <util/CircularArray.h>

namespace norm
{

class LKFS
{
public:
    enum class State { ready, in_use, invalid };

public:
    LKFS();
    ~LKFS();

    void reset(float sampleRate, int numberOfChannels);
    void processNext100ms(const juce::AudioBuffer<float>& buffer);
    // Returns integrated loudness in dB. Needs reset after this.
    float getIntegratedLoudness();
    float getSamplePeak();

private:
    void setSampleRate(float sampleRate);
    void setNumberOfChannels(int numberOfChannels);

    int chnum = 0;
    double fs = -1;
    float mLinearAttenuation = 0;
    const float mAbsoluteGate = -70;
    int mExpectedBufferSize = 0;
    float mSamplePeak = 0;

    State mState;

    CircularArray<float> mCircularBuffer;
    std::vector<float> mBlockLoudnessValuesLin;
    std::unique_ptr<KWFilter[]> mFilters; 
};

} // namespace norm

