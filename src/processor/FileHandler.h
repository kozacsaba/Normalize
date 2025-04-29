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

class FileHandler
{
private:
    inline static const char LoudnessTag[] = "LKFS";
    inline static const char SamplePeakTag[] = "PEAK";
    inline static const char Unset_v[] = "Unset";

    enum class Format
    {
        unknown,
        mp3,
        wav,
    };

public:
    /** Opens file, reads audio file properties and caches metadata (if exists).
     *  Throws if opening the file was not successful.
     *  Sets fMeasured flag according to metadata.
     *  The file provided here cannot be changed later. Use a unique_ptr to this
     *  class to make it reusable.
     */
    FileHandler(juce::File file);
    ~FileHandler();

    /** Loads audio from the file that FileHandler holds, and caches metadata.
     *  Returns true if loading was successful.
     *  Sets fAudioLoaded flag.
     */
    bool loadAudio();

    /** Measures loaded audio file. Throws if no audio file is loaded.
     *  Sets fMeasured flag.
     *  Caches loudness and sample peak, but does not write them into metadata.    
     */
    void measure();

    /** Writes file and metada (is exists) into file, with gain_dB 
     *  amplification. Throws of no audio is loaded.
     */
    void writeWithGain(float gain_dB = 0);

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

private:
    bool readNextBlock(juce::AudioBuffer<float>* buffer);
    void setFormat(juce::String format);
    void writeFormatMP3(float gain_dB);
    void writeFormatWav(float gain_dB);

    juce::AudioFormatManager mAudioFormatManager;
    std::unique_ptr<juce::AudioFormatReader> mAudioReader;
    LKFS mProcessor;

    juce::File mFile;
    Format mFormat;
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