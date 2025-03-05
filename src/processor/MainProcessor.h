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

// TODO:
/*
    the three public utility funcitons: 
    beginProcessing | resync | setRootDirectory
    could all have a wrapper that actually starts executing them in a seperate
    thread. possibly a simple thread, not a threadpool, because they should not
    be ran in paralel. however, the file processing COULD run in paralel...
    This topic probably needs to be a separate issue. but one thread could and 
    should be done here

    loudness measuring and normalising should all probably be a little more
    separated, even into different deep-searching processes.
*/


class MainProcessor
{
    const float eps = 0.01;

public:
    MainProcessor();
    MainProcessor(MainProcessor&&) = delete;
    MainProcessor(const MainProcessor&) = delete;

    void abort() { mAbortFlag = true; }
    void beginProcessing();
    void resync();

    void setRootDirectory(juce::File directory);

    juce::ValueTree getValueTree() const { return mRoot; }

    float getTargetLKFS() const 
    { 
        return (float)mRoot.getChildWithName(vt::Tree::settings)
                            [vt::Settings::target_lkfs];
    }
    bool getShouldSearchRecursively() const
    {
        return (bool)mRoot.getChildWithName(vt::Tree::settings)
                           [vt::Settings::recursive_search];
    }
    bool getShouldIgnoreLoudnessTag() const
    {
        return (bool)mRoot.getChildWithName(vt::Tree::settings)
                           [vt::Settings::ignore_loudness_tag];
    }
    bool getShouldIgnoreSamplePeak() const
    {
        return (bool)mRoot.getChildWithName(vt::Tree::settings)
                           [vt::Settings::ignore_sample_peak];
    }
    bool getShouldFollowSymLinks() const 
    {
        return (bool)mRoot.getChildWithName(vt::Tree::settings)
                           [vt::Settings::follow_symlinks];
    }

private:
    bool canProcessFile(juce::File file);
    void parseFile(juce::File file, juce::ValueTree fileNode);
    void parseDirectory(juce::File directory, juce::ValueTree folderNode);
    int countAudioFiles() const;
    int countFilesInFolderRecursively(juce::ValueTree folderNode) const;

    void processFile(juce::ValueTree fileNode);
    void processDirectory(juce::ValueTree directoryNode);

    FileHandler mFileHandler;
    LKFS mLoudnessProcessor;
    juce::AudioFormatManager mTestManager;

    juce::ValueTree mRoot;
    bool mAbortFlag;
};
}