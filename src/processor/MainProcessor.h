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
*/

class BackendTest;

namespace norm
{

class MainProcessor
{
    const float eps = 0.01f;
    using nodeAction = std::function<void(juce::ValueTree, juce::File)>;

public:
    MainProcessor(juce::ValueTree& root);
    MainProcessor(MainProcessor&&) = delete;
    MainProcessor(const MainProcessor&) = delete;

    // TODO: f/#16
    //void abort() { mAbortFlag = true; }
    
    /** Parses everything in the root directory. Can be ran when only the root
     *  directory is set in the value tree.
     *  Looks for loudness and peak metadata. If not found, sets measured
     *  property to false. Does not load actual audio or takes any measurements.
     */
    void parse() noexcept;

    /** Takes measurements of files that are referenced in the value tree. If
     *  ignore_tags settings is off, skips files that do have loudness and
     *  peak metadata. Writes measurements into file metadata and valuetree as
     *  well.
     */
    void measure() noexcept;

    /** Tries to normalise all valid, measured files to target loudness in
     *  settings. Invalid files are skipped, valid files that could not be
     *  normalised are logged.
     *  Changes loudness and sample peak values in value tree and file metadata.
     *  Skips and logs valid files that are no measured.
     */
    void normailse() noexcept;

    /** Sets root file in value tree and starts a new parse process.
     *  Counts all available audio files and updates it in value tree.
     */
    void setRootDirectory(juce::File directory);
    juce::ValueTree getValueTree() const { return mRoot; }

private:
    void traverse (juce::ValueTree node, nodeAction callback) noexcept;
    bool canProcessFile(juce::File file);
    void parseFile(juce::ValueTree fileNode, juce::File file);
    void parseDirectory(juce::ValueTree folderNode, juce::File directory);
    void countAudioFiles();

    juce::AudioFormatManager mTestManager;

    juce::ValueTree& mRoot;
    bool mAbortFlag;

    friend BackendTest;
};
}