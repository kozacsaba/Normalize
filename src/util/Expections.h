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
        node_not_file,
        traverse_callback_error,
        normalise_before_measurement,
        could_not_load_audio,
        would_peak,
        
        numberOfTypes,
    };

    inline std::map<ExcType, const char*> TypeMap =
    {
        {ExcType::abort, "Abort."},
        {ExcType::root_dir_unset, "Root Directory is not set."},
        {ExcType::root_dir_missing, "Root directory does not exist."},
        {ExcType::node_not_file, "This node does not represent a file or folder."},
        {ExcType::traverse_callback_error, "Callback error while traversing file structure."},
        {ExcType::normalise_before_measurement, "You cannot normalise before measuring loudness."},
        {ExcType::could_not_load_audio, "Could not load audio from file."},
        {ExcType::would_peak, "Audio would peak if normalised with current value."},
    };

    NORM_DEF_EXC;

    namespace get
    {
        inline void abort() { throw exception(ExcType::abort); }
        inline void root_dir_unset() { throw exception(ExcType::root_dir_unset); }
        inline void root_dir_missing() { throw exception(ExcType::root_dir_missing); }
        inline void node_not_file() { throw exception(ExcType::node_not_file); }
        inline void traverse_callback_error() { throw exception(ExcType::traverse_callback_error); }
        inline void normalise_before_measurement() { throw exception(ExcType::normalise_before_measurement); }
        inline void could_not_load_audio() { throw exception(ExcType::could_not_load_audio); }
        inline void would_peak() { throw exception(ExcType::would_peak); }
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

namespace FileHandler
{
    enum class ExcType : int
    {
        no_reader_for_file,
        no_writer_for_file,
        insufficient_buffer,
        no_audio_loaded,

        numberOfTypes
    };

    inline std::map<ExcType, const char*> TypeMap =
    {
        {ExcType::no_reader_for_file, "Could not create reader for file."},
        {ExcType::no_writer_for_file, "Could not create writer for file."},
        {ExcType::insufficient_buffer, "Buffer is insufficient for one audio block."},
        {ExcType::no_audio_loaded, "Audio is not loaded into the FileHandler from the File."},
    };

    NORM_DEF_EXC;

    namespace get
    {
        inline void no_reader_for_file() { throw exception(ExcType::no_reader_for_file); }
        inline void no_writer_for_File() { throw exception(ExcType::no_writer_for_file); }
        inline void insufficient_buffer() { throw exception(ExcType::insufficient_buffer); }
        inline void no_audio_loaded() { throw exception(ExcType::no_audio_loaded); }
    }

} // namespace FileHandler

} // namespace norm::exc

#undef NORM_DEF_EXC