/*  After selecting a starting folder, this should be a list of all files
    that are to be normalized. The reason it gets its own class is that each
    list element should have an option to exclude it (and possibly other
    options), which is done simplest by deriving a class from juce::ListBox and
    juce::ListBoxModel.
*/

#include "FileList.h"