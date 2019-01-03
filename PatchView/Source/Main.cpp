#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"




//==============================================================================
class PatchViewApplication  : public JUCEApplication
{
public:
    //==============================================================================
    PatchViewApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        configureLookAndFeel();
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    void configureLookAndFeel()
    {
        auto& laf = Desktop::getInstance().getDefaultLookAndFeel();
        laf.setColour (TextEditor::backgroundColourId, Colours::white);
        laf.setColour (TextEditor::textColourId, Colours::black);
        laf.setColour (TextEditor::highlightColourId, Colours::lightblue);
        laf.setColour (TextEditor::highlightedTextColourId, Colours::black);
        laf.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        laf.setColour (TextEditor::focusedOutlineColourId, Colours::lightblue);
        laf.setColour (Label::ColourIds::textColourId, Colours::black);
        laf.setColour (Label::ColourIds::textWhenEditingColourId, Colours::black);
        laf.setColour (Label::ColourIds::backgroundWhenEditingColourId, Colours::white);
        laf.setColour (ListBox::backgroundColourId, Colours::white);

        laf.setColour (TreeView::dragAndDropIndicatorColourId, Colours::green);
        laf.setColour (TreeView::selectedItemBackgroundColourId, Colours::transparentBlack);
        laf.setColour (TreeView::ColourIds::linesColourId, Colours::red);
        laf.setColour (TreeView::oddItemsColourId, Colours::whitesmoke);
        laf.setColour (TreeView::evenItemsColourId, Colours::white);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
    }

    //==============================================================================
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (String name) : DocumentWindow (name, Colours::white, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

//            context.attachTo (*this);
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
//        OpenGLContext context;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};




//==============================================================================
START_JUCE_APPLICATION (PatchViewApplication)
