/*  Macro definitions used for logging. The macros should be kept the same
    throughout the lifetime of the project, but the implementation could
    be changed anytime, could even be made configurable by some other defs
*/

#pragma once

#include <juce_core/juce_core.h>

#define ENFORCE_SEMICOLON(statement) do { statement } while (0)

// Should be used when something could directly or indirectly cause a crash
#define MY_LOG_ERROR(MSG, ...) ENFORCE_SEMICOLON(juce::ignoreUnused(MSG,  __VA_ARGS__);)

// Should be used when something isn't neccessarily an error, but could indicate that something went wrong
#define MY_LOG_WARNING(MSG, ...) ENFORCE_SEMICOLON(juce::ignoreUnused(MSG, __VA_ARGS__);)

// Should be used to log info that does not indicate any problems, but might be useful for diagnostics
#define MY_LOG_INFO(MSG, ...) ENFORCE_SEMICOLON(juce::ignoreUnused(MSG, __VA_ARGS__);)


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
