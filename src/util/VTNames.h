#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace norm::vt
{
    namespace tree
    {
        const juce::Identifier Settings = "settings";
        const juce::Identifier Interface = "interface";
    }

    namespace settings
    {
        const juce::Identifier target_lkfs = "target_lkfs";
        const juce::Identifier recursive_search = "recursive_search";
        const juce::Identifier ignore_loudness_tag = "ignore_loudness_tag"; 
        const juce::Identifier ignore_sample_peak = "ignore_loudness_tag";
        const juce::Identifier follow_symlinks = "follow_symlinks";
    }

    namespace interface
    {
        const juce::Identifier progress_bar = "progress_bar";
        const juce::Identifier current_file = "current_file";
    }
}

/*  
    root
    L.. settings
    |   L.. target lkfs             [float]
    |   L.. recursive search        [bool]
    |   L.. ignore loudness tag     [bool]
    |   L.. ignore sample peak      [bool]
    |   L.. follow symlinks         [bool]
    L.. interface
        L.. progress bar %          [float]
        L.. current file            [string]
*/