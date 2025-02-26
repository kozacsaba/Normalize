#include "MainProcessor.h"
#include "util/Logger.h"

using namespace norm;

MainProcessor::MainProcessor()
{

}

void MainProcessor::processFile(juce::File file)
{
    EXPECT_OR_RETURN(
        file.existsAsFile(),
        void{},
        "File does not exist: {}",
        file.getFullPathName()
    );

    mFileHandler.openFile(file);
    float loudness = 0;
    float peak = 0;

    if (mFileHandler.hasLoudnessMetadata() &&
        mFileHandler.hasSamplePeakMetadata() &&
        !getShouldIgnoreLoudnessTag() )
    {
        loudness = mFileHandler.getLoudnessMetadata();
        peak = mFileHandler.getSamplePeakMetadata();
    }
    else
    {
        double sampleRate = mFileHandler.getSampleRate();
        int numberOfChannels = (int)(mFileHandler.getNumberOfChannels());
        int samplesPerBlock = (int)(sampleRate * 0.1);
    
        mLoudnessProcessor.reset((float)sampleRate, numberOfChannels);
        juce::AudioBuffer<float> buffer(numberOfChannels, samplesPerBlock);
    
        while (mFileHandler.readNextBlock(&buffer))
        {
            mLoudnessProcessor.processNext100ms(buffer);
        }

        loudness = mLoudnessProcessor.getIntegratedLoudness();
        peak = mLoudnessProcessor.getSamplePeak();
    }

    float diff = getTargetLKFS() / loudness;
    float ratio = juce::Decibels::decibelsToGain(diff);

    if ( ratio < 1.f + eps && ratio > 1.f - eps )
    {
        return;
    }

    if (!getShouldIgnoreSamplePeak())
    {
        float wouldBePeak = peak * ratio;
        if (wouldBePeak > 1.f)
        {
            // DO SOMETHING HERE
            // maybe an exception
        }
    }

    mFileHandler.applyGainDecibel(diff);
    mFileHandler.writeFile();
}

void MainProcessor::processDirectory(juce::File directory)
{
    if (directory.existsAsFile())
    {
        MY_LOG_INFO("The following file was passed to process as a directory:\
             {}. It will be skipped", directory.getFullPathName());
        return;
    }

    auto childFiles = directory.findChildFiles(
        juce::File::TypesOfFileToFind::findFilesAndDirectories,
        false,
        "*",
        getShouldFollowSymLinks() ? juce::File::FollowSymlinks::noCycles 
                                  : juce::File::FollowSymlinks::no
    );

    for(const auto& file : childFiles)
    {
        if (file.existsAsFile())
        {
            processFile(file);
            continue;
        }

        if (file.isDirectory())
        {
            if (getShouldSearchRecursively())
            {
                processDirectory(file);
            }
            else
            {
                continue;
            }
        }

        MY_LOG_WARNING("The following file was not processed, because it appears\
            to be neither a file, nor a directory: {}", file.getFullPathName());
    }
}