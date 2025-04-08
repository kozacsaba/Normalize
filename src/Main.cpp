#include "processor/MainProcessor.h"
#include "gui/MainComponent.h"
#include "util/Logger.h"
#include "util/VTNames.h"

class NormalizeApplication final : public juce::JUCEApplication
{
public:
    NormalizeApplication() {}

    const juce::String getApplicationName() override       { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion() override    { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused (commandLine);
        norm::Logger::getInstance()->addListener(norm::StdLogger::getInstance());

        mRoot = norm::vt::buildValueTree();
        mProcessor = std::make_unique<norm::MainProcessor>(mRoot);

        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;

        norm::Logger::getInstance()->removaAllListeners();
        mProcessor = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);
    }

    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (backgroundColourId),
                              allButtons)
        {
            setUsingNativeTitleBar (true);

            auto* app = (NormalizeApplication*)NormalizeApplication::getInstance();

            setContentOwned(
                new MainComponent(
                    app->getValueTreeRoot(), 
                    app->getProcessor()),
                true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    juce::ValueTree getValueTreeRoot() const { return mRoot; }

    norm::MainProcessor* getProcessor() const { return mProcessor.get(); }

private:
    juce::ValueTree mRoot;

    std::unique_ptr<norm::MainProcessor> mProcessor;

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (NormalizeApplication)
