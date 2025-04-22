// I don't think this is macos compatible, but I don't have an apple device I
// could fix this on right now. Maybe in the future...

#include "suppress_warnings.h"

SUPPRESS_WARNINGS_BEGIN
#define asWIN32DLL
#include "mp3gain/mp3gain.h"
#undef asWIN32DLL
SUPPRESS_WARNINGS_END

namespace norm
{
    inline void wrap_changeGain(char *filename, int leftgainchange, int rightgainchange)
    {
        // win32
        changeGain(filename, leftgainchange, rightgainchange);
    }
}