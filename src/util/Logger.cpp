/*  TODO LIST 
    * f/#9 TODO-4
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


template<>
void Logger::parseArg<int>(juce::StringArray& args, int arg)
{
    args.add(juce::String(arg));
}

template<>
void Logger::parseArg<float>(juce::StringArray& args, float arg)
{
    args.add(juce::String(arg, 4, false));
}

template<>
void Logger::parseArg<double>(juce::StringArray& args, double arg)
{
    args.add(juce::String(arg, 4, false));
}

template<>
void Logger::parseArg<std::string>(juce::StringArray& args, std::string arg)
{
    args.add(juce::String(arg));
}

template<>
void Logger::parseArg<const char*>(juce::StringArray& args, const char* arg)
{
    args.add(juce::String(arg));
}

template<>
void Logger::parseArg<juce::String>(juce::StringArray& args, juce::String arg)
{
    args.add(arg);
}

//==============================================================================

void StdLogger::log(juce::String msg)
{
    std::cerr << msg.toStdString() << std::endl;   
}

StdLogger::StdLogger()
    : LogDestination() {}

