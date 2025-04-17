#include "FileHandler.h"
#include "util/Logger.h"
#include "util/Expections.h"

namespace norm
{
    FileHandler::FileHandler(juce::File file)
        : mFile(file)
        , mFormat(Format::unknown)
    {
        mAudioFormatManager.registerBasicFormats();

        mAudioReader.reset(mAudioFormatManager.createReaderFor(mFile));
        if (mAudioReader == nullptr) exc::FileHandler::get::no_reader_for_file();
        setFormat(mAudioReader->getFormatName());

        mFileAttributes.numberOfChannels = mAudioReader->numChannels;
        mFileAttributes.length = mAudioReader->lengthInSamples;
        mFileAttributes.sampleRate = mAudioReader->sampleRate;

        mSamplesPerBlock = (int)std::floor(mFileAttributes.sampleRate / 10.0);
        mWorkBuffer.setSize(
            (int) mFileAttributes.numberOfChannels,
            mSamplesPerBlock
        );

        // >>>>> metadata will be handled in a different way
        {
        mFileAttributes.metadata = mAudioReader->metadataValues;
        }
        // <<<<<

        // >>>>> metadata will be handled in a different way
        {
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
        }
        // <<<<<
    }
    FileHandler::~FileHandler() 
    {
        // reader must not outlive the format manager
        mAudioReader.reset();
    }

    bool FileHandler::loadAudio()
    {
        mPlayhead = 0;

        mBuffer.setSize ((int)mFileAttributes.numberOfChannels, 
                         (int)mFileAttributes.length);

        // TODO: audio might not fit in one buffer - issue #25
        bool success = mAudioReader->read (&mBuffer,
                                           0,
                                           (int) mFileAttributes.length,
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
                          (int) mFileAttributes.numberOfChannels);

        while(readNextBlock(&mWorkBuffer))
        {
            mProcessor.processNext100ms(mWorkBuffer);
        }

        mLoudness = mProcessor.getIntegratedLoudness();
        mPeak = mProcessor.getSamplePeak();

        fMeasured = true;
    }
    void FileHandler::writeWithGain(float gain_dB)
    {
        if (!fAudioLoaded) exc::FileHandler::get::no_audio_loaded();
        float gain_lin = juce::Decibels::decibelsToGain(gain_dB);

        if (fMeasured)
        {
            mLoudness += gain_dB;
            mPeak *= gain_lin;
        }

        switch (mFormat)
        {
        case Format::mp3:
            writeFormatMP3(gain_dB);
            break;
        
        case Format::wav:
            writeFormatWav(gain_dB);
            break;

        default:
            exc::FileHandler::get::format_not_supported();
        }
    }
    
    bool FileHandler::readNextBlock(juce::AudioBuffer<float>* buffer)
    {
        if (mPlayhead + mSamplesPerBlock > mFileAttributes.length) return false;

        if (!fAudioLoaded) exc::FileHandler::get::no_audio_loaded();

        bool bufferSizeOkay =
            buffer != nullptr &&
            buffer->getNumChannels() >= (int) mFileAttributes.numberOfChannels &&
            buffer->getNumSamples() >= mSamplesPerBlock;
        if (!bufferSizeOkay) exc::FileHandler::get::insufficient_buffer();

        for (int ch = 0; ch < (int) mFileAttributes.numberOfChannels; ch++)
        {
            buffer->copyFrom(
                ch,
                0,
                mBuffer.getReadPointer(ch, (int) mPlayhead),
                mSamplesPerBlock
            );
        }

        mPlayhead += mSamplesPerBlock;

        return true;
    }
    void FileHandler::setFormat(juce::String format)
    {
        if (format == juce::WavAudioFormat{}.getFormatName())
        {
            mFormat = Format::wav;
            return;
        }
        else if (format == juce::MP3AudioFormat{}.getFormatName())
        {
            mFormat = Format::mp3;
            return;
        }
        else
        {
            MY_LOG_ERROR(
                "Audio file format \"{}\" not supported.",
                format
            );
            exc::FileHandler::get::format_not_supported();
        }
    }

    void FileHandler::writeFormatWav(float gain_lin)
    {
        mBuffer.applyGain(gain_lin);

        auto* format = mAudioFormatManager.findFormatForFileExtension(
            mFile.getFileExtension());

        // >>>>> metadata
        if (fMeasured)
        {
            mFileAttributes.metadata.set(LoudnessTag, juce::String(mLoudness));
            mFileAttributes.metadata.set(SamplePeakTag, juce::String(mPeak));
        }
        // <<<<< metadata

        // will be deleted by the writer if created successfully
        auto* outStream = new juce::FileOutputStream(mFile);

        auto writer = std::unique_ptr<juce::AudioFormatWriter>(
            format->createWriterFor(
                outStream,
                mFileAttributes.sampleRate,
                mFileAttributes.numberOfChannels,
                (int)mAudioReader->bitsPerSample,
                // note:
                // metadata is written into file here too, but this is actually
                // fine, because this is not the custom metadata for storing
                // loudness and peak data, but standard data, like artist,
                // album, title, etc.
                mFileAttributes.metadata,
                0));

        if (writer == nullptr)
        {
            // This is kinda silly on JUCE's part, because if the writer was
            // created successfully, it does own the stream, if it wasn't, then
            // it doesn't. So we cannot allocate in the constructor argument
            // list - to enforce ownership relations - because that could leave
            // us with leaking memory. We have to create a raw pointer and 
            // either manually manage it or leave it to the writer, depending on
            // whether it could be created successfully or not.
            delete outStream;
            exc::FileHandler::get::no_writer_for_File();
        }

        writer->writeFromAudioSampleBuffer (mBuffer,
                                            0, 
                                            (int)mFileAttributes.length);

        const bool flushed = writer->flush();
        jassert(flushed);
    }
    void FileHandler::writeFormatMP3(float gain_dB)
    {
        //
    }
}