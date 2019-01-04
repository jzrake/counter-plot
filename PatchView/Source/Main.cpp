#include "Main.hpp"
#include "MainComponent.hpp"




//==============================================================================
START_JUCE_APPLICATION (PatchViewApplication)




//==============================================================================
PatchViewApplication::MainWindow::MainWindow (String name) : DocumentWindow (name, Colours::black, DocumentWindow::allButtons)
{
    content = std::make_unique<MainComponent>();

    addKeyListener (getApp().commandManager.getKeyMappings());
    setUsingNativeTitleBar (true);
    setContentNonOwned (content.get(), true);
    setResizable (true, true);
    centreWithSize (getWidth(), getHeight());
    setVisible (true);
}

PatchViewApplication::MainWindow::~MainWindow()
{
}

void PatchViewApplication::MainWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}




//==============================================================================
PatchViewApplication::MainMenu::MainMenu()
{
    setApplicationCommandManagerToWatch (&getApp().commandManager);
}

StringArray PatchViewApplication::MainMenu::getMenuBarNames()
{
    return {"File"};
}

PopupMenu PatchViewApplication::MainMenu::getMenuForIndex (int /*topLevelMenuIndex*/, const String& menuName)
{
    PopupMenu menu;
    menu.addCommandItem (&getApp().getCommandManager(), Commands::openDirectory);
    return menu;
}

void PatchViewApplication::MainMenu::menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/)
{
    // Not sure what this method is for...
}




//==============================================================================
PatchViewApplication& PatchViewApplication::getApp()
{
    return *dynamic_cast<PatchViewApplication*> (JUCEApplication::getInstance());
}

ApplicationCommandManager& PatchViewApplication::getCommandManager()
{
    return commandManager;
}

PatchViewApplication::PatchViewApplication()
{
}

const String PatchViewApplication::getApplicationName()
{
    return ProjectInfo::projectName;
}

const String PatchViewApplication::getApplicationVersion()
{
    return ProjectInfo::versionString;
}

bool PatchViewApplication::moreThanOneInstanceAllowed()
{
    return true;
}

void PatchViewApplication::initialise (const String& commandLine)
{
    configureLookAndFeel();
    commandManager.setFirstCommandTarget (this); // Need this?
    commandManager.registerAllCommandsForTarget (this);
    MenuBarModel::setMacMainMenu (&menu, nullptr);
    mainWindow = std::make_unique<MainWindow> (getApplicationName());
}

void PatchViewApplication::shutdown()
{
    MenuBarModel::setMacMainMenu (nullptr, nullptr);
    mainWindow = nullptr;
}

void PatchViewApplication::systemRequestedQuit()
{
    quit();
}

void PatchViewApplication::anotherInstanceStarted (const String& commandLine)
{
}




//==========================================================================
void PatchViewApplication::getAllCommands (Array<CommandID>& commands)
{
    JUCEApplication::getAllCommands (commands);

    const CommandID ids[] = {
        Commands::openDirectory,
    };
    commands.addArray (ids, numElementsInArray (ids));
}

void PatchViewApplication::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case Commands::openDirectory:
            result.setInfo ("Open...", "", "File", 0);
            result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 0));
            break;
        default:
            JUCEApplication::getCommandInfo (commandID, result);
            break;
    }
}

bool PatchViewApplication::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case Commands::openDirectory:             return presentOpenDirectoryDialog();
        default:                                  return JUCEApplication::perform (info);
    }
}




//==========================================================================
void PatchViewApplication::configureLookAndFeel()
{
    auto& laf = Desktop::getInstance().getDefaultLookAndFeel();
    laf.setColour (TextEditor::backgroundColourId, Colours::white);
    laf.setColour (TextEditor::textColourId, Colours::black);
    laf.setColour (TextEditor::highlightColourId, Colours::lightblue);
    laf.setColour (TextEditor::highlightedTextColourId, Colours::black);
    laf.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    laf.setColour (TextEditor::focusedOutlineColourId, Colours::lightblue);
    laf.setColour (Label::ColourIds::textColourId, Colours::lightgrey);
    laf.setColour (Label::ColourIds::textWhenEditingColourId, Colours::lightgrey);
    laf.setColour (Label::ColourIds::backgroundWhenEditingColourId, Colours::white);
    laf.setColour (ListBox::backgroundColourId, Colours::white);
    laf.setColour (TreeView::backgroundColourId, Colours::darkgrey.darker (0.1f));
    laf.setColour (TreeView::selectedItemBackgroundColourId, Colours::darkgrey.darker (0.2f));
    laf.setColour (TreeView::dragAndDropIndicatorColourId, Colours::green);
    laf.setColour (TreeView::evenItemsColourId, Colours::transparentBlack);
    laf.setColour (TreeView::oddItemsColourId, Colours::transparentBlack);
    laf.setColour (TreeView::linesColourId, Colours::red);
}

bool PatchViewApplication::presentOpenDirectoryDialog()
{
    FileChooser chooser ("Open directory...",
                         currentDirectory,
                         "",
                         true,
                         false,
                         nullptr);

    if (chooser.browseForDirectory())
    {
        mainWindow->content->setCurrentDirectory (chooser.getResult());
    }
    
    return true;
}
