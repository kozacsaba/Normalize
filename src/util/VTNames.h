#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace norm::vt
{
    using id = const juce::Identifier;

    namespace Tree
    {
        id root = "root";
        id settings = "settings";
        id interface = "interface";
        id directory = "directory";
    }

    namespace Settings
    {
        id target_lkfs = "target_lkfs";
        id recursive_search = "recursive_search";
        id ignore_loudness_tag = "ignore_loudness_tag"; 
        id ignore_sample_peak = "ignore_loudness_tag";
        id follow_symlinks = "follow_symlinks";
    }

    namespace Interface
    {
        id progress_bar = "progress_bar";
        id current_file = "current_file";
        id show_log = "show_log";
        id is_processing = "is_processing";
        id number_of_files = "number_of_files";
    }

    namespace Directory
    {
        id folder = "folder";
        id file = "file";

        namespace Folder
        {
            id path = "path";
            id name = "name";
        }

        namespace File
        {
            id path = "path";
            id name = "name";
            id valid = "valid";
            id has_warning = "has_warning";
            id selected = "selected";
            id processed = "processed";
            id loudness = "loudness";
            id peak = "peak";
            id err_msg = "err_msg";
        }
    }
}

/*  
    root
    L.. settings
    |   L.. target_lkfs             [float]
    |   L.. recursive_search        [bool]
    |   L.. ignore_loudness_tag     [bool]
    |   L.. ignore_sample_peak      [bool]
    |   L.. follow_symlinks         [bool]
    L.. interface
    |   L.. progress_bar %          [float]
    |   L.. current_file            [string]
    |   L.. is_processing           [bool]
    |   L.. show_log                [bool]
    |   L.. number_of_files         [int]
    L.. directory
        L.. <folder>
        |   L.. <...>
        L.. <file>
*/

// note:
// the target directory is stored in the value tree because after a parse, info
// should be available on the gui, relating to files that are already normalized
// processing should be executed based on the value tree structure, not the
// file structure, besause that is more in synch with what the user probably
// intended to do.