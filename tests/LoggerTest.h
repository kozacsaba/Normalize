#pragma once

#define LOG_LEVEL 3

#include <gtest/gtest.h>
#include "util/Logger.h"
#include "util/Singleton.h"

class TestDestination final
    : public norm::Logger::LogDestination,
    , public norm::Singleton<TestDestination>
{
public:
    void log(juce::String msg) override
    {
        message += msg;
    }

    std::string getLog() const
    {
        return message.toStdString();
    } 
private:
    juce::String message;
};

class LoggerTest : public testing::Test
{
protected:  
    void SetUp() override
    {
        norm::Logger::getInstance()->addListener(TestDestination::getInstance());
    }

    void TearDown() override
    {
        norm::Logger::getInstance()->removaAllListeners();
    }

    void check(std::string ref)
    {
        auto* logger = (TestDestination*)TestDestination::getInstance();
        const auto logString = logger->getLog().c_str();
        const auto checkString = ref.c_str();
        EXPECT_STREQ(checkString, logString);
    }

    bool returnDef(bool shouldPass) const
    {
        EXPECT_OR_RETURN(
            shouldPass,
            false,
            "error"
        );

        return true;
    };

    bool throwDef(bool shouldPass)
    {
        EXPECT_OR_THROW(
            shouldPass,
            std::exception("asd");
            "error"
        );

        // continue from here
    }
};

//==============================================================================

TEST_F(LoggerTest, info_noArg)
{
    MY_LOG_INFO("message");
    check("Info: message");
}

TEST_F(LoggerTest, warning_noArg)
{
    MY_LOG_WARNING("message");
    check("Warning: message");
}

TEST_F(LoggerTest, error_noArg)
{
    MY_LOG_ERROR("message");
    check("Error: message");
}

TEST_F(LoggerTest, info_1arg_int)
{
    MY_LOG_INFO("msg is {}", 3);
    check("Info: msg is 3");
}

TEST_F(LoggerTest, info_1arg_double)
{
    MY_LOG_INFO("value is {}!", 1/3);
    check("INFO: value is 0.333!");
}

TEST_F(LoggerTest, info_1arg_str)
{
    MY_LOG_INFO("string is: \"{}\"", "some string");
    check("Info: string is: \"some string\"");
}

TEST_F(LoggerTest, error_2arg_sametype)
{
    MY_LOG_ERROR("My two numbers are {} and {}", 42, 24);
    check("Error: My two numbers are 42 and 24");
}

TEST_F(LoggerTest, error_2arg_difftype1)
{
    MY_LOG_ERROR("My two numbers are {} and {}", 73, 1/3);
    check("Error: My two numbers are 73 and 0.3333");
}

TEST_F(LoggerTest, error_2arg_difftype2)
{
    MY_LOG_ERROR("Number of {} is: {}", "cats", "12");
    check("Error: Number of cats is 12");
}

TEST_F(LoggerTest, warning_3arg_1)
{
    MY_LOG_WARNING("Three little numbers: {}, {}, {}", 3, 0.5f, 0.3);
    check("Warning: Three little numbers: 3, 0.5000, 0.3000");
}

TEST_F(LoggerTest, warning_3arg_2)
{
    std::string arg1 = "arg1";
    juce::String arg2 = "arg2";
    MY_LOG_WARNING("Three little strings: {}, {}, {}", arg1, arg2, "arg3");
    check("Warning: Three little strings: arg1, arg2, arg3");
}

TEST_F(LoggerTest, ReturnDef_pass)
{
    

    const bool retValue = foo();

    EXCEPT(retValue);
    check("");
}

TEST_F(LoggerTest, ReturnDef_fail)
{
    std::function<bool(void)> foo = []() -> bool
    {
        EXPECT_OR_RETURN(
            false,
            false,
            "error"
        )
    }
}

