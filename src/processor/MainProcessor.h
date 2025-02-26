#pragma once

/*  This lil thing here should encapsulate everything audio and processing
    related. A pointer to this will be passed to the main component, so that
    gui components will be able to call public methods.
*/

#include "FileHandler.h"
#include "LKFSProcessor.h"
#include "util/VTNames.h"
#include "juce_data_structures/juce_data_structures.h"

namespace norm
{

class MainProcessor
{
    constexpr float eps = 0.01;

public:
    MainProcessor();
    MainProcessor(MainProcessor&&) = delete;
    MainProcessor(const MainProcessor&) = delete;

    juce::ValueTree getValueTree() const { return mRoot; }

    float getTargetLKFS() const 
    { 
        return (float)mRoot[vt::tree::Settings][vt::settings::target_lkfs];
    }
    bool getShouldSearchRecursively() const
    {
        return (bool)mRoot[vt::tree::Settings][vt::settings::recursive_search];
    }
    bool getShouldIgnoreLoudnessTag() const
    {
        return (bool)mRoot[vt::tree::Settings]
                          [vt::settings::ignore_loudness_tag];
    }
    bool getShouldIgnoreSamplePeak() const
    {
        return (bool)mRoot[vt::tree::Settings]
                          [vt::settings::ignore_sample_peak];
    }
    bool getShouldFollowSymLinks() const 
    {
        return (bool)mRoot[vt::tree::Settings]
                          [vt::settings::follow_symlinks];
    }

private:
    void processFile(juce::File file);
    void processDirectory(juce::File directory);


    juce::ValueTree mRoot;

    FileHandler mFileHandler;
    LKFS mLoudnessProcessor;


    
};

}