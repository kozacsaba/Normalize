#include "FileHandler.h"
#include "util/Logger.h"
#include "util/Expections.h"
#include "wrapper/mp3gain.hpp"
#include "wav/wavfile.h"
#include "mpegfile.h"
#include "textidentificationframe.h"
#include "tfilestream.h"

using namespace norm;

FileHandler::FileHandler(juce::File file)
    : mFile(file)
    , mFormat(Format::unknown)
    , fMeasured(false)
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

    switch (mFormat)
    {
    case Format::wav:
    {
        TagLib::RIFF::WAV::File wavFile(
            mFile.getFullPathName().toStdString().c_str());
        ID3Tag* tag = wavFile.ID3v2Tag();
        parseID3v2Tag(tag);
        break;
    }
    case Format::mp3:
    {
        TagLib::MPEG::File mp3File(
            mFile.getFullPathName().toStdString().c_str());
        ID3Tag* tag = mp3File.ID3v2Tag();
        parseID3v2Tag(tag);
        break;
    }
    case Format::unknown:
    default:
        exc::FileHandler::get::format_not_supported();
    }
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

    case Format::unknown:
    default:
        exc::FileHandler::get::format_not_supported();
    }
}

bool FileHandler::readNextBlock(juce::AudioBuffer<float>* buffer)
{
    // last block should be discarded if incomplete, according to ITU BS.1770-5
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

void FileHandler::parseID3v2Tag(ID3Tag* tag)
{
    auto lkfs = TagLib::ID3v2::UserTextIdentificationFrame::find(tag, LoudnessTag);
    auto peak = TagLib::ID3v2::UserTextIdentificationFrame::find(tag, SamplePeakTag);
    if(!lkfs || !peak) return;

    const TagLib::StringList& lkfsFields = lkfs->fieldList();
    const TagLib::StringList& peakFields = peak->fieldList();

    if(lkfsFields.isEmpty() || peakFields.isEmpty()) return;

    const std::string lkfsStr = lkfsFields[1].to8Bit();
    const std::string peakStr = peakFields[1].to8Bit();

    mLoudness = std::stof(lkfsStr);
    mPeak = std::stof(peakStr);
    fMeasured = true;
}
void FileHandler::deepCopyRIFF(RIFFTag* source, RIFFTag* target)
{
    const auto& flm = source->fieldListMap();
    for(auto& field : flm)
    {
        target->setFieldText(field.first, field.second);
    }
}
void FileHandler::deepCopyID3v2(ID3Tag* source, ID3Tag* target)
{
    while(!target->frameListMap().isEmpty())
    {
        target->removeFrames(
            target->frameListMap().begin()->first
        );
    }

    auto* tagHeader = source->header();
    auto& fl = source->frameList();
    for(auto* frame : fl)
    {
        target->addFrame(TagLib::ID3v2::FrameFactory::instance()->createFrame(
            frame->render(), tagHeader));
    }
}

void FileHandler::writeFormatWav(float gain_dB)
{
    // save metadata ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    std::unique_ptr<RIFFTag> riffData = nullptr;
    std::unique_ptr<ID3Tag> id3v2Data = nullptr;

    auto wavFile = std::make_unique<TagLib::RIFF::WAV::File>(
        mFile.getFullPathName().toStdString().c_str());

    if(wavFile->hasInfoTag())
    {
        riffData = std::make_unique<RIFFTag>();
        deepCopyRIFF(wavFile->InfoTag(), riffData.get());
    }

    if(wavFile->hasID3v2Tag())
    {
        id3v2Data = std::make_unique<ID3Tag>();
        deepCopyID3v2(wavFile->ID3v2Tag(), id3v2Data.get());
    }

    wavFile.reset();

    // apply gain (involves deleting and rewriting file) ~~~~~~~~~~~~~~~~~~~~~~~

    const float gain_lin = juce::Decibels::decibelsToGain(gain_dB);
    mBuffer.applyGain(gain_lin);

    auto format = std::make_unique<juce::WavAudioFormat>();
    mFile.deleteFile();

    // will be owned by the writer if created successfully
    auto* outStream = new juce::FileOutputStream(mFile);

    auto writer = std::unique_ptr<juce::AudioFormatWriter>(
        format->createWriterFor(
            outStream,
            mFileAttributes.sampleRate,
            mFileAttributes.numberOfChannels,
            (int) mAudioReader->bitsPerSample,
            {},
            0
    ));
    
    if (writer)
    {
        writer->writeFromAudioSampleBuffer (mBuffer,
            0, 
            (int)mFileAttributes.length);

        const bool flushed = writer->flush();
        jassert(flushed);
    }
    else
    {
        delete outStream;
        exc::FileHandler::get::no_writer_for_File();
    }
    writer = nullptr;

    // restore metadata ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    auto fileStream = std::make_unique<TagLib::FileStream>(
        mFile.getFullPathName().toStdString().c_str(), false);
    wavFile = std::make_unique<TagLib::RIFF::WAV::File>(
        fileStream.get());
    
    if(riffData)
        deepCopyRIFF(riffData.get(), wavFile->InfoTag());
    if(id3v2Data)
        deepCopyID3v2(id3v2Data.get(), wavFile->ID3v2Tag());

    // write custom tags ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    if (fMeasured)
    {
        using namespace TagLib;

        ID3Tag* tag = wavFile->ID3v2Tag();

        auto lkfsFrame = std::make_unique<ID3v2::UserTextIdentificationFrame>();
        lkfsFrame->setDescription(LoudnessTag);
        lkfsFrame->setText(std::to_string(mLoudness));
        if(auto oldFrame = ID3v2::UserTextIdentificationFrame::find(tag, LoudnessTag))
        {
            tag->removeFrame(oldFrame, true);
        }
        tag->addFrame(lkfsFrame.release());

        auto peakFrame = std::make_unique<ID3v2::UserTextIdentificationFrame>();
        peakFrame->setDescription(SamplePeakTag);
        peakFrame->setText(std::to_string(mPeak));
        if(auto oldFrame = ID3v2::UserTextIdentificationFrame::find(tag, SamplePeakTag))
        {
            tag->removeFrame(oldFrame, true);
        }
        tag->addFrame(peakFrame.release());
    }

    wavFile->save();
}
void FileHandler::writeFormatMP3(float gain_dB)
{
    // sample_value = quantized_value × 2^((global_gain - 210) / 4)
    const float lin_scale = std::sqrt(std::sqrt(2.f));
    const float scale = 20.f * std::log10(lin_scale);
    const float global_gain = gain_dB / scale;
    std::string fileName = mFile.getFullPathName().toStdString();

    wrap_changeGain(
        fileName.data(),
        (int)std::roundf(global_gain),
        (int)std::roundf(global_gain));

    if(fMeasured)
    {
        using namespace TagLib;

        auto mp3File = std::make_unique<MPEG::File>(
            mFile.getFullPathName().toStdString().c_str());
        ID3Tag* tag = mp3File->ID3v2Tag();

        auto lkfsFrame = std::make_unique<ID3v2::UserTextIdentificationFrame>();
        lkfsFrame->setDescription(LoudnessTag);
        lkfsFrame->setText(std::to_string(mLoudness));
        if(auto oldFrame = ID3v2::UserTextIdentificationFrame::find(tag, LoudnessTag))
        {
            tag->removeFrame(oldFrame, true);
        }
        tag->addFrame(lkfsFrame.release());

        auto peakFrame = std::make_unique<ID3v2::UserTextIdentificationFrame>();
        peakFrame->setDescription(SamplePeakTag);
        peakFrame->setText(std::to_string(mPeak));
        if(auto oldFrame = ID3v2::UserTextIdentificationFrame::find(tag, SamplePeakTag))
        {
            tag->removeFrame(oldFrame, true);
        }
        tag->addFrame(peakFrame.release());

        mp3File->save();
    }
}
