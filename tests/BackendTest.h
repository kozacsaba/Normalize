/*  This is a comprehensive test suite for the integrated process of measuring
    audio file loudness, adjusting levels, and writing metadata. It depends on
    the correct behavior of multiple modules, which are tested sequentially with
    incremental integration.
*/

#pragma once

#include <gtest/gtest.h>
#include "util/VTNames.h"
#include "processor/MainProcessor.h"

// TODO: 
//  * write tests for sync -> parsing and measuring files in file structure
//  * write tests for adjusting loudness to required value - and then
//    measure to check if it was successful
//  * write tests for integrating the two processes
//  * also test if the loudness tags were added correctly
//  * test for peaking

class BackendTest : public testing::Test
{
protected:
    const float eps = 0.05f;

    inline static char* f_23lkfs = "23LKFS";
    inline static char* f_24lkfs = "24LKFS";
    inline static char* f_pdf = "PDF";

private:
    void prepareWorkbench()
    {
        workdir = juce::File(TEST_AUDIO_DIR).getSiblingFile("work");
        workdir.deleteRecursively();
        workdir.createDirectory();

        juce::StringArray folders = {f_23lkfs, f_24lkfs, f_pdf};
        auto test_file_dir = juce::File(TEST_AUDIO_DIR);

        for (const auto& foldername : folders)
        {
            auto folder = workdir.getChildFile(foldername);
            folder.createDirectory();

            juce::String wildcard = "*" + foldername + "*";
            auto files = test_file_dir.findChildFiles(
                juce::File::findFiles,
                false,
                wildcard
            );

            for (const auto& file : files)
            {
                auto filename = file.getFileName();
                auto destination = workdir.getChildFile(filename);
                bool copiedSuccessfully = file.copyFileTo(destination);

                if(!copiedSuccessfully)
                {
                    MY_LOG_WARNING(
                        "Could not copy file \"{}\" from \"{}\" to \"{}\"",
                        file.getFileName(),
                        file.getParentDirectory().getFullPathName(),
                        destination.getParentDirectory().getFullPathName()
                    );
                }
            }
        }
    }

    void cleanupWorkbench()
    {
        workdir = juce::File(TEST_AUDIO_DIR).getSiblingFile("work");
        workdir.deleteRecursively();
        workdir.createDirectory();
    }

    void checkUniformLoudness (juce::ValueTree directory, 
                               float targetValue, 
                               bool searchRecursively = false,
                               bool ignoreUnprocessed = false)
    {
        ASSERT_TRUE(directory.getType() == norm::vt::Directory::folder);

        for (int i = 0; i < directory.getNumChildren(); i++)
        {
            juce::ValueTree child = directory.getChild(i);
            if (child.getType() == norm::vt::Directory::folder)
            {
                if(searchRecursively) 
                    checkUniformLoudness(child, targetValue, true);

                continue;
            }
            
            juce::var isProcessedVar = 
                child.getProperty(norm::vt::Directory::File::processed);

            const bool isProcessed = 
                isProcessedVar.isBool() && (bool)isProcessed;

            EXPECT_TRUE(isProcessed || ignoreUnprocessed);

            if(isProcessed)
            {
                const float loudness = 
                    child.getProperty(norm::vt::Directory::File::loudness);

                EXPECT_GT(loudness, targetValue - eps);
                EXPECT_LT(loudness, targetValue + eps);
            }
        }
    }

protected:
    void SetUp() override
    {
        prepareWorkbench();
        vtRoot = norm::vt::buildValueTree();
        mainProc = std::make_unique<norm::MainProcessor>(vtRoot);
        mainProc->setRootDirectory(workdir);
    }

    void TearDown() override
    {
        cleanupWorkbench();
        mainProc.reset();
    }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    bool call_canProcessFile(juce::File file) 
    { 
        return mainProc->canProcessFile(file); 
    }

    void call_parseFile(juce::File file, juce::ValueTree fileNode)
    {
        mainProc->parseFile(file, fileNode);
    }

    void call_parseDirectory(juce::File directory, juce::ValueTree folderNode)
    {
        mainProc->parseDirectory(directory, folderNode);
    }

    int call_countAudioFiles()
    {
        return mainProc->countAudioFiles();
    }

    void call_processFile(juce::ValueTree fileNode)
    {
        mainProc->processFile(fileNode);
    }

    void call_processDirectory(juce::ValueTree directoryNode)
    {
        mainProc->processDirectory(directoryNode);
    }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    juce::ValueTree vtRoot;
    std::unique_ptr<norm::MainProcessor> mainProc;
    juce::File workdir;

};

