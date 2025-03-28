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
        id ignore_tags = "ignore_tags";
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
            id measured = "measured";
            id loudness = "loudness";
            id peak = "peak";
            id err_msg = "err_msg";
        }
    }

    inline juce::ValueTree buildValueTree()
    {
        juce::ValueTree root(norm::vt::Tree::root);

        //--------------------------------------------------------------------------

        juce::ValueTree Interface(norm::vt::Tree::interface);
        Interface.setProperty (norm::vt::Interface::current_file,
                               "...",
                               nullptr);
        Interface.setProperty (norm::vt::Interface::is_processing,
                               false,
                               nullptr);
        Interface.setProperty (norm::vt::Interface::number_of_files,
                               0,
                               nullptr);
        Interface.setProperty (norm::vt::Interface::progress_bar,
                               0,
                               nullptr);
        Interface.setProperty (norm::vt::Interface::show_log,
                               false,
                               nullptr);
        root.addChild(Interface, 0, nullptr);

        //--------------------------------------------------------------------------

        juce::ValueTree Settings(norm::vt::Tree::settings);
        Settings.setProperty (norm::vt::Settings::follow_symlinks,
                              false,
                              nullptr);
        Settings.setProperty (norm::vt::Settings::ignore_tags,
                              false,
                              nullptr);
        Settings.setProperty (norm::vt::Settings::recursive_search,
                              true,
                              nullptr);
        Settings.setProperty (norm::vt::Settings::target_lkfs,
                              -18.f,
                              nullptr);
        root.addChild(Settings, 0, nullptr);

        //--------------------------------------------------------------------------

        juce::ValueTree Directory(norm::vt::Tree::directory);
        juce::ValueTree rootDir(norm::vt::Directory::folder);
        Directory.addChild(rootDir, 0, nullptr);
        root.addChild(Directory, 0, nullptr);

        //--------------------------------------------------------------------------

        return root;
    }

}

/*  
    root
    L.. settings
    |   L.. target_lkfs             [float]
    |   L.. recursive_search        [bool]
    |   L.. ignore_tags             [bool]
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
// file structure, because that is more in sync with what the user probably
// intended to do.