#include "MainComponent.h"
#include "util/VTNames.h"

MainComponent::MainComponent (juce::ValueTree root,
                              norm::MainProcessor* proc)
    : mRoot(root)
    , mProcessor(proc)
    , mMainPanel(mRoot, mProcessor)
{
    addAndMakeVisible(mMainPanel);
    addAndMakeVisible(mFileList);

    setSize (800, 600);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    mFileList.setBounds(area.removeFromRight(area.proportionOfWidth(1.f/3.f)));
    mMainPanel.setBounds(area);
}
