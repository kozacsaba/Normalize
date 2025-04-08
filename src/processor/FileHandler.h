#pragma once

/*  The purpose of this class is to handle ONE audio file, including reading it
    in buffers, managing metadata, applying constant gain and so on. It must be
    reusable, but does not need to know anything about the file system and does
    no calculations whatsoever.
*/

#include <memory>
#include <optional>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include "LKFSProcessor.h"

namespace norm
{

// TODO - cq/#17
/*
    Most of the functions of this class should be private, and only funcitonally
    complete funcitons should be visible from the outside, to enforce
    encapsulation. Anything that could leave this class in an incomplete state,
    or a file in a half-processed state, should be a private function, not
    accessible from the outside.
    The process should also be separated into a measuring process and a gain
    process, because they might need to be done separately.    
*/

class FileHandler
{
public:
    inline static const char LoudnessTag[] = "LKFS";
    inline static const char SamplePeakTag[] = "PEAK";
    inline static const char Unset_v[] = "Unset";

public:
    FileHandler(juce::File file);
    ~FileHandler();

    bool loadAudio();
    void measure();
    void applyGainDecibel(float gain);
    void writeFile();

    bool isMeasured() const { return fMeasured; }
    std::optional<float> getLoudness() const 
    {
        if (fMeasured) return mLoudness;
        else return std::nullopt;
    }
    std::optional<float> getSamplePeak() const
    {
        if (fMeasured) return mPeak;
        else return std::nullopt;
    }

    // unsigned int getNumberOfChannels() const { return mFileAttributes.numberOfChannels; }
    // double getSampleRate() const { return mFileAttributes.sampleRate; }

private:
    bool readNextBlock(juce::AudioBuffer<float>* buffer);

    juce::AudioFormatManager mAudioFormatManager;
    std::unique_ptr<juce::AudioFormatReader> mAudioReader;
    LKFS mProcessor;

    juce::File mFile;
    juce::AudioBuffer<float> mBuffer;
    juce::AudioBuffer<float> mWorkBuffer;
    juce::int64 mPlayhead = 0;

    struct {
        juce::StringPairArray metadata;
        unsigned int numberOfChannels = 0;
        juce::int64 length = 0;
        double sampleRate = 0;
        int qualityOptionIndex = 0;
    } mFileAttributes;

    bool fMeasured = false;
    bool fAudioLoaded = false;

    float mLoudness = 0;
    float mPeak = 0;
    int mSamplesPerBlock = 0;
};

} // namespace norm