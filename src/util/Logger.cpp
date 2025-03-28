/*  TODO LIST 
    * f/#9 TODO-4 TODO-3
*/

#include "Logger.h"

using namespace norm;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Logger::~Logger() noexcept(false)
{
    if (listeners.size() > 0)
    {
        //MARK: TODO-4
        // custom exception
        throw std::exception("Listeners are not unregistered");
    }
}

Logger* Logger::getInstance()
{
    if(instance == nullptr)
    {
        instance = std::unique_ptr<Logger>(new Logger());
    }
    
    return instance.get();
}

void Logger::addListener(LogDestination* listener)
{
    listeners.push_back(listener);
}

void Logger::removeListener(LogDestination* listener)
{
    for (size_t i = 0; i < listeners.size(); i++)
    {
        if(listeners[i] == listener)
        {
            listeners.erase(listeners.begin() + (ptrdiff_t)i);
            return;
        }
    }
}

void Logger::removaAllListeners()
{
    listeners.clear();
}

void Logger::broadcastMessage(juce::String msg) const
{
    for (auto& listener : listeners)
    {
        listener->log(msg);
    }
}

juce::String Logger::interpolate(juce::String raw_message, juce::StringArray args)
{
    juce::String log_message;

    for (int i = 0; i < args.size(); i++)
    {
        int parseIndex = raw_message.indexOf("{}");
        if (parseIndex == -1)
        {
            //MARK: TODO-3
            // custom exception
            throw std::exception("Too many arguments in log message.");
        }

        log_message.append(raw_message.substring(0, parseIndex), 
                            (size_t) raw_message.length());
        auto arg = juce::String(args[i]);
        log_message.append(arg, (size_t) arg.length());
        raw_message = raw_message.substring(parseIndex + 2, 
                                            raw_message.length());
    }

    log_message.append(raw_message, (size_t)raw_message.length());
    
    if(!log_message.endsWith("\n"))
    {
        log_message += "\n";
    }

    return log_message;
}

template<>
juce::String Logger::parseArg<int>(int arg)
{
    return juce::String(arg);
}

template<>
juce::String Logger::parseArg<float>(float arg)
{
    return juce::String(arg, LOG_NUM_DECIMALS, false);
}

template<>
juce::String Logger::parseArg<double>(double arg)
{
    return juce::String(arg, LOG_NUM_DECIMALS, false);
}

template<>
juce::String Logger::parseArg<std::string>(std::string arg)
{
    return juce::String(arg);
}

template<>
juce::String Logger::parseArg<const char*>(const char* arg)
{
    return juce::String(arg);
}

template<>
juce::String Logger::parseArg<juce::String>(juce::String arg)
{
    return arg;
}

//==============================================================================

void StdLogger::log(juce::String msg)
{
    std::cerr << msg.toStdString() << std::endl;   
}
