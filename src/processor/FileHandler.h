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

class FileHandler
{
public:
    inline static const char LoudnessTag[] = "LKFS";

public:
    FileHandler();
    ~FileHandler();

    bool openFile(juce::File file);
    bool readNextBlock(juce::AudioBuffer<float>* buffer);
    void applyGainDecibel(float gain);
    void writeFile();

    bool hasLoudnessMetadata() { return mHasLoudnessMetadata; }
    void setLoundessMetadata(float loudness);
    unsigned int getNumberOfChannels() { return mFileAttributes.numberOfChannels; }
    double getSampleRate() { return mFileAttributes.sampleRate; }

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
    float mLoudness = 0;
    int mSamplesPerBlock = 0;

    bool mHasFileOpen = false;
};

} // namespace norm