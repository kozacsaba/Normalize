#include "FileHandler.h"
#include "util/Logger.h"
#include "util/Expections.h"

namespace norm
{
    FileHandler::FileHandler(juce::File file)
        : mFile(file)
    {
        mAudioFormatManager.registerBasicFormats();

        mAudioReader.reset(mAudioFormatManager.createReaderFor(mFile));
        if (mAudioReader == nullptr) exc::FileHandler::get::no_reader_for_file();

        mFileAttributes.numberOfChannels = mAudioReader->numChannels;
        mFileAttributes.length = mAudioReader->lengthInSamples;
        mFileAttributes.sampleRate = mAudioReader->sampleRate;
        mFileAttributes.metadata = mAudioReader->metadataValues;

        mSamplesPerBlock = (int)std::floor(mFileAttributes.sampleRate / 10.0);

        bool tmp_isMeasured = true;

        juce::String loudnessMetadata =
            mFileAttributes.metadata.getValue(LoudnessTag, Unset_v);
        if (loudnessMetadata == Unset_v)
            tmp_isMeasured = false;
        else
            mLoudness = loudnessMetadata.getFloatValue();

        juce::String samplePeakMetadata =
            mFileAttributes.metadata.getValue(SamplePeakTag, Unset_v);
        if (samplePeakMetadata == Unset_v)
            tmp_isMeasured = false;
        else
            mPeak = samplePeakMetadata.getFloatValue();

        fMeasured = tmp_isMeasured;

        mWorkBuffer.setSize(
            mFileAttributes.numberOfChannels,
            mSamplesPerBlock
        );
    }
    FileHandler::~FileHandler() {}

    bool FileHandler::loadAudio()
    {
        mPlayhead = 0;

        mBuffer.setSize ((int)mFileAttributes.numberOfChannels, 
                         (int)mFileAttributes.length);

        bool success = mAudioReader->read (&mBuffer,
                                           0,
                                           mFileAttributes.length,
                                           0,
                                           true,
                                           true);

        fAudioLoaded = success;
        return fAudioLoaded;
    }
    void FileHandler::measure()
    {
        if (!fAudioLoaded) exc::FileHandler::get::no_audio_loaded();

        mPlayhead = 0;
        mProcessor.reset (mFileAttributes.sampleRate, 
                          mFileAttributes.numberOfChannels);

        while(readNextBlock(&mWorkBuffer))
        {
            mProcessor.processNext100ms(mWorkBuffer);
        }

        mLoudness = mProcessor.getIntegratedLoudness();
        mPeak = mProcessor.getSamplePeak();

        fMeasured = true;
    }
    void FileHandler::applyGainDecibel(float gain)
    {
        if (!fAudioLoaded) exc::FileHandler::get::no_audio_loaded();

        float linear_gain = juce::Decibels::decibelsToGain(gain);
        mBuffer.applyGain(linear_gain);

        if (fMeasured)
        {
            mLoudness += gain;
            mPeak *= linear_gain;
        }
    }
    void FileHandler::writeFile()
    {
        if (!fAudioLoaded) exc::FileHandler::get::no_audio_loaded();

        std::unique_ptr<juce::AudioFormat> format;
        format.reset(mAudioFormatManager.findFormatForFileExtension(
            mFile.getFileExtension()));

        juce::FileOutputStream outputStream(mFile);

        if (fMeasured)
        {
            mFileAttributes.metadata.set(LoudnessTag, juce::String(mLoudness));
            mFileAttributes.metadata.set(SamplePeakTag, juce::String(mPeak));
        }

        auto writer = format->createWriterFor(&outputStream,
                                              mFileAttributes.sampleRate,
                                              mFileAttributes.numberOfChannels,
                                              (int)mAudioReader->bitsPerSample,
                                              mFileAttributes.metadata,
                                              0);

        writer->writeFromAudioSampleBuffer (mBuffer,
                                            0, 
                                            (int)mFileAttributes.length);
    }

    bool FileHandler::readNextBlock(juce::AudioBuffer<float>* buffer)
    {
        if (mPlayhead + mSamplesPerBlock > mFileAttributes.length) return false;

        if (!fAudioLoaded) exc::FileHandler::get::no_audio_loaded();

        bool bufferSizeOkay =
            buffer != nullptr &&
            buffer->getNumChannels() >= mFileAttributes.numberOfChannels &&
            buffer->getNumSamples() >= mSamplesPerBlock;
        if (!bufferSizeOkay) exc::FileHandler::get::insufficient_buffer();

        for (int ch = 0; ch < mFileAttributes.numberOfChannels; ch++)
        {
            buffer->copyFrom(
                ch,
                0,
                mBuffer.getReadPointer(ch, mPlayhead),
                mSamplesPerBlock
            );
        }

        mPlayhead += mSamplesPerBlock;

        return true;
    }

}