#include "MainProcessor.h"
#include "util/Logger.h"
#include "util/Expections.h"

using namespace norm;

MainProcessor::MainProcessor(juce::ValueTree& root)
    : mRoot(root)
    , mAbortFlag(false)
{
    mTestManager.registerBasicFormats();
}

void MainProcessor::parse() noexcept
{
    nodeAction callback = [this](juce::ValueTree node, juce::File file)
    {
        if (file.existsAsFile())
        {
            parseFile(node, file);
        }
        else if (file.isDirectory())
        {
            parseDirectory(node, file);
        }
        else
        {
            MY_LOG_WARNING(
                "Item: {}",
                file.getFullPathName()
            );
            exc::MainProc::get::node_not_file();
        }
    };

    juce::ValueTree rootFileNode = 
        mRoot.getChildWithName(vt::Tree::directory).getChild(0);

    traverse(rootFileNode, callback);
}
void MainProcessor::measure() noexcept
{
    nodeAction callback = [this](juce::ValueTree node, juce::File file)
    {
        if (!file.existsAsFile()) return;
        if (node.getType() != vt::Directory::file) return;

        const bool ignore_tags = 
            mRoot.getChildWithName(vt::Tree::settings)
            [vt::Settings::ignore_tags];
        if(!ignore_tags && node[vt::Directory::File::measured]) return;

        try
        {
            auto fileHandler = std::make_unique<FileHandler>(file);
            const bool loadedAudio = fileHandler->loadAudio();
    
            EXPECT_OR_RETURN (loadedAudio,
                              void (),
                              "Could not load audio for file {} -- Skipped.",
                              file.getFullPathName());
    
            fileHandler->measure();
            fileHandler->writeWithGain();
    
            node.setProperty (vt::Directory::File::loudness,
                              fileHandler->getLoudness().value(),
                              nullptr);
    
            node.setProperty (vt::Directory::File::peak,
                              fileHandler->getSamplePeak().value(),
                              nullptr);
    
            node.setProperty (vt::Directory::File::has_warning,
                              false,
                              nullptr);
    
            node.setProperty (vt::Directory::File::measured,
                              true,
                              nullptr);
        }
        catch (exc::FileHandler::exception& e)
        {
            MY_LOG_WARNING ("While measuring file {} :\n",
                            file.getFullPathName());

            node.setProperty (vt::Directory::File::has_warning,
                              true,
                              nullptr);

            node.setProperty (vt::Directory::File::err_msg,
                              e.what(),
                              nullptr);

            throw exc::FileHandler::exception(e.getType());
        }
    };

    juce::ValueTree rootFileNode =
        mRoot.getChildWithName(vt::Tree::directory).getChild(0);

    traverse(rootFileNode, callback);
}
void MainProcessor::normailse() noexcept
{
    nodeAction callback = [this](juce::ValueTree node, juce::File file)
    {
        if (!file.existsAsFile()) return;
        if (node.getType() != vt::Directory::file) return;
        if (!node[vt::Directory::File::valid]) return;

        try
        {
            auto fileHandler = std::make_unique<FileHandler>(file);
            if (!fileHandler->isMeasured()) 
                exc::MainProc::get::normalise_before_measurement();

            if (!fileHandler->loadAudio())
                exc::MainProc::get::could_not_load_audio();

            float loudnessDB = fileHandler->getLoudness().value();
            float target = (float)mRoot.getChildWithName(vt::Tree::settings)
                           [vt::Settings::target_lkfs];
            float gainNeeded = target - loudnessDB;

            float wouldBePeak = fileHandler->getSamplePeak().value() *
                                juce::Decibels::decibelsToGain(gainNeeded);
            if (wouldBePeak > 1.f) exc::MainProc::get::would_peak();

            fileHandler->writeWithGain(gainNeeded);

            node.setProperty (vt::Directory::File::loudness,
                              fileHandler->getLoudness().value(),
                              nullptr);

            node.setProperty (vt::Directory::File::peak,
                              fileHandler->getSamplePeak().value(),
                              nullptr);

            node.setProperty (vt::Directory::File::has_warning,
                              false,
                              nullptr);
        }
        catch(const exc::MainProc::exception& e)
        {
            MY_LOG_WARNING ("In file {}:",
                            file.getFullPathName());

            node.setProperty (vt::Directory::File::has_warning,
                              true,
                              nullptr);

            node.setProperty (vt::Directory::File::err_msg,
                              e.what(),
                              nullptr);

            throw exc::MainProc::exception(e.getType());
        }
    };

    juce::ValueTree rootFileNode =
        mRoot.getChildWithName(vt::Tree::directory).getChild(0);

    traverse(rootFileNode, callback);
}
void MainProcessor::setRootDirectory(juce::File directory)
{
    try
    {
        EXPECT_OR_RETURN (directory.isDirectory(),
                          void (),
                          "Selected item is not a directory: {}",
                          directory.getFullPathName());

        auto directoryTree = mRoot.getChildWithName(vt::Tree::directory);
        directoryTree.removeAllChildren(nullptr);
        juce::ValueTree rootNode(vt::Directory::folder);
        directoryTree.appendChild(rootNode, nullptr);
        rootNode.setProperty (vt::Directory::Folder::path,
                              directory.getFullPathName(),
                              nullptr);
        parse();
        countAudioFiles();
    } NORM_CATCH_ALL;
}

void MainProcessor::traverse (juce::ValueTree node, nodeAction callback) noexcept
{
    try
    {
        if(!node.isValid()) exc::MainProc::get::root_dir_unset();
    
        const vt::id type = node.getType();
        const bool isFile = type == vt::Directory::file ||
                            type == vt::Directory::folder;
    
        if (!isFile) exc::MainProc::get::node_not_file();
    
        const vt::id path = type == vt::Directory::file 
            ? vt::Directory::File::path
            : vt::Directory::Folder::path;
    
        juce::File file(node[path].toString());
        callback(node, file);
    }
    catch (const exc::MainProc::exception& e)
    {
        MY_LOG_INFO ("While traversing tree:\n{}",
                     e.what());
        return;
    }
    catch(const std::exception& e)
    {
        MY_LOG_ERROR ("Unexpected error:\n{}",
                      e.what());
        return;
    }

    for (int i = 0; i < node.getNumChildren(); i++)
    {
        traverse(node.getChild(i), callback);
    }
}

bool MainProcessor::canProcessFile(juce::File file)
{
    juce::String extension = file.getFileExtension();
    auto* format = mTestManager.findFormatForFileExtension(extension);
    if (format == nullptr) return false;

    auto reader = std::unique_ptr<juce::AudioFormatReader>(
        mTestManager.createReaderFor(file));
    return reader != nullptr;
}
void MainProcessor::parseFile(juce::ValueTree fileNode, juce::File file)
{
    if (fileNode.getType() != vt::Directory::file ||
        !file.existsAsFile()) 
        exc::MainProc::get::node_not_file();

    using namespace vt::Directory;

    fileNode.setProperty (File::name, file.getFileName(), nullptr);
    fileNode.setProperty (File::valid, canProcessFile(file), nullptr);
    fileNode.setProperty (File::has_warning, false, nullptr);
    fileNode.setProperty (File::selected, true, nullptr);

    if (!fileNode[File::valid])
    {
        fileNode.setProperty (File::err_msg, "File format not supported", nullptr);
        fileNode.setProperty (File::has_warning, true, nullptr);
        return;
    }

    auto fileHandler = std::make_unique<FileHandler>(file);
    fileNode.setProperty (File::measured, fileHandler->isMeasured(), nullptr);
    if (!fileNode[File::measured]) return;

    fileNode.setProperty (File::loudness, fileHandler->getLoudness().value(), nullptr);
    fileNode.setProperty (File::peak, fileHandler->getSamplePeak().value(), nullptr);

    if ((float)fileNode[File::peak] >= 1.f)
    {
        fileNode.setProperty (File::has_warning, true, nullptr);
        fileNode.setProperty (File::err_msg, "Audio is potentially peaking.", nullptr);
    }
}
void MainProcessor::parseDirectory(juce::ValueTree folderNode, juce::File directory)
{
    if (folderNode.getType() != vt::Directory::folder ||
        !directory.isDirectory())
        exc::MainProc::get::node_not_file();

    using namespace vt::Directory;

    folderNode.setProperty (Folder::name, directory.getFileName(), nullptr);

    auto children = directory.findChildFiles(
        juce::File::findFilesAndDirectories,
        false
    );

    for (auto& child : children)
    {
        if (child.existsAsFile())
        {
            juce::ValueTree childFileNode(vt::Directory::file);
            childFileNode.setProperty(File::path, child.getFullPathName(), nullptr);
            folderNode.appendChild(childFileNode, nullptr);
        }
        else if (child.isDirectory())
        {
            juce::ValueTree childFolderNode(vt::Directory::folder);
            childFolderNode.setProperty(Folder::path, child.getFullPathName(), nullptr);
            folderNode.appendChild(childFolderNode, nullptr);
        }
    }
}
void MainProcessor::countAudioFiles()
{
    int numAudioFiles = 0;
    nodeAction callback = [&numAudioFiles](juce::ValueTree node, juce::File file)
    {
        juce::ignoreUnused(file);
        if (node.getType() != vt::Directory::file) return;
        numAudioFiles++;
    };

    juce::ValueTree rootFileNode =
        mRoot.getChildWithName(vt::Tree::directory).getChild(0);

    traverse(rootFileNode, callback);

    juce::ValueTree interface = mRoot.getChildWithName(vt::Tree::interface);
    interface.setProperty(
        vt::Interface::number_of_files,
        numAudioFiles,
        nullptr
    );
}