//==============================================================================

TEST_F(BackendTest, canProcessFile_wav)
{
    auto wav_files = workdir.getChildFile(f_23lkfs).findChildFiles(
        juce::File::findFiles,
        false,
        "*.wav");
    
    for (const auto& file : wav_files)
    {
        juce::String filename = file.getFileName();
        const bool canBeProcessed = call_canProcessFile(file);
        if(!canBeProcessed) MY_LOG_ERROR(
            "Validating file \"{}\" failed (false negative):",
            filename
        );
        EXPECT_TRUE(canBeProcessed);
    }
}

TEST_F(BackendTest, canProcessFile_pdf)
{
    auto pdf_files = workdir.getChildFile(f_pdf).findChildFiles(
        juce::File::findFiles,
        false,
        "*.pdf");
    
    for (const auto& file : pdf_files)
    {
        juce::String filename = file.getFileName();
        const bool canBeProcessed = call_canProcessFile(file);
        if(canBeProcessed) MY_LOG_ERROR(
            "Validating file \"{}\" failed (false positive):",
            filename
        );
        EXPECT_FALSE(canBeProcessed);
    }
}

// MARK: TODO
TEST_F(BackendTest, canProcessFile_mp3)
{
    // needs an mp3 file
    EXPECT_TRUE(true);
}

// void parseFile(juce::File file, juce::ValueTree fileNode);
// parsefile needs to be tested before AND after processing

TEST_F(BackendTest, parseFile_unprocessed_wav)
{
    juce::ValueTree fileNode(norm::vt::Directory::file);
    juce::File audioFile = 
        workdir                                         // work folder <juce::File>
        .getChildFile(f_23lkfs)                         // subfolder <juce::File>
        .findChildFiles(juce::File::findFiles, false)   // files <juce::Array<juce::File>>
        [0];                                            // first file in array <juce::File>
    
    call_parseFile(audioFile, fileNode);

    const std::string exp_path = audioFile.getParentDirectory().getFullPathName().toStdString();
    const std::string res_path = fileNode.getProperty(norm::vt::Directory::File::path).toString().toStdString();
    EXPECT_STREQ(exp_path.c_str(), res_path.c_str());

    const std::string exp_name = audioFile.getFileName().toStdString();
    const std::string res_name = fileNode.getProperty(norm::vt::Directory::File::name).toString().toStdString();
    EXPECT_STREQ(exp_name.c_str(), res_name.c_str());

    const bool res_valid = (bool)fileNode.getProperty(norm::vt::Directory::File::valid);
    EXPECT_TRUE(res_valid);

    const bool res_processed = (bool)fileNode.getProperty(norm::vt::Directory::File::processed);
    EXPECT_FALSE(res_processed);
}

TEST_F(BackendTest, parseFile_unprocessed_pdf)
{
    juce::ValueTree fileNode(norm::vt::Directory::file);
    juce::File audioFile = 
        workdir                                         // work folder <juce::File>
        .getChildFile(f_pdf)                            // subfolder <juce::File>
        .findChildFiles(juce::File::findFiles, false)   // files <juce::Array<juce::File>>
        [0];                                            // first file in array <juce::File>
    
    call_parseFile(audioFile, fileNode);

    const std::string exp_path = audioFile.getParentDirectory().getFullPathName().toStdString();
    const std::string res_path = fileNode.getProperty(norm::vt::Directory::File::path).toString().toStdString();
    EXPECT_STREQ(exp_path.c_str(), res_path.c_str());

    const std::string exp_name = audioFile.getFileName().toStdString();
    const std::string res_name = fileNode.getProperty(norm::vt::Directory::File::name).toString().toStdString();
    EXPECT_STREQ(exp_name.c_str(), res_name.c_str());

    const bool res_valid = (bool)fileNode.getProperty(norm::vt::Directory::File::valid);
    EXPECT_FALSE(res_valid);

    juce::var err_msg_var = fileNode.getProperty(norm::vt::Directory::File::err_msg);
    EXPECT_TRUE(err_msg_var.isString());
    const std::string res_err_msg = err_msg_var.toString().toStdString();
    EXPECT_STRNE(res_err_msg.c_str(), "");
}

// void parseDirectory(juce::File directory, juce::ValueTree folderNode);
// parse dir needs depends on parsefile

TEST_F(BackendTest, parseDirectory_unprocessed)
{
    juce::ValueTree folderNode (norm::vt::Directory::folder);
    call_parseDirectory(workdir, folderNode);

    // check root folder

    const std::string res_path = 
        folderNode.getProperty(norm::vt::Directory::Folder::path)
                  .toString()
                  .toStdString();
    const std::string exp_path = workdir.getFullPathName().toStdString();
    EXPECT_STREQ(res_path.c_str(), exp_path.c_str());

    const std::string res_name = 
        folderNode.getProperty(norm::vt::Directory::Folder::name)
                  .toString()
                  .toStdString();
    const std::string exp_name = workdir.getFileName().toStdString();
    EXPECT_STREQ(res_name.c_str(), exp_name.c_str());

    // check subfolder

    auto subfolderNode = folderNode.getChildWithName(norm::vt::Directory::folder);

    const std::string subf_res_name = 
        subfolderNode.getProperty(norm::vt::Directory::Folder::name)
                     .toString()
                     .toStdString();
    auto subfolderFile = workdir.getChildFile(subf_res_name);

    const std::string subf_res_path = 
    subfolderNode.getProperty(norm::vt::Directory::Folder::path)
                 .toString()
                 .toStdString();
    const std::string subf_exp_path = subfolderFile.getFullPathName().toStdString();
    EXPECT_STREQ(subf_res_path.c_str(), subf_exp_path.c_str());

    // check a file

    auto fileNode = subfolderNode.getChildWithName(norm::vt::Directory::file);

    const std::string file_res_name = 
        fileNode.getProperty(norm::vt::Directory::File::name)
                .toString()
                .toStdString();
    auto file = subfolderFile.getChildFile(file_res_name);

    const std::string file_res_path = 
        fileNode.getProperty(norm::vt::Directory::File::path)
                .toString()
                .toStdString();
    const std::string file_exp_path = 
        file.getParentDirectory()
            .getFullPathName()
            .toStdString();
    EXPECT_STREQ(file_res_path.c_str(), file_exp_path.c_str());

    const bool file_res_valid = fileNode.getProperty(norm::vt::Directory::File::valid);
    const bool file_exp_valid = call_canProcessFile(file);
    EXPECT_TRUE(file_res_valid == file_exp_valid);
}


TEST_F(BackendTest, countAudioFiles_pdf)
{
    const int exp_num_files = 1;

    auto pdf_dir = workdir.getChildFile(f_pdf);
    mainProc->setRootDirectory(pdf_dir);
    mainProc->resync();
    const int num_files = call_countAudioFiles();

    EXPECT_EQ(exp_num_files, num_files);
}

TEST_F(BackendTest, countAudioFiles_23lkfs)
{
    const int exp_num_files = 8;

    auto f_23_dir = workdir.getChildFile(f_23lkfs);
    mainProc->setRootDirectory(f_23_dir);
    mainProc->resync();
    const int num_files = call_countAudioFiles();

    EXPECT_EQ(exp_num_files, num_files);
}

TEST_F(BackendTest, countAudioFiles_24lkfs)
{
    const int exp_num_files = 8;

    auto f_24_dir = workdir.getChildFile(f_24lkfs);
    mainProc->setRootDirectory(f_24_dir);
    mainProc->resync();
    const int num_files = call_countAudioFiles();

    EXPECT_EQ(exp_num_files, num_files);
}

TEST_F(BackendTest, countAudioFiles_root_non_recur)
{
    const int exp_num_files = 0;

    vtRoot.getChildWithName(norm::vt::Tree::settings)
        .setProperty (norm::vt::Settings::recursive_search, 
                        false, // true by default
                        nullptr);
    mainProc->resync();

    const int num_files = call_countAudioFiles();

    EXPECT_EQ(exp_num_files, num_files);
}

TEST_F(BackendTest, countAudioFiles_root_recursive)
{
    const int exp_num_files = 16;

    mainProc->resync();
    const int num_files = call_countAudioFiles();

    EXPECT_EQ(exp_num_files, num_files);
}


TEST_F(BackendTest, processFile_wav_23to18)
{
    auto file = workdir.getChildFile(f_23lkfs).findChildFiles(
        juce::File::findFiles, false, "*.wav")[0];

    juce::ValueTree node(norm::vt::Directory::file);

    call_parseFile(file, node);

    call_processFile(node);

    // check if loudness was set correctly

    // cq/#17 might need to be implemented sooner than i tought
    // main processor should probably seperate measuring and amping files
}


// void processFile(juce::ValueTree fileNode);
// this is also done based on valuetree nodes, so we have to parse first



// void processDirectory(juce::ValueTree directoryNode);


// void beginProcessing();
 
