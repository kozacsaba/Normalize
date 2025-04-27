// I don't think this is macos compatible, but I don't have an apple device I
// could fix this on right now. Maybe in the future...

#include "mp3gain.h"

namespace norm
{

    inline int wrap_changeGain(char *filename, int leftgainchange, int rightgainchange)
    {
        // win32
        return changeGain(filename, leftgainchange, rightgainchange);

        // need to add macos call in wrapper
    }
}