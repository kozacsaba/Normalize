/*  Macro definitions used for logging. The macros should be kept the same
    throughout the lifetime of the project, but the implementation could
    be changed anytime, could even be made configurable by some other defs
*/

#pragma once

#include <juce_core/juce_core.h>
#include <iostream>
#include <juce_data_structures/juce_data_structures.h>

#ifndef LOG_LEVEL
    #define LOG_LEVEL 3
#endif

namespace norm
{
    template<typename t>
    concept Loggable = 
        std::is_arithmetic_v<t> ||
        std::is_same_v<t, juce::String> ||
        std::is_same_v<t, std::string> ||
        std::is_same_v<t, std::string_view> ||
        std::is_same_v<t, std::nullptr_t>;


    class Logger : juce::ThreadPool
    {
    private:
        inline static const int THREAD_KILL_TIMEOUT_MS = 500;
        inline static const juce::String LOG_THREAD_NAME = "Log_Thread";

    public:
        Logger(const Logger&) = delete;
        Logger(Logger&&) = delete;
        Logger& operator=(const Logger&) = delete;
        ~Logger()
        {
            StdLogger* stdLogger = StdLogger::getInstance();
            removeListener(stdLogger);
        }
        static Logger* getInstance()
        {
            if(instance == nullptr)
            {
                auto options = juce::ThreadPoolOptions {}
                    .withThreadName(LOG_THREAD_NAME)
                    .withNumberOfThreads(1);
                instance = std::unique_ptr<Logger>(new Logger(options));
            }
            
            return instance.get();
        }

        template<Loggable t, Loggable u, Loggable v>
        void logMessage(std::string_view type, 
                        std::string_view msg, 
                        t arg1 = nullptr, 
                        u arg2 = nullptr, 
                        v arg3 = nullptr)
        {
            if (juce::Thread::getCurrentThread()->getThreadName() !=
                LOG_THREAD_NAME)
            {
                addJob([&,this](){
                    logMessage(type, msg, arg1, arg2, arg3);
                });
                return;
            }

            try
            {
                log_internal(type, msg, arg1, arg2, arg3);
            }
            catch (std::exception& e)
            {
                juce::String err_msg = "Error: Could not process log message:\n";
                err_msg += juce::String(msg.data()) + "\n";
                err_msg += e.what();

                latestLogString = err_msg;
            }
        }

        void addListener(juce::Value::Listener* listener)
        {
            latestLogString.addListener(listener);
        }
        void removeListener(juce::Value::Listener* listener)
        {
            latestLogString.removeListener(listener);
        }

    private:
        Logger(juce::ThreadPoolOptions options)
            : ThreadPool(options)
        {
            StdLogger* stdLogger = StdLogger::getInstance();
            addListener(stdLogger);
        }

        template<Loggable t, Loggable u, Loggable v>
        void log_internal(std::string_view type, 
                          std::string_view msg, 
                          t arg1 = nullptr, 
                          u arg2 = nullptr, 
                          v arg3 = nullptr)
        {
            juce::StringArray args;

            if ( !std::is_same_v<t, std::nullptr_t> )
            {
                parseArg<t>(args, arg1);
            }
            if ( !std::is_same_v<u, std::nullptr_t> )
            {
                parseArg<u>(args, arg2);
            }
            if ( !std::is_same_v<v, std::nullptr_t> )
            {
                parseArg<v>(args, arg3);
            }

            juce::String log_message;
            auto raw_message = juce::String(type.data());
            raw_message += juce::String(msg.data());
            for (int i = 0; i < args.size(); i++)
            {
                int parseIndex = raw_message.indexOf("{}");
                if (parseIndex == -1)
                {
                    throw std::exception("Too many arguments in log message.");
                }

                log_message.append(raw_message.substring(0, parseIndex), 
                                   (size_t) raw_message.length());
                auto arg = juce::String(args[i]);
                log_message.append(arg, (size_t) arg.length());
                raw_message = raw_message.substring(parseIndex + 1, 
                                                    raw_message.length());
            }

            log_message.append(raw_message, (size_t) raw_message.length());
            
            latestLogString = log_message;
        }

        template <Loggable t>
        void parseArg(juce::StringArray& args, t arg)
        {
            juce::ignoreUnused(arg);
            args.add("Error-type");
        }

        template<>
        void parseArg<int>(juce::StringArray& args, int arg)
        {
            args.add(juce::String(arg));
        }

        template<>
        void parseArg<float>(juce::StringArray& args, float arg)
        {
            args.add(juce::String(arg));
        }

        template<>
        void parseArg<double>(juce::StringArray& args, double arg)
        {
            args.add(juce::String(arg));
        }

        template<>
        void parseArg<std::string>(juce::StringArray& args, std::string arg)
        {
            args.add(juce::String(arg));
        }

        template<>
        void parseArg<std::string_view>(juce::StringArray& args, std::string_view arg)
        {
            args.add(juce::String(std::string(arg.data())));
        }

        template<>
        void parseArg<juce::String>(juce::StringArray& args, juce::String arg)
        {
            args.add(arg);
        }

        inline static std::unique_ptr<Logger> instance = nullptr;
        juce::Value latestLogString;

        class StdLogger : public juce::Value::Listener
        {
        public:
            StdLogger(const StdLogger&) = delete;
            StdLogger(StdLogger&&) = delete;
            StdLogger& operator=(const StdLogger&) = delete;
            static StdLogger* getInstance()
            {
                if (instance == nullptr)
                {
                    instance = std::unique_ptr<StdLogger>(new StdLogger);
                }

                return instance.get();
            }
    
            void valueChanged(juce::Value& value)
            {
                std::cerr << value.toString() << std::endl;
            }
    
        private:
            inline static std::unique_ptr<StdLogger> instance = nullptr; 
            StdLogger() = default;
        };
    };
}

//==============================================================================

#define ENFORCE_SEMICOLON(statement) do { statement } while (0)

#define __LOG_NOARG(T, MSG)                                                     \
    norm::Logger::getInstance()->logMessage(T, MSG, nullptr, nullptr, nullptr)  \
    
#define __LOG_1ARG(T, MSG, ARG)                                                 \
    norm::Logger::getInstance()->logMessage(T, MSG, ARG, nullptr, nullptr)      \

#define __LOG_2ARG(T, MSG, ARG1, ARG2)                                          \
    norm::Logger::getInstance()->logMessage(T, MSG, ARG1, ARG2, nullptr)        \

#define __LOG_3ARG(T, MSG, ARG1, ARG2, ARG3)                                    \
        norm::Logger::getInstance()->logMessage(T, MSG, ARG1, ARG2, ARG3)       \

#define __GET_LOGGER(T, MSG, OPT_ARG1, OPT_ARG2, OPT_ARG3, NAME, ...) NAME
#define __LOG_MSG(T, ...) __GET_LOGGER(T, __VA_ARGS__, __LOG_3ARG, __LOG_2ARG, __LOG_1ARG, __LOG_NOARG) (T, __VA_ARGS__)

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

