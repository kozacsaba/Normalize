#pragma once

#include <gtest/gtest.h>
#include "util/VTNames.h"
#include "processor/FileHandler.h"
#include "util/Logger.h"

class FileHandlerTest : public testing::Test
{
protected:
    const float eps = 0.05f;
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

TEST_F(FileHandlerTest, Measure_file_1)
{
    auto file = workdir.getChildFile("1770-2 Conf Mono Voice+Music-23LKFS.wav");
    auto handler = std::make_unique<norm::FileHandler>(file);

    handler->loadAudio();
    handler->measure();
    ASSERT_TRUE(handler->isMeasured());

    const float m_val = handler->getLoudness().value();
    const float e_val = -23.f;

    EXPECT_GT(m_val, e_val - eps);
    EXPECT_LT(m_val, e_val + eps);
}

TEST_F(FileHandlerTest, Measure_file_2)
{
    auto file = workdir.getChildFile("1770-2 Conf Stereo VinL+R-24LKFS.wav");
    auto handler = std::make_unique<norm::FileHandler>(file);

    handler->loadAudio();
    handler->measure();
    ASSERT_TRUE(handler->isMeasured());

    const float m_val = handler->getLoudness().value();
    const float e_val = -24.f;

    EXPECT_GT(m_val, e_val - eps);
    EXPECT_LT(m_val, e_val + eps);
}

TEST_F(FileHandlerTest, Write_Read)
{
    auto file = workdir.getChildFile("Koza - Bass - shape 03 - 128kbps.mp3");
    auto handler_1 = std::make_unique<norm::FileHandler>(file);
    ASSERT_FALSE(handler_1->isMeasured());

    handler_1->loadAudio();
    handler_1->measure();
    ASSERT_TRUE(handler_1->isMeasured());
    const float m_val_1 = handler_1->getLoudness().value();
    handler_1->writeFile();

    // destroy handler instance
    handler_1.reset();

    auto handler_2 = std::make_unique<norm::FileHandler>(file);
    ASSERT_TRUE(handler_2->isMeasured());
    const float m_val_2 = handler_2->getLoudness().value();

    ASSERT_FLOAT_EQ(m_val_1, m_val_2);
}

TEST_F(FileHandlerTest, Att10_WR)
{
    const float attenuation = 10.f;
    auto file = workdir.getChildFile("Koza - Drumloop - leave me - 192kbps.mp3");
    auto handler_1 = std::make_unique<norm::FileHandler>(file);
    ASSERT_FALSE(handler_1->isMeasured());

    handler_1->loadAudio();
    handler_1->measure();
    ASSERT_TRUE(handler_1->isMeasured());
    const float m_val_1 = handler_1->getLoudness().value();
    handler_1->applyGainDecibel(-attenuation);
    handler_1->writeFile();

    // destroy handler instance
    handler_1.reset();

    auto handler_2 = std::make_unique<norm::FileHandler>(file);
    ASSERT_TRUE(handler_2->isMeasured());
    const float m_val_2 = handler_2->getLoudness().value();

    EXPECT_FLOAT_EQ(m_val_1-attenuation, m_val_2);

    handler_2->loadAudio();
    handler_2->measure();
    const float m_val_2_measured = handler_2->getLoudness().value();

    EXPECT_GT(m_val_2_measured, m_val_2 - eps);
    EXPECT_LT(m_val_2_measured, m_val_2 + eps);
}

TEST_F(FileHandlerTest, Att18_WR)
{
    const float attenuation = 18.f;
    auto file = workdir.getChildFile("Koza - SFX - atmo - wonder (E) - 320kbps.mp3");
    auto handler_1 = std::make_unique<norm::FileHandler>(file);
    ASSERT_FALSE(handler_1->isMeasured());

    handler_1->loadAudio();
    handler_1->measure();
    ASSERT_TRUE(handler_1->isMeasured());
    const float m_val_1 = handler_1->getLoudness().value();
    handler_1->applyGainDecibel(-attenuation);
    handler_1->writeFile();

    // destroy handler instance
    handler_1.reset();

    auto handler_2 = std::make_unique<norm::FileHandler>(file);
    ASSERT_TRUE(handler_2->isMeasured());
    const float m_val_2 = handler_2->getLoudness().value();

    EXPECT_FLOAT_EQ(m_val_1-attenuation, m_val_2);

    handler_2->loadAudio();
    handler_2->measure();
    const float m_val_2_measured = handler_2->getLoudness().value();

    EXPECT_GT(m_val_2_measured, m_val_2 - eps);
    EXPECT_LT(m_val_2_measured, m_val_2 + eps);
}
