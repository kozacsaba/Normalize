#include "FileHandler.h"
#include "util/Logger.h"

namespace norm
{
    FileHandler::FileHandler()
        : mHasFileOpen(false)
    {
        mAudioFormatManager.registerBasicFormats();
    }
    FileHandler::~FileHandler() {}

    bool FileHandler::openFile(juce::File file)
    {
        mFile = file;
        mAudioReader.reset(mAudioFormatManager.createReaderFor(mFile));

        EXPECT_OR_RETURN (
            mAudioReader != nullptr,
            false, 
            "Unable to create audio file reader source from file {}.",
            mFile.getFullPathName().toStdString());
        
        mFileAttributes.length = mAudioReader->lengthInSamples;
        mFileAttributes.numberOfChannels = mAudioReader->numChannels;
        mBuffer.setSize((int)mFileAttributes.numberOfChannels, 
                        (int)mFileAttributes.length);
        mPlayhead = 0;
        mFileAttributes.sampleRate = mAudioReader->sampleRate;
        mSamplesPerBlock = (int)std::floor(mFileAttributes.sampleRate / 10.0);

        mFileAttributes.metadata = mAudioReader->metadataValues;

        juce::String loudnessMetadata = 
            mFileAttributes.metadata.getValue(LoudnessTag, Unset_v);
        if (loudnessMetadata == Unset_v) 
        { 
            mHasLoudnessMetadata = false;
        }
        else
        {
            mHasLoudnessMetadata = true;
            mLoudness = loudnessMetadata.getFloatValue();
        }

        juce::String samplePeakMetadata =
            mFileAttributes.metadata.getValue(SamplePeakTag, Unset_v);
        if (samplePeakMetadata == Unset_v)
        {
            mHasSamplePeakMetadata = false;
        }
        else
        {
            mHasSamplePeakMetadata = true;
            mPeak = samplePeakMetadata.getFloatValue();
        }

        mHasFileOpen = true;
        return true;
    }
    bool FileHandler::readNextBlock(juce::AudioBuffer<float>* buffer)
    {
        EXPECT_OR_RETURN (mHasFileOpen,
                          false,
                          "Cannot perform read without a file open");

        if (mPlayhead + mSamplesPerBlock > mFileAttributes.length) return false;

        mAudioReader->read (buffer,
                            0,
                            mSamplesPerBlock,
                            (int)mPlayhead,
                            true,
                            true);

        for (unsigned int ch = 0; ch < mFileAttributes.numberOfChannels; ch++)
        {
            mBuffer.copyFrom ((int)ch, 
                              (int)mPlayhead, 
                              buffer->getReadPointer((int)ch), 
                              mSamplesPerBlock);
        }

        mPlayhead += mSamplesPerBlock;
        return true;
    }
    void FileHandler::applyGainDecibel(float gain)
    {
        EXPECT_OR_RETURN (mHasLoudnessMetadata, 
                          void(), 
                          "Calculate Loudness before applying gain");

        float linear_gain = juce::Decibels::decibelsToGain(gain);
        mBuffer.applyGain(linear_gain);

        mLoudness += gain;
        mFileAttributes.metadata.set(LoudnessTag, juce::String(mLoudness));

        mPeak *= linear_gain;
        mFileAttributes.metadata.set(SamplePeakTag, juce::String(mPeak));
        mHasSamplePeakMetadata = true;
    }
    
    void FileHandler::setLoundessMetadata(float loudness)
    {
        mLoudness = loudness;
        mFileAttributes.metadata.set(LoudnessTag, juce::String(mLoudness));
        mHasLoudnessMetadata = true;
    }
    float FileHandler::getLoudnessMetadata() const
    {
        EXPECT_OR_THROW(
            hasLoudnessMetadata(),
            std::exception("No Loudness Metadata to return"),
            "No Loudness Metadata is present"
        );

        return mLoudness;
    }
    
    void FileHandler::setSamplePeakMetadata(float peak)
    {
        mPeak = peak;
        mFileAttributes.metadata.set(SamplePeakTag, juce::String(peak));
        mHasSamplePeakMetadata = true;
    }
    float FileHandler::getSamplePeakMetadata() const
    {
        EXPECT_OR_THROW(
            hasSamplePeakMetadata(),
            std::exception("No Loudness Metadata to return"),
            "No Loudness Metadata is present"
        );

        return mPeak;
    }
    
    void FileHandler::writeFile()
    {
        EXPECT_OR_RETURN (mHasLoudnessMetadata && mHasFileOpen,
                          void(),
                          "No file open, or file not analyzed");

        std::unique_ptr<juce::AudioFormat> format;
        format.reset(mAudioFormatManager.findFormatForFileExtension(
            mFile.getFileExtension()));

        juce::FileOutputStream outputStream(mFile);

        auto writer = format->createWriterFor(&outputStream,
                                              mFileAttributes.sampleRate,
                                              mFileAttributes.numberOfChannels,
                                              (int)mAudioReader->bitsPerSample,
                                              mFileAttributes.metadata,
                                              0);

        writer->writeFromAudioSampleBuffer (mBuffer,
                                            0, 
                                            (int)mFileAttributes.length);

        mHasFileOpen = false;
    }

}