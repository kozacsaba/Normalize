/*
    This holds the action bar / loading bar panel, the config/log switch and
    the config/log panel. Is a listener for app state, so that it can show /
    hide the correct panels.
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "util/VTNames.h"
#include "util/Logger.h"
#include "processor/MainProcessor.h"

namespace norm
{

class SettingsPanel : public juce::Viewport
{
private:

    const int recommendedRowHeight = 30;

    class ViewedComponent : public juce::Component
    {
    private:

        class SettingButton : public juce::ToggleButton
        {
        public:
            SettingButton(vt::id setting_id, juce::String displayName);

            static void setSettingsNode(juce::ValueTree node);

        private:
            // Could be raplaced by root node to make sure the same object is
            // used everywhere, but I don't think it is neccessary, as there are
            // no listeners to the Settings Tree.
            static juce::ValueTree settingsNode;
            const vt::id settingId;
        };

    public:
        ViewedComponent(juce::ValueTree& root);

        void resized() override;

        int getNumberOfSettings() const
        {
            return btnSettings.size();
        }

    private:
        juce::ValueTree& mRoot;
        juce::Array<std::shared_ptr<juce::ToggleButton>> btnSettings;
    };

public:
    SettingsPanel(juce::ValueTree& root);

    void resized() override;

private:
    juce::ValueTree& mRoot;
    ViewedComponent mViewedComponent;
};

class LogPanel 
    : public juce::Component
    , public norm::Logger::LogDestination
{
public:
    LogPanel();
    ~LogPanel() override;

    void resized() override;

    void log(juce::String msg) override;

private:
    juce::TextEditor mLogDisplay;
};

class ActionPanel : public juce::Component
{
public:
    ActionPanel(juce::ValueTree& root, 
                MainProcessor* processor);

    void resized() override;

    void targetEntered();
    void startClicked();

private:
    static bool isValidNumber(juce::String str);

    juce::ValueTree& mRoot;
    MainProcessor* mProcessor;

    juce::Label lblTarget;
    juce::Label lblTargetInput;
    juce::TextButton btnStart;
};

class LoadingPanel 
    : public juce::Component
    , public juce::ValueTree::Listener
{
    const int progress_percent_width = 30;
    const int abort_button_width = 80;
    const int loading_bar_margin = 2;

public:
    LoadingPanel(juce::ValueTree& root, 
                 MainProcessor* processor);
    ~LoadingPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void valueTreePropertyChanged (juce::ValueTree &treeWhosePropertyHasChanged,
                                   const juce::Identifier &property) override;

    void abortClicked();

private:
    juce::ValueTree& mRoot;
    MainProcessor* mProcessor;

    juce::Label lblProgressPercent;
    juce::Rectangle<int> loadingBar;
    juce::Label lblStatus;
    juce::TextButton btnAbort;

    int numberOfAllFiles;
    int numberOfProcessedFiles;
};

class MainPanel 
    : public juce::Component
    , public juce::ValueTree::Listener
{
    const int toggle_button_height = 30;
    const int toggle_button_width = 70;

public:
    MainPanel(juce::ValueTree& root,
              MainProcessor* processor);
    ~MainPanel() override;

    void resized() override;

    void valueTreePropertyChanged (juce::ValueTree &treeWhosePropertyHasChanged, 
                                   const juce::Identifier &property) override;

private:
    void toggleViewClicked();

    juce::ValueTree& mRoot;
    MainProcessor* mProcessor;

    SettingsPanel mSettingsPanel;
    LogPanel mLogPanel;
    ActionPanel mActionPanel;
    LoadingPanel mLoadingPanel;

    juce::TextButton btnToggleView;
};

}