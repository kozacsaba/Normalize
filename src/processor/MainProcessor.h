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

// TODO
/*
    f/#16
    the three public utility funcitons: 
    beginProcessing | resync | setRootDirectory
    could all have a wrapper that actually starts executing them in a seperate
    thread. possibly a simple thread, not a threadpool, because they should not
    be ran in paralel. however, the file processing COULD run in paralel...
    This topic probably needs to be a separate issue. but one thread could and 
    should be done here

    cq/#17
    loudness measuring and normalising should all probably be a little more
    separated, even into different deep-searching processes. Most of the file-
    level processing logic should be moved to the FileHandler class
*/

class MainProcessor
{
    const float eps = 0.01f;

public:
    MainProcessor(juce::ValueTree root);
    MainProcessor(MainProcessor&&) = delete;
    MainProcessor(const MainProcessor&) = delete;

    void abort() { mAbortFlag = true; }
    void beginProcessing();
    void resync();

    void setRootDirectory(juce::File directory);

    juce::ValueTree getValueTree() const { return mRoot; }

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