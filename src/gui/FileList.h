/*  
    After selecting a starting folder, this should be a list of all files
    that are to be normalized. The reason it gets its own class is that each
    list element should have an option to exclude it (and possibly other
    options), which is done simplest by deriving a class from juce::TreeView and
    juce::TreeVirewItem.
*/

#pragma once
#include "juce_gui_basics/juce_gui_basics.h"

namespace norm
{

class FileList : public juce::Component
{
public:

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::gold);
        g.setColour(juce::Colours::black);
        g.setFont(12.0f);
        g.drawText("PLACEHOLDER", getLocalBounds(), juce::Justification::centred, true);

        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 2);
    }

private:
    juce::Rectangle<int> area;
};

}