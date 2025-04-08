#pragma once

/*  This lil thing here should encapsulate everything audio and processing
    related. A pointer to this will be passed to the main component, so that
    gui components will be able to call public methods.
*/

#include <functional>
#include <juce_data_structures/juce_data_structures.h>

#include "FileHandler.h"
#include "LKFSProcessor.h"
#include "util/VTNames.h"

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
    using nodeAction = std::function<void(juce::ValueTree, juce::File)>;

public:
    MainProcessor(juce::ValueTree& root);
    MainProcessor(MainProcessor&&) = delete;
    MainProcessor(const MainProcessor&) = delete;

    void abort() { mAbortFlag = true; }
    
    void parse() noexcept;
    void measure() noexcept;
    void normailse() noexcept;

    void setRootDirectory(juce::File directory);
    juce::ValueTree getValueTree() const { return mRoot; }

private:
    void traverse (juce::ValueTree node, nodeAction callback) noexcept;

    bool canProcessFile(juce::File file);
    void parseFile(juce::ValueTree fileNode, juce::File file);
    void parseDirectory(juce::ValueTree folderNode, juce::File directory);
    int countAudioFiles() const;
    int countFilesInFolderRecursively(juce::ValueTree folderNode) const;



    juce::AudioFormatManager mTestManager;

    juce::ValueTree& mRoot;
    bool mAbortFlag;
};
}