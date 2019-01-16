#include "Main.hpp"
#include "LookAndFeel.hpp"
#include "DataHelpers.hpp"
#include "../Components/MainComponent.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Viewers/Viewer.hpp"




//=============================================================================
START_JUCE_APPLICATION (PatchViewApplication)

static herr_t h5_error_handler(hid_t estack, void*)
{
    // H5Eprint(estack, stdout);
    return 0;
}




//=============================================================================
PatchViewApplication::MainWindow::MainWindow (String name) : DocumentWindow (name, Colours::black, DocumentWindow::allButtons)
{
    content = std::make_unique<MainComponent>();

    addKeyListener (getApp().commandManager->getKeyMappings());
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




//=============================================================================
PatchViewApplication::MainMenu::MainMenu()
{
    setApplicationCommandManagerToWatch (getApp().commandManager.get());
}

StringArray PatchViewApplication::MainMenu::getMenuBarNames()
{
    return {"File", "View"};
}

PopupMenu PatchViewApplication::MainMenu::getMenuForIndex (int /*topLevelMenuIndex*/, const String& menuName)
{
    auto manager = &getApp().getCommandManager();
    PopupMenu menu;

    if (menuName == "File")
    {
        menu.addCommandItem (manager, Commands::openDirectory);
        menu.addSeparator();
        menu.addCommandItem (manager, Viewer::Commands::makeSnapshotAndOpen);
        menu.addCommandItem (manager, Viewer::Commands::saveSnapshotAs);
        return menu;
    }
    if (menuName == "View")
    {
        menu.addCommandItem (manager, Commands::toggleEnvironmentView);
        menu.addCommandItem (manager, Commands::toggleDirectoryView);
        menu.addCommandItem (manager, Commands::reloadDirectoryView);
        menu.addSeparator();
        menu.addCommandItem (manager, Viewer::Commands::nextColourMap);
        menu.addCommandItem (manager, Viewer::Commands::prevColourMap);
        menu.addCommandItem (manager, Viewer::Commands::resetScalarRange);
        return menu;
    }
    jassertfalse;
    return menu;
}

void PatchViewApplication::MainMenu::menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/)
{
    // Not sure what this method is for...
}




//=============================================================================
PatchViewApplication& PatchViewApplication::getApp()
{
    return *dynamic_cast<PatchViewApplication*> (JUCEApplication::getInstance());
}

ApplicationCommandManager& PatchViewApplication::getCommandManager()
{
    return *commandManager;
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
    H5Eset_auto(H5E_DEFAULT, h5_error_handler, NULL);

    // Read app properties to restore last session
    applicationProperties.setStorageParameters (makePropertiesFileOptions());
    auto& settings = *applicationProperties.getUserSettings();
    auto cwd = settings.getValue ("LastCurrentDirectory", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());

    configureLookAndFeel();

    commandManager = std::make_unique<ApplicationCommandManager>();
    menu           = std::make_unique<MainMenu>();
    mainWindow     = std::make_unique<MainWindow> (getApplicationName());
    mainWindow->content->setCurrentDirectory (cwd);

    commandManager->registerAllCommandsForTarget (this);
    Viewer::registerCommands (*commandManager);
    MenuBarModel::setMacMainMenu (menu.get(), nullptr);

    startTimer (500);
    settingsLastPolled = Time::getCurrentTime();
}

void PatchViewApplication::shutdown()
{
    MenuBarModel::setMacMainMenu (nullptr, nullptr);
}

void PatchViewApplication::systemRequestedQuit()
{
    quit();
}

void PatchViewApplication::anotherInstanceStarted (const String& commandLine)
{
}




//=============================================================================
void PatchViewApplication::getAllCommands (Array<CommandID>& commands)
{
    JUCEApplication::getAllCommands (commands);

    const CommandID ids[] = {
        Commands::openDirectory,
        Commands::reloadCurrentFile,
        Commands::toggleDirectoryView,
        Commands::reloadDirectoryView,
        Commands::toggleEnvironmentView,
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
        case Commands::reloadCurrentFile:
            result.setInfo ("Reload Current File", "", "File", 0);
            result.defaultKeypresses.add (KeyPress ('r', ModifierKeys::commandModifier, 0));
            break;
        case Commands::toggleDirectoryView:
            result.setInfo ("Show Side Bar", "", "View",
                            mainWindow && mainWindow->content->isDirectoryTreeShowing() ? ApplicationCommandInfo::isTicked : 0);
            result.defaultKeypresses.add (KeyPress ('K', ModifierKeys::commandModifier, 0));
            break;
        case Commands::reloadDirectoryView:
            result.setInfo ("Reload Directory Tree", "", "View", 0);
            result.defaultKeypresses.add (KeyPress ('r', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
            break;
        case Commands::toggleEnvironmentView:
            result.setInfo ("Show Viewer Environment", "", "View",
                            mainWindow && mainWindow->content->isEnvironmentViewShowing() ? ApplicationCommandInfo::isTicked : 0);
            result.defaultKeypresses.add (KeyPress ('B', ModifierKeys::commandModifier, 0));
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
        case Commands::reloadCurrentFile:         mainWindow->content->reloadCurrentFile(); return true;
        case Commands::toggleDirectoryView:       mainWindow->content->toggleDirectoryTreeShown(); return true;
        case Commands::reloadDirectoryView:       mainWindow->content->reloadDirectoryTree(); return true;
        case Commands::toggleEnvironmentView:     mainWindow->content->toggleEnvironmentViewShown(); return true;
        default:                                  return JUCEApplication::perform (info);
    }
}

void PatchViewApplication::timerCallback()
{
    // This is some experimental code to support user colour schemes
    // -------------------------------------------------------------
    auto settingsFile = File::getSpecialLocation (File::userHomeDirectory).getChildFile ("patch_view_settings.json");

    if (settingsLastPolled > settingsFile.getLastModificationTime())
    {
        return;
    }

    settingsLastPolled = Time::getCurrentTime();
    var settings = JSON::parse (settingsFile);
    auto& laf = Desktop::getInstance().getDefaultLookAndFeel();

    LookAndFeelHelpers::setLookAndFeelDefaults (laf, LookAndFeelHelpers::BackgroundScheme::dark);
    LookAndFeelHelpers::setLookAndFeelDefaults (laf, LookAndFeelHelpers::TextColourScheme::pastels2);
    FigureView        ::setLookAndFeelDefaults (laf, FigureView::ColourScheme::dark);

    if (auto obj = settings.getDynamicObject())
    {
        for (auto item : obj->getProperties())
        {
            auto id = LookAndFeelHelpers::colourIdFromString (item.name.toString());
            auto colour = DataHelpers::colourFromVar (item.value);

            if (colour != Colours::transparentWhite)
            {
                laf.setColour (id, colour);
            }
        }
    }
    mainWindow->sendLookAndFeelChange();
}




//=============================================================================
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

    LookAndFeelHelpers::setLookAndFeelDefaults (laf, LookAndFeelHelpers::BackgroundScheme::dark);
    LookAndFeelHelpers::setLookAndFeelDefaults (laf, LookAndFeelHelpers::TextColourScheme::pastels2);
    FigureView        ::setLookAndFeelDefaults (laf, FigureView::ColourScheme::dark);
}

bool PatchViewApplication::presentOpenDirectoryDialog()
{
    FileChooser chooser ("Open directory...", mainWindow->content->getCurrentDirectory(), "", true, false, nullptr);

    if (chooser.browseForDirectory())
    {
        auto nwd = chooser.getResult();
        mainWindow->content->setCurrentDirectory (nwd);
        applicationProperties.getUserSettings()->setValue ("LastCurrentDirectory", nwd.getFullPathName());
    }
    return true;
}

PropertiesFile::Options PatchViewApplication::makePropertiesFileOptions()
{
    PropertiesFile::Options opts;
    opts.applicationName = getApplicationName();
    opts.commonToAllUsers = false;
    opts.doNotSave = false;
    opts.filenameSuffix = ".settings";
    opts.folderName = "";
    opts.osxLibrarySubFolder = "Application Support";
    opts.ignoreCaseOfKeyNames = false;
    opts.millisecondsBeforeSaving = 1000;
    opts.storageFormat = PropertiesFile::StorageFormat::storeAsXML;
    return opts;
}
