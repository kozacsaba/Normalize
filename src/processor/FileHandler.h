#pragma once

/*  The purpose of this class is to handle ONE audio file, including reading it
    in buffers, managing metadata, applying constant gain and so on. It must be
    reusable, but does not need to know anything about the file system and does
    no calculations whatsoever.
*/

#include <memory>
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

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
    FileHandler();
    ~FileHandler();

    bool openFile(juce::File file);
    bool readNextBlock(juce::AudioBuffer<float>* buffer);
    void applyGainDecibel(float gain);
    void writeFile();

    bool hasLoudnessMetadata() const { return mHasLoudnessMetadata; }
    void setLoundessMetadata(float loudness);
    float getLoudnessMetadata() const;

    bool hasSamplePeakMetadata() const { return mHasSamplePeakMetadata; }
    void setSamplePeakMetadata(float peak);
    float getSamplePeakMetadata() const;

    unsigned int getNumberOfChannels() const { return mFileAttributes.numberOfChannels; }
    double getSampleRate() const { return mFileAttributes.sampleRate; }

private:
    juce::AudioFormatManager mAudioFormatManager;
    std::unique_ptr<juce::AudioFormatReader> mAudioReader;

    juce::File mFile;
    juce::AudioBuffer<float> mBuffer;
    juce::int64 mPlayhead;

    struct {
        juce::StringPairArray metadata;
        unsigned int numberOfChannels = 0;
        juce::int64 length = 0;
        double sampleRate = 0;
        int qualityOptionIndex = 0;
    } mFileAttributes;

    bool mHasLoudnessMetadata = false;
    bool mHasSamplePeakMetadata = false;
    float mLoudness = 0;
    float mPeak = 0;
    int mSamplesPerBlock = 0;

    bool mHasFileOpen = false;
};

} // namespace norm