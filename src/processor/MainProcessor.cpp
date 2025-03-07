#include "MainProcessor.h"
#include "util/Logger.h"
#include "util/Expections.h"

using namespace norm;

MainProcessor::MainProcessor(juce::ValueTree root)
    : mRoot(root)
    , mAbortFlag(false)
{
    mTestManager.registerBasicFormats();
}

void MainProcessor::beginProcessing()
{
    mRoot.getChildWithName(vt::Tree::interface).setProperty
    (
        vt::Interface::is_processing,
        true,
        nullptr
    );

    try
    {
        juce::ValueTree dirNode = mRoot.getChildWithName(vt::Tree::directory);
        juce::ValueTree rootDir = dirNode.getChild(0);
        if (rootDir == juce::ValueTree()) exc::MainProc::get::root_dir_unset();

        juce::File rootFile(rootDir[vt::Directory::Folder::path]);
        if (!rootFile.isDirectory()) exc::MainProc::get::root_dir_missing();

        processDirectory(rootDir);
    }
    catch (exc::MainProc::exception& e)
    {
        MY_LOG_ERROR(
            "Error while processing files: {}",
            e.what()
        );

        mRoot.getChildWithName(vt::Tree::interface).setProperty(
            vt::Interface::show_log,
            true,
            nullptr
        );
    }
    catch (std::exception& e)
    {
         MY_LOG_ERROR(
            "Unexpected error: {}",
            e.what()
        );

        mRoot.getChildWithName(vt::Tree::interface).setProperty(
            vt::Interface::show_log,
            true,
            nullptr
        );
    }

    mRoot.getChildWithName(vt::Tree::interface).setProperty
    (
        vt::Interface::is_processing,
        false,
        nullptr
    );
}
void MainProcessor::resync()
{
    try
    {
        juce::ValueTree rootNode = mRoot.getChildWithName(vt::Tree::directory)
                                        .getChild(0);

        if(!rootNode.isValid()) exc::MainProc::get::root_dir_unset();

        juce::File rootFile(rootNode[vt::Directory::Folder::path]);
        if(!rootFile.isDirectory()) exc::MainProc::get::root_dir_missing();

        rootNode.removeAllChildren(nullptr);
        parseDirectory(rootFile, rootNode);
    }
    catch(const exc::MainProc::exception& e)
    {
        MY_LOG_ERROR(
            "Error while attempting to parse directory: {}",
            e.what()
        );
    }
    catch(const std::exception& e)
    {
        MY_LOG_ERROR(
            "Unexpected error: {}",
            e.what()
        );
    }
}
void MainProcessor::setRootDirectory(juce::File directory)
{
    try
    {
        EXPECT_OR_RETURN(
            directory.isDirectory(),
            void (),
            "Selected item is not a directory: {}",
            directory.getFullPathName()
        );

        juce::ValueTree rootNode(vt::Directory::folder);
        parseDirectory(directory, rootNode);

        auto directoryTree = mRoot.getChildWithName(vt::Tree::directory);
        directoryTree.removeAllChildren(nullptr);

        directoryTree.appendChild(rootNode, nullptr);
    }
    catch (exc::MainProc::exception& e)
    {
        MY_LOG_ERROR(
            "Error while setting root directory: {}. Please try again.",
            e.what()
        );
    }
    catch (std::exception& e)
    {
        MY_LOG_ERROR(
            "Unexcepted error: {}",
            e.what()
        );
    }
}

bool MainProcessor::canProcessFile(juce::File file)
{
    juce::String extension = file.getFileExtension();
    auto* format = mTestManager.findFormatForFileExtension(extension);
    if (format == nullptr) return false;

    juce::AudioFormatReader* reader =
        mTestManager.createReaderFor(file);

    if (reader == nullptr) return false;

    delete reader;
    return true;
}
void MainProcessor::parseFile(juce::File file, juce::ValueTree fileNode)
{
    EXPECT_OR_RETURN(
        fileNode.getType() == vt::Directory::file,
        void (),
        "ValueTree node provided is not a file node"
    );

    using namespace vt::Directory;

    fileNode.setProperty (
        File::path,
        file.getFullPathName(),
        nullptr
    );

    fileNode.setProperty (
        File::name,
        file.getFileName(),
        nullptr
    );

    fileNode.setProperty(
        File::valid,
        canProcessFile(file),
        nullptr
    );

    if (!fileNode[File::valid])
    {
        fileNode.setProperty(
            File::err_msg,
            "File format not supported",
            nullptr
        );

        return;
    }

    fileNode.setProperty(
        File::has_warning,
        false,
        nullptr
    );

    fileNode.setProperty(
        File::selected,
        false,
        nullptr
    );

    mFileHandler.openFile(file);
    const bool processed = mFileHandler.hasLoudnessMetadata() &&
                           mFileHandler.hasSamplePeakMetadata();

    fileNode.setProperty(
        File::processed,
        processed,
        nullptr
    );

    if (!fileNode[File::processed]) return;

    fileNode.setProperty(
        File::loudness,
        mFileHandler.getLoudnessMetadata(),
        nullptr
    );

    fileNode.setProperty(
        File::peak,
        mFileHandler.getSamplePeakMetadata(),
        nullptr
    );

    if ((float)fileNode[File::peak] >= 1.f)
    {
        fileNode.setProperty(
            File::has_warning,
            true,
            nullptr
        );

        fileNode.setProperty(
            File::err_msg,
            "Audio is potentially peaking.",
            nullptr
        );
    }
}
void MainProcessor::parseDirectory(juce::File directory, juce::ValueTree folderNode)
{
    EXPECT_OR_RETURN(
        folderNode.getType() == vt::Directory::folder,
        void (),
        "ValueTree node provided is not a folder node"
    );

    EXPECT_OR_RETURN(
        directory.isDirectory(),
        void (),
        "File provided is not a directory"
    );

    using namespace vt::Directory;

    folderNode.setProperty(
        Folder::path,
        directory.getFullPathName(),
        nullptr
    );

    folderNode.setProperty(
        Folder::name,
        directory.getFileName(),
        nullptr
    );

    auto children = directory.findChildFiles(
        juce::File::findFilesAndDirectories,
        false
    );

    for (auto& child : children)
    {
        if (child.existsAsFile())
        {
            juce::ValueTree childFileNode(vt::Directory::file);
            parseFile(child, childFileNode);
            folderNode.appendChild(childFileNode, nullptr);
        }
        else if (child.isDirectory())
        {
            juce::ValueTree childFolderNode(vt::Directory::folder);
            parseDirectory(child, childFolderNode);
            folderNode.appendChild(childFolderNode, nullptr);
        }
        else
        {
            MY_LOG_WARNING(
                "Item {} is not recognised as a file, nor a folder.",
                child.getFullPathName()
            );
        }
    }
}

int MainProcessor::countAudioFiles() const
{
    juce::ValueTree directory = mRoot.getChildWithName(vt::Tree::directory);
    juce::ValueTree rootDir = directory.getChild(0);
    if(rootDir == juce::ValueTree() ) exc::MainProc::get::root_dir_unset();

    int numAudioFiles = countFilesInFolderRecursively(rootDir);
    juce::ValueTree interface = mRoot.getChildWithName(vt::Tree::interface);
    interface.setProperty(
        vt::Interface::number_of_files,
        numAudioFiles,
        nullptr
    );
    return numAudioFiles;
}
int MainProcessor::countFilesInFolderRecursively(juce::ValueTree folderNode) const
{
    EXPECT_OR_RETURN(
        folderNode.getType() == vt::Directory::folder,
        0,
        "Node has type {}, not folder. Children cannot be counted.",
        folderNode.getType().toString()
    );

    int fileCount = 0;
    int childCount = folderNode.getNumChildren();

    for (int i = 0; i < childCount; i++)
    {
        juce::ValueTree child = folderNode.getChild(i);

        if (child.getType() == vt::Directory::file)
        {
            if(child[vt::Directory::File::valid]) fileCount++;
        }
        else if ( child.getType() == vt::Directory::folder)
        {
            fileCount += countFilesInFolderRecursively(child);
        }
    }

    return fileCount;
}

void MainProcessor::processFile(juce::ValueTree fileNode)
{
    if (mAbortFlag) { exc::MainProc::get::abort(); }

    EXPECT_OR_RETURN(
        fileNode.getType() == vt::Directory::file,
        void (),
        "Tried to process an item with type {} as a file. (skipped)",
        fileNode.getType().toString()
    );

    juce::File file(fileNode[vt::Directory::File::name]);

    EXPECT_OR_RETURN(
        file.existsAsFile(),
        void (),
        "File does not exist: {}. (skipped)",
        file.getFullPathName()
    );

    mRoot.getChildWithName(vt::Tree::interface).setProperty(
        vt::Interface::current_file,
        fileNode[vt::Directory::File::name],
        nullptr
    );

    mFileHandler.openFile(file);
    float loudness = 0;
    float peak = 0;
    const bool shouldIgnoreLoudnessTag = 
        mRoot.getChildWithName(vt::Tree::settings)
        [vt::Settings::ignore_loudness_tag];

    if (fileNode[vt::Directory::File::processed] &&
        !shouldIgnoreLoudnessTag)
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

    const float target = 
        mRoot.getChildWithName(vt::Tree::settings)
        [vt::Settings::target_lkfs]; 
    const float diff = target - loudness;
    const float ratio = juce::Decibels::decibelsToGain(diff);

    if ( ratio < 1.f + eps && ratio > 1.f - eps )
    {
        fileNode.setProperty(
            vt::Directory::File::processed,
            true,
            nullptr
        );
        fileNode.setProperty(
            vt::Directory::File::loudness,
            loudness,
            nullptr
        );
        fileNode.setProperty(
            vt::Directory::File::peak,
            peak,
            nullptr
        );
        return;
    }

    const bool shouldIgnoreSamplePeak =
        mRoot.getChildWithName(vt::Tree::settings)
        [vt::Settings::ignore_sample_peak];
    if (!shouldIgnoreSamplePeak)
    {
        float wouldBePeak = peak * ratio;
        if (wouldBePeak > 1.f)
        {
            // TODO
            // DO SOMETHING HERE
            // maybe an exception
        }
    }

    fileNode.setProperty(
        vt::Directory::File::processed,
        true,
        nullptr
    );
    fileNode.setProperty(
        vt::Directory::File::loudness,
        target,
        nullptr
    );
    fileNode.setProperty(
        vt::Directory::File::peak,
        peak,
        nullptr
    );

    mFileHandler.applyGainDecibel(diff);
    mFileHandler.writeFile();
}
void MainProcessor::processDirectory(juce::ValueTree directoryNode)
{
    if (mAbortFlag) { exc::MainProc::get::abort(); }

    EXPECT_OR_RETURN(
        directoryNode.getType() == vt::Directory::folder,
        void (),
        "Tried to process an item with type {} as a folder. (skipped)",
        directoryNode.getType().toString()
    );

    int childCount = directoryNode.getNumChildren();
    for (int i = 0; i < childCount; i++)
    {
        juce::ValueTree child = directoryNode.getChild(i);

        if (child.getType() == vt::Directory::file)
        {
            processFile(child);
        }
        else if (child.getType() == vt::Directory::folder)
        {
            processDirectory(child);
        }
        else
        {
            MY_LOG_WARNING(
                "Corrupted ValueTree Node with type {}. (skipped)",
                child.getType().toString()
            );
        }
    }
}
