/*  Macro definitions used for logging. The macros should be kept the same
    throughout the lifetime of the project, but the implementation could
    be changed anytime, could even be made configurable by some other defs
*/

/*  Todo list:
    * f/#9 : TODO-1
    * not issued : TODO-5
*/

#pragma once

#include <juce_core/juce_core.h>
#include <iostream>
#include <juce_data_structures/juce_data_structures.h>
#include "util/Singleton.h"

#ifndef LOG_LEVEL
    /*
        0: Logging is disabled.
        1: Only errors are logged.
        2: Errors and warnings are logged.
        3: Errors, warnings and info are all logged.
    */
    #define LOG_LEVEL 3
#endif

#ifndef LOG_NUM_DECIMALS
    #define LOG_NUM_DECIMALS 4
#endif

namespace norm
{

template<typename t>
concept Loggable = 
    std::is_arithmetic_v<t> ||
    std::is_same_v<t, juce::String> ||
    std::is_same_v<t, std::string> ||
    std::is_same_v<t, const char*> ||
    std::is_same_v<t, std::nullptr_t>;

class Logger
{
public:

    /*  This class logs nowhere, its just a dummy to derive from.
        Derived classes should implement the log() function.
        Derived classes are recommended to derive from the Singleton class
        as well, found in util/Singleton.h
    */
    class LogDestination
    {
    public:
        virtual void log(juce::String msg) { juce::ignoreUnused(msg); }

    protected:
        LogDestination() = default;
    };

public:
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    ~Logger() noexcept(false);
    static Logger* getInstance();

    template<Loggable t = nullptr_t, 
                Loggable u = nullptr_t, 
                Loggable v = nullptr_t>
    inline void logMessage(const char* type, const char* msg, 
                    t arg1 = nullptr, u arg2 = nullptr, v arg3 = nullptr)
    {
        try
        {
            juce::StringArray args;

            if ( !std::is_same_v<t, std::nullptr_t> )
            {
                args.add(parseArg<t>(arg1));
            }
            if ( !std::is_same_v<u, std::nullptr_t> )
            {
                args.add(parseArg<u>(arg2));
            }
            if ( !std::is_same_v<v, std::nullptr_t> )
            {
                args.add(parseArg<v>(arg3));
            }

            juce::String log_base = juce::String(type) + juce::String(msg);
            juce::String log_message = interpolate(log_base, args);

            broadcastMessage(log_message);
        }
        catch (std::exception& e)
        {
            juce::String err_msg = 
            "Error: Could not process log message:\n";
            err_msg += juce::String(msg) + "\n";
            err_msg += e.what();
    
            broadcastMessage(err_msg);
        }
        //MARK: TODO-1
        //when introducing custom exceptions, parse errors and other errors
        // should be handled differently
    }

    void addListener(LogDestination* listener);
    void removeListener(LogDestination* listener);
    void removaAllListeners();

private:
    Logger() {}

    void broadcastMessage(juce::String msg) const;

    inline static std::unique_ptr<Logger> instance = nullptr;
    std::vector<LogDestination*> listeners;

    static juce::String interpolate(juce::String raw_message, juce::StringArray args);

    template<Loggable t>
    inline static juce::String parseArg(t arg)
    {
        juce::ignoreUnused(arg);
        return juce::String("Error-type");
    }

    template<> juce::String parseArg<int>(int arg);
    template<> juce::String parseArg<float>(float arg);
    template<> juce::String parseArg<double>(double arg);
    template<> juce::String parseArg<std::string>(std::string arg);
    template<> juce::String parseArg<const char*>(const char* arg);
    template<> juce::String parseArg<juce::String>(juce::String arg);

};

class StdLogger final
    : public Logger::LogDestination
    , public Singleton<StdLogger>
{
public:
    void log(juce::String msg) override;
};

}

//==============================================================================

#define ENFORCE_SEMICOLON(statement) do { statement } while (0)

#define __LOG_NOARG(T, MSG)                                                     \
    norm::Logger::getInstance()->logMessage(T, MSG)                             \
    
#define __LOG_1ARG(T, MSG, ARG)                                                 \
    norm::Logger::getInstance()->logMessage(T, MSG, ARG)                        \

#define __LOG_2ARG(T, MSG, ARG1, ARG2)                                          \
    norm::Logger::getInstance()->logMessage(T, MSG, ARG1, ARG2)                 \

#define __LOG_3ARG(T, MSG, ARG1, ARG2, ARG3)                                    \
        norm::Logger::getInstance()->logMessage(T, MSG, ARG1, ARG2, ARG3)       \

#define __GET_LOGGER(T, MSG, OPT_ARG1, OPT_ARG2, OPT_ARG3, NAME, ...) NAME
#define __LOG_MSG(T, ...) __GET_LOGGER                                          \
            (                                                                   \
                T,                                                              \
                __VA_ARGS__,                                                    \
                __LOG_3ARG,                                                     \
                __LOG_2ARG,                                                     \
                __LOG_1ARG,                                                     \
                __LOG_NOARG                                                     \
            )                                                                   \
            (T, __VA_ARGS__)                                                    \

//MARK: TODO-5
// refactor MY_LOG__ to NORM_LOG__ since norm is the project namespace

// Should be used when something could directly or indirectly cause a crash
#if LOG_LEVEL > 0
    #define MY_LOG_ERROR(MSG, ...) __LOG_MSG("Error: ", MSG, __VA_ARGS__)
#elif
    #define MY_LOG_ERRPR(MSG, ...) juce::ignoreUnused(MSG, __VA_ARGS__)
#endif

// Should be used when something isn't neccessarily an error, but could indicate
// that something went wrong
#if LOG_LEVEL > 1
    #define MY_LOG_WARNING(MSG, ...) __LOG_MSG("Warning: ", MSG, __VA_ARGS__)
#else
    #define MY_LOG_WARNING(MSG, ...) juce::ignoreUnused(MSG, __VA_ARGS__)
#endif

// Should be used to log info that does not indicate any problems, but might be
// useful for diagnostics
#if LOG_LEVEL > 2
    #define MY_LOG_INFO(MSG, ...) __LOG_MSG("Info: ", MSG, __VA_ARGS__)
#else
    #define MY_LOG_INFO(MSG, ...) juce::ignoreUnused(MSG, __VA_ARGS__)
#endif

// Expect condition and return if false. Log msg provided in __VA_ARGS__
#define EXPECT_OR_RETURN(COND, VALUE, MSG, ...) ENFORCE_SEMICOLON(              \
    if (!(COND))                                                                \
    {                                                                           \
        MY_LOG_WARNING (MSG, __VA_ARGS__);                                      \
        return VALUE;                                                           \
    })                                                                          \

// Expect condition and throw if false. Log msg provided in __VA_ARGS__
#define EXPECT_OR_THROW(COND, EXCEPT, MSG, ...) ENFORCE_SEMICOLON(              \
    if (!(COND))                                                                \
    {                                                                           \
        MY_LOG_WARNING (MSG, __VA_ARGS__);                                      \
        throw EXCEPT;                                                           \
    })                                                                          \

#define NORM_CATCH_ALL                                                          \
catch(const std::exception& e)                                                  \
{                                                                               \
    MY_LOG_ERROR(                                                               \
        "Unexpected error: {}",                                                 \
        e.what()                                                                \
    );                                                                          \
}                                                                               \
ENFORCE_SEMICOLON()                                                             \
