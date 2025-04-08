#include "MainPanel.h"
#include "util/Logger.h"
#include "util/Expections.h"

using namespace norm;

//==============================================================================

juce::ValueTree SettingsPanel::ViewedComponent::SettingButton::
settingsNode = juce::ValueTree();

SettingsPanel::ViewedComponent::SettingButton::
SettingButton (vt::id setting_id, juce::String displayName)
    : ToggleButton(displayName)
    , settingId(setting_id)
{
    onClick = [this] () {
        settingsNode.setProperty(
            settingId,
            getToggleState(),
            nullptr
        );
    };
}

void SettingsPanel::ViewedComponent::SettingButton::
setSettingsNode (juce::ValueTree node)
{
    if (node == juce::ValueTree()) 
        exc::Gui::get::settings_node_missing();
    if (node.getType() != vt::Tree::settings) 
        exc::Gui::get::settings_node_corrupted();

    settingsNode = node;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SettingsPanel::ViewedComponent::ViewedComponent(juce::ValueTree& root)
    : mRoot(root)
    , btnSettings {
        new SettingButton (vt::Settings::recursive_search,
                           "Search directory recursively"),
        new SettingButton (vt::Settings::ignore_loudness_tag,
                           "Ignore previous measurements"),
        new SettingButton (vt::Settings::follow_symlinks,
                           "Follow symlinks when looking for files")}
{
    for (auto& btn : btnSettings)
    {
        addAndMakeVisible(btn.get());
    }

    SettingButton::setSettingsNode(mRoot.getChildWithName(vt::Tree::settings));
}

void SettingsPanel::ViewedComponent::resized() 
{
    auto area = getLocalBounds();
    const int elementHeight = area.getHeight() / getNumberOfSettings();
    for(auto& setting : btnSettings)
    {
        setting->setBounds(area.removeFromTop(elementHeight));
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SettingsPanel::SettingsPanel(juce::ValueTree& root)
    : mRoot(root)
    , mViewedComponent(mRoot)
{
    setViewedComponent(&mViewedComponent, false);
}

void SettingsPanel::resized()
{
    auto viewedArea = getViewArea();
    viewedArea.setHeight (recommendedRowHeight * 
                          mViewedComponent.getNumberOfSettings()
    );
}

//==============================================================================

LogPanel::LogPanel()
{
    Logger::getInstance()->addListener(this);

    addAndMakeVisible(mLogDisplay);
    mLogDisplay.setMultiLine(true, true);
    mLogDisplay.setReadOnly(true);
}

LogPanel::~LogPanel()
{
    Logger::getInstance()->removeListener(this);
}

void LogPanel::resized()
{
    mLogDisplay.setBounds(getLocalBounds());
}

void LogPanel::log(juce::String msg)
{
    mLogDisplay.insertTextAtCaret(msg);
}

//==============================================================================

ActionPanel::ActionPanel (juce::ValueTree& root, 
                          MainProcessor* processor)
    : mRoot(root)
    , mProcessor(processor)
    , lblTarget("lblTarget", "Target LKFS: ")
    , lblTargetInput("lblInput", "Enter Target")
    , btnStart("btnStart")
{
    addAndMakeVisible (lblTarget);
    lblTarget.setEditable (false);

    addAndMakeVisible (lblTargetInput);
    lblTargetInput.setEditable (true, true, true);
    lblTargetInput.onTextChange = [this] { targetEntered(); };

    addAndMakeVisible (btnStart);
    btnStart.setButtonText("START");
    btnStart.onClick = [this] { startClicked(); };
}

void ActionPanel::resized()
{
    auto area = getLocalBounds();

    lblTarget.setBounds(area.removeFromLeft(area.proportionOfWidth(1.f/3.f)));
    lblTargetInput.setBounds(area.removeFromLeft(area.proportionOfWidth(0.5f)));
    btnStart.setBounds(area);
}

bool ActionPanel::isValidNumber(juce::String str)
{
    if(!str.containsOnly("+-.,0123456789"))
    {
        const char* errmsg = "Target loudness \"{}\" declined, because it contains unrecodnised characters. Try a number.";

        MY_LOG_INFO(errmsg, str);
        return false;   
    }

    if(str.containsAnyOf("+-"))
    {
        if(str.containsChar('+') && str.containsChar('-'))
        {
            const char* errmsg = "Target loudness \"{}\" declined, because it is not a single number.";
            MY_LOG_INFO(errmsg, str);
            return false;
        }

        juce::juce_wchar token = str.containsChar('+') ? '+' : '-';

        if(str[0] != token)
        {
            const char* errmsg = "Target loudness \"{}\" declined, because the {} sign is not at the beginning.";
            MY_LOG_INFO(errmsg, str, juce::String(token));
            return false;
        }

        if(str.substring(1).containsChar(token))
        {
            const char* errmsg = "Target loudness \"{}\" declined, because it contains multiple {} signs.";
            MY_LOG_INFO(errmsg, str, token);
            return false;
        }
    }

    if(str.containsAnyOf(".,"))
    {
        if(str.containsChar('.') && str.containsChar(','))
        {
            const char* errmsg = "Target loudness \"{}\" declined, because it contains both . and ,\nYou can use either as the decimal marker, but not both.";
            MY_LOG_INFO(errmsg, str);
            return false;
        }

        juce::juce_wchar token = str.containsChar('.') ? '.' : ',';

        juce::String decimal = str.fromFirstOccurrenceOf (juce::String(token), 
                                                          false,
                                                          true);

        if(decimal.containsChar(token))
        {
            const char* errmsg = "Target loudness \"{}\" declined, because it has multiple occurances of {}";
            MY_LOG_INFO(errmsg, str, juce::String(token));
            return false;
        }
    }

    return true;
}

void ActionPanel::targetEntered()
{
    juce::String entry = lblTargetInput.getText();
    if (!isValidNumber(entry))
    {
        lblTarget.setText("invalid", juce::dontSendNotification);
        return;
    }

    const bool processRunning = 
        mRoot.getChildWithName(vt::Tree::interface)
        [vt::Interface::is_processing];

    if(processRunning)
    {
        const char* errmsg = "Cannot set target value while normalisation or parsing is running";
        MY_LOG_INFO(errmsg);
        lblTargetInput.setText("busy", juce::dontSendNotification);
        return;
    }

    const float target = entry.getFloatValue();
    mRoot.getChildWithName(vt::Tree::settings).setProperty(
        vt::Settings::target_lkfs,
        target,
        nullptr
    );
}

void ActionPanel::startClicked()
{
    const bool processRunning = 
    mRoot.getChildWithName(vt::Tree::interface)
    [vt::Interface::is_processing];

    if(processRunning)
    {
        const char* errmsg = "Cannot launch normalisation while another normalisation or a parsing process is running. Try again later, or kill current process";
        MY_LOG_INFO(errmsg);
        lblTargetInput.setText("busy", juce::dontSendNotification);
        return;
    }

    mProcessor->beginProcessing();
}

//==============================================================================

LoadingPanel::LoadingPanel(juce::ValueTree& root, 
                           MainProcessor* processor)
    : mRoot(root)
    , mProcessor(processor)
    , btnAbort("abort")
    , numberOfAllFiles(0)
    , numberOfProcessedFiles(0)
{
    mRoot.getChildWithName(vt::Tree::interface).addListener(this);

    addAndMakeVisible(lblProgressPercent);
    lblProgressPercent.setEditable(false);
    lblProgressPercent.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(lblStatus);
    lblStatus.setEditable(false);
    lblStatus.setJustificationType(juce::Justification::centredLeft);

    addAndMakeVisible(btnAbort);
    btnAbort.setButtonText("Abort");
    btnAbort.onClick = [this] { abortClicked(); };
}

LoadingPanel::~LoadingPanel()
{
    mRoot.getChildWithName(vt::Tree::interface).removeListener(this);
}

void LoadingPanel::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::darkgreen);
    g.fillRect(loadingBar);

    const float progress = 
        (float)numberOfProcessedFiles / 
        (float)numberOfAllFiles;

    auto filledpart = loadingBar;
    g.setColour(juce::Colours::green);
    g.fillRect(filledpart.removeFromLeft(
        filledpart.proportionOfWidth(progress)));
}

void LoadingPanel::resized()
{
    auto area = getLocalBounds();

    btnAbort.setBounds(area.removeFromRight(abort_button_width));
    lblStatus.setBounds(area.removeFromBottom(area.proportionOfHeight(0.5f)));
    lblProgressPercent.setBounds(area.removeFromLeft(progress_percent_width));
    loadingBar = area.reduced(loading_bar_margin);
}

void LoadingPanel::valueTreePropertyChanged (
    juce::ValueTree &treeWhosePropertyHasChanged, 
    const juce::Identifier &property)
{
    if(treeWhosePropertyHasChanged.getType() != vt::Tree::interface) return;

    if(property == vt::Interface::current_file)
    {
        lblStatus.setText(
            "Processing: " +
            treeWhosePropertyHasChanged[property].toString(),
            juce::dontSendNotification
        );
    }
    else if(property == vt::Interface::progress_bar)
    {
        numberOfProcessedFiles = treeWhosePropertyHasChanged[property];
        repaint();
    }
    else if(property == vt::Interface::number_of_files)
    {
        numberOfAllFiles = treeWhosePropertyHasChanged[property];
    }
}

void LoadingPanel::abortClicked()
{
    mProcessor->abort();
}

//==============================================================================

MainPanel::MainPanel (juce::ValueTree& root,
                      MainProcessor* processor)
    : mRoot(root)
    , mProcessor(processor)
    , mSettingsPanel(mRoot)
    , mLogPanel()
    , mActionPanel(mRoot, mProcessor)
    , mLoadingPanel(mRoot, mProcessor)
    , btnToggleView("toggle_view")
{
    addAndMakeVisible(mSettingsPanel);
    addChildComponent(mLogPanel);
    addAndMakeVisible(mActionPanel);
    addChildComponent(mLoadingPanel);

    addAndMakeVisible(btnToggleView);
    btnToggleView.setButtonText("Toggle View");
    btnToggleView.onClick = [this] { toggleViewClicked(); };

    mRoot.getChildWithName(vt::Tree::interface).addListener(this);
}

MainPanel::~MainPanel()
{
    mRoot.getChildWithName(vt::Tree::interface).removeListener(this);
}

void MainPanel::resized()
{
    auto area = getLocalBounds();

    auto topPanel = area.removeFromTop(toggle_button_height);
    btnToggleView.setBounds(topPanel.removeFromRight(toggle_button_width));
    mActionPanel.setBounds(topPanel);
    mLoadingPanel.setBounds(topPanel);

    mSettingsPanel.setBounds(area);
    mLogPanel.setBounds(area);
}

void MainPanel::valueTreePropertyChanged (
    juce::ValueTree &treeWhosePropertyHasChanged, 
    const juce::Identifier &property)
{
    MY_LOG_INFO(
        "Detected change of property {} in tree {}.\n",
        property.toString(),
        treeWhosePropertyHasChanged.getType().toString()
    );

    if(treeWhosePropertyHasChanged.getType() != vt::Tree::interface) return;

    if(property == vt::Interface::show_log)
    {
        const bool showLog = treeWhosePropertyHasChanged[property];
        mLogPanel.setVisible(showLog);
        mSettingsPanel.setVisible(!showLog);
    }
    else if(property == vt::Interface::is_processing)
    {
        const bool showLoading = treeWhosePropertyHasChanged[property];
        mLoadingPanel.setVisible(showLoading);
        mActionPanel.setVisible(!showLoading);
    }
}

void MainPanel::toggleViewClicked()
{
    const bool show_log = 
        mRoot.getChildWithName(vt::Tree::interface)[vt::Interface::show_log];

    mRoot.getChildWithName(vt::Tree::interface).setProperty(
        vt::Interface::show_log,
        !show_log,
        nullptr
    );
}
