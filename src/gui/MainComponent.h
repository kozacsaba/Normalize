#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "util/Expections.h"
#include "gui/MainPanel.h"
#include "gui/FileList.h"

class MainComponent final : public juce::Component
{
public:
    MainComponent (juce::ValueTree root,
                   norm::MainProcessor* proc);

    void resized() override;

private:
    juce::ValueTree mRoot;
    norm::MainProcessor* mProcessor;

    norm::MainPanel mMainPanel;
    norm::FileList mFileList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
