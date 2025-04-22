#if defined(_MSC_VER)
    #define SUPPRESS_WARNINGS_BEGIN __pragma(warning(push, 0))
    #define SUPPRESS_WARNINGS_END   __pragma(warning(pop))

#elif defined(__clang__)
    #define SUPPRESS_WARNINGS_BEGIN               \
        _Pragma("clang diagnostic push")          \
        _Pragma("clang diagnostic ignored \"-Weverything\"")

    #define SUPPRESS_WARNINGS_END                 \
        _Pragma("clang diagnostic pop")

#elif defined(__GNUC__)
    #define SUPPRESS_WARNINGS_BEGIN               \
        _Pragma("GCC diagnostic push")            \
        _Pragma("GCC diagnostic ignored \"-Wall\"") \
        _Pragma("GCC diagnostic ignored \"-Wextra\"") \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"")

    #define SUPPRESS_WARNINGS_END                 \
        _Pragma("GCC diagnostic pop")

#else
    #define SUPPRESS_WARNINGS_BEGIN
    #define SUPPRESS_WARNINGS_END
#endif
