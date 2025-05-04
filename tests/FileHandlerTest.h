#pragma once

#include <gtest/gtest.h>
#include "util/VTNames.h"
#include "processor/FileHandler.h"
#include "util/Logger.h"

class FileHandlerTest : public testing::Test
{
protected:
    const float eps = 0.05f;
    const float mp3delta = 1.5f / 2.f;
    juce::File workdir;

    void SetUp() override
    {
        auto test_file_dir = juce::File(TEST_AUDIO_DIR);
        workdir = test_file_dir.getSiblingFile("work");
        workdir.deleteRecursively();
        workdir.createDirectory();
        
        auto files = test_file_dir.findChildFiles(
            juce::File::findFiles,
            true);

        for (const auto& file : files)
        {
            if(file.isDirectory()) continue;

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

    void TearDown() override
    {
        workdir = juce::File(TEST_AUDIO_DIR).getSiblingFile("work");
        workdir.deleteRecursively();
        workdir.createDirectory();
    }

};

//==============================================================================
// MEASURE TEST

TEST_F(FileHandlerTest, measure_wav_23lkfs)
{
    auto testfile = workdir.getChildFile("1770-2_Comp_23LKFS_500Hz_2ch.wav");
    ASSERT_TRUE(testfile.existsAsFile());

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->measure();

    const float measured = handler->getLoudness().value();
    const float expected = -23.f;

    EXPECT_GT(measured, expected - eps);
    EXPECT_LT(measured, expected + eps);
}

TEST_F(FileHandlerTest, measure_wav_18lkfs)
{
    auto testfile = workdir.getChildFile("1770-2_Comp_18LKFS_FrequencySweep.wav");
    ASSERT_TRUE(testfile.existsAsFile());

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->measure();

    const float measured = handler->getLoudness().value();
    const float expected = -18.f;

    EXPECT_GT(measured, expected - eps);
    EXPECT_LT(measured, expected + eps);
}

//==============================================================================
// AUDIO FILE WRITE TEST

TEST_F(FileHandlerTest, write_wav_1)
{
    auto testfile = workdir.getChildFile("1770-2 Conf Mono Voice+Music-23LKFS.wav");
    ASSERT_TRUE(testfile.existsAsFile());
    const float original = -23.f;
    const float diff = -6.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->writeWithGain(diff);
    handler = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);

    checker->loadAudio();
    checker->measure();
    const float modified = checker->getLoudness().value();

    EXPECT_GT(modified, original + diff - eps);
    EXPECT_LT(modified, original + diff + eps);
}

TEST_F(FileHandlerTest, write_wav_2)
{
    auto testfile = workdir.getChildFile("1770-2 Conf Mono Voice+Music-24LKFS.wav");
    ASSERT_TRUE(testfile.existsAsFile());
    const float original = -24.f;
    const float diff = -3.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->writeWithGain(diff);
    handler = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);

    checker->loadAudio();
    checker->measure();
    const float modified = checker->getLoudness().value();

    EXPECT_GT(modified, original + diff - eps);
    EXPECT_LT(modified, original + diff + eps);
}

TEST_F(FileHandlerTest, write_mp3_1)
{
    auto testfile = workdir.getChildFile("Koza - Bass - shape 03 - 128kbps.mp3");
    ASSERT_TRUE(testfile.existsAsFile());
    const float diff = -3.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->measure();
    const float original = handler->getLoudness().value();
    handler->writeWithGain(diff);
    handler = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);

    checker->loadAudio();
    checker->measure();
    const float modified = checker->getLoudness().value();

    EXPECT_GT(modified, original + diff - eps - mp3delta);
    EXPECT_LT(modified, original + diff + eps + mp3delta);
}

TEST_F(FileHandlerTest, write_mp3_2)
{
    auto testfile = workdir.getChildFile("Koza - Drumloop - leave me - 192kbps.mp3");
    ASSERT_TRUE(testfile.existsAsFile());
    const float diff = -9.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->measure();
    const float original = handler->getLoudness().value();
    handler->writeWithGain(diff);
    handler = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);

    checker->loadAudio();
    checker->measure();
    const float modified = checker->getLoudness().value();

    EXPECT_GT(modified, original + diff - eps - mp3delta);
    EXPECT_LT(modified, original + diff + eps + mp3delta);
}

TEST_F(FileHandlerTest, write_mp3_3)
{
    auto testfile = workdir.getChildFile("Koza - SFX - atmo - wonder (E) - 320kbps.mp3");
    ASSERT_TRUE(testfile.existsAsFile());
    const float diff = +5.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->measure();
    const float original = handler->getLoudness().value();
    handler->writeWithGain(diff);
    handler = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);

    checker->loadAudio();
    checker->measure();
    const float modified = checker->getLoudness().value();

    EXPECT_GT(modified, original + diff - eps - mp3delta);
    EXPECT_LT(modified, original + diff + eps + mp3delta);
}

//==============================================================================
// METADATA TEST

TEST_F(FileHandlerTest, tag_store_wav)
{
    auto testfile = workdir.getChildFile("1770-2_Comp_24LKFS_10000Hz_2ch.wav");
    ASSERT_TRUE(testfile.existsAsFile());
    const float expected = -24.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->measure();
    handler->writeWithGain();
    handler = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);

    ASSERT_TRUE(checker->isMeasured());
    const float cached = checker->getLoudness().value();

    EXPECT_GT(cached, expected - eps);
    EXPECT_LT(cached, expected + eps);
}

TEST_F(FileHandlerTest, tag_store_mp3)
{
    auto testfile = workdir.getChildFile("Koza - SFX - atmo - wonder (E) - 320kbps.mp3");
    ASSERT_TRUE(testfile.existsAsFile());

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);

    handler->loadAudio();
    handler->measure();
    const float expected = handler->getLoudness().value();
    handler->writeWithGain();
    handler = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);

    ASSERT_TRUE(checker->isMeasured());
    const float cached = checker->getLoudness().value();

    EXPECT_GT(cached, expected - eps);
    EXPECT_LT(cached, expected + eps);
}

TEST_F(FileHandlerTest, tag_use_wav)
{
    auto testfile = workdir.getChildFile("1770-2 Conf Mono Voice+Music-23LKFS.wav");
    ASSERT_TRUE(testfile.existsAsFile());
    const float original = -23.f;
    const float diff = -6.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);
    handler->loadAudio();
    handler->measure();
    handler->writeWithGain();
    handler = nullptr;

    auto amp = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(amp);
    ASSERT_TRUE(amp->isMeasured());
    amp->loadAudio();
    amp->writeWithGain(diff);
    amp = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);
    checker->loadAudio();
    checker->measure();
    const float modified = checker->getLoudness().value();

    EXPECT_GT(modified, original + diff - eps);
    EXPECT_LT(modified, original + diff + eps);
}

TEST_F(FileHandlerTest, tag_use_mp3)
{
    auto testfile = workdir.getChildFile("Koza - Drumloop - leave me - 192kbps.mp3");
    ASSERT_TRUE(testfile.existsAsFile());
    const float diff = -9.f;

    auto handler = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(handler);
    handler->loadAudio();
    handler->measure();
    const float original = handler->getLoudness().value();
    handler->writeWithGain();
    handler = nullptr;

    auto amp = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(amp);
    ASSERT_TRUE(amp->isMeasured());
    amp->loadAudio();
    amp->writeWithGain(diff);
    amp = nullptr;

    auto checker = std::make_unique<norm::FileHandler>(testfile);
    ASSERT_TRUE(checker);
    checker->loadAudio();
    checker->measure();
    const float modified = checker->getLoudness().value();

    EXPECT_GT(modified, original + diff - eps - mp3delta);
    EXPECT_LT(modified, original + diff + eps + mp3delta);
}
