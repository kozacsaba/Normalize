#include <exception>
#include <map>

#define NORM_DEF_EXC                                                            \
class exception : public std::exception                                         \
{                                                                               \
    public:                                                                     \
        exception(ExcType type) : mType(type) {};                               \
        exception(exception&&) = default;                                       \
        exception(const exception&) = default;                                  \
        ~exception() override = default;                                        \
                                                                                \
        const char* what() const override { return TypeMap[mType]; }            \
                                                                                \
    private:                                                                    \
        const ExcType mType;                                                    \
}                                                                               \

namespace norm::exc
{

namespace MainProc
{
    enum class ExcType : int
    {
        abort = 0,
        root_dir_unset,
        root_dir_missing,
        
        numberOfTypes
    };

    std::map<ExcType, const char*> TypeMap =
    {
        {ExcType::abort, "Abort"},
        {ExcType::root_dir_unset, "Root Directory is not set"},
        {ExcType::root_dir_missing, "Root directory does not exist"}
    };

    NORM_DEF_EXC;

    namespace get
    {
        void abort() { throw exception(MainProc::ExcType::abort); }
        void root_dir_unset() { throw exception(MainProc::ExcType::root_dir_unset); }
        void root_dir_missing() { throw exception(MainProc::ExcType::root_dir_missing); }
    };

} // namespace MainProc

} // namespace norm::exc

#undef NORM_DEF_EXC