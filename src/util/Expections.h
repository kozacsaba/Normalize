#pragma once

#include <exception>
#include <map>

#define NORM_DEF_EXC                                                            \
class exception : public std::exception                                         \
{                                                                               \
    public:                                                                     \
        exception(ExcType type) : mType(type) {}                                \
        exception(exception&&) = default;                                       \
        exception(const exception&) = default;                                  \
        ~exception() override = default;                                        \
                                                                                \
        const char* what() const override { return TypeMap[mType]; }            \
        const ExcType& getType() const { return mType; }                        \
                                                                                \
    private:                                                                    \
        const ExcType mType;                                                    \
}                                                                               \

// i was not prepared...

/*
#define NORM_STOP(...)
#define NORM_EXPAND(...) __VA_ARGS__ NORM_STOP()

#define NORM_FOR_EACH_PAIR(macro, arg1, arg2, ...)                              \
    NORM_EXPAND(macro)(arg1, arg2)                                              \
    __VA_OPT__(NORM_EXPAND(NORM_FOR_EACH_PAIR(macro, __VA_ARGS__)))             \

#define NORM_KEY_VALUE(key, value) {ExcType::##key, ##value},
#define NORM_FIRST_OF_PAIR(first, second) first,

#define NORM_EXC_ENUM(...)                                                      \
enum class ExcType : int                                                        \
{                                                                               \
    NORM_FOR_EACH_PAIR(NORM_FIRST_OF_PAIR, __VA_ARGS__)                         \
};                                                                              \
*/

namespace norm::exc
{

namespace MainProc
{

    enum class ExcType : int
    {
        abort = 0,
        root_dir_unset,
        root_dir_missing,
        
        numberOfTypes,
    };

    inline std::map<ExcType, const char*> TypeMap =
    {
        {ExcType::abort, "Abort"},
        {ExcType::root_dir_unset, "Root Directory is not set"},
        {ExcType::root_dir_missing, "Root directory does not exist"},
    };

    NORM_DEF_EXC;

    namespace get
    {
        inline void abort() { throw exception(ExcType::abort); }
        inline void root_dir_unset() { throw exception(ExcType::root_dir_unset); }
        inline void root_dir_missing() { throw exception(ExcType::root_dir_missing); }
    }

} // namespace MainProc

namespace Gui
{
    enum class ExcType : int
    {
        settings_node_corrupted,
        settings_node_missing,

        numberOfTypes
    };

    inline std::map<ExcType, const char*> TypeMap =
    {
        {ExcType::settings_node_corrupted, "Settings node is corrupted"},
        {ExcType::settings_node_missing, "Settings node is missing"}
    };

    NORM_DEF_EXC;

    namespace get
    {
        inline void settings_node_corrupted() { throw exception(ExcType::settings_node_corrupted); }
        inline void settings_node_missing() { throw exception(ExcType::settings_node_missing); }
    }

} // namespace Gui

} // namespace norm::exc

#undef NORM_DEF_EXC