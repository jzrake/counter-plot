#include "Main.hpp"
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
PatchViewApplication::MainWindow::MainWindow (String name) : DocumentWindow (name, Colours::darkgrey, DocumentWindow::allButtons)
{
    addKeyListener (getApp().commandManager->getKeyMappings());
    Desktop::getInstance().addFocusChangeListener (this);

    content = std::make_unique<MainComponent>();

    setUsingNativeTitleBar (true);
    setContentNonOwned (content.get(), true);
    setResizable (true, true);
    centreWithSize (getWidth(), getHeight());
    setVisible (true);
}

PatchViewApplication::MainWindow::~MainWindow()
{
    Desktop::getInstance().removeFocusChangeListener (this);
}

bool PatchViewApplication::MainWindow::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::escapeKey)
    {
        return content->hideExtraComponents();
    }
    return false;
}

void PatchViewApplication::MainWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}

void PatchViewApplication::MainWindow::paintOverChildren (Graphics& g)
{
    if (auto focusedComponent = Component::getCurrentlyFocusedComponent())
    {
        auto bounds = focusedComponent->getScreenBounds().translated (-getX(), -getY());
        g.setColour (Colours::orange);
        g.drawLine (bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getBottom());
    }
}

void PatchViewApplication::MainWindow::globalFocusChanged (Component*)
{
    repaint();
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
        menu.addCommandItem (manager, Commands::reloadCurrentFile);
        menu.addSeparator();
        menu.addCommandItem (manager, Viewer::Commands::makeSnapshotAndOpen);
        menu.addCommandItem (manager, Viewer::Commands::saveSnapshotAs);
        menu.addCommandItem (manager, Commands::makeAnimationAndOpen);
        menu.addCommandItem (manager, Commands::saveAnimationAs);
        return menu;
    }
    if (menuName == "View")
    {
        menu.addCommandItem (manager, Commands::toggleUserExtensionsDirectoryEditor);
        menu.addCommandItem (manager, Commands::toggleEnvironmentView);
        menu.addCommandItem (manager, Commands::toggleKernelRuleEntry);
        menu.addCommandItem (manager, Commands::toggleDirectoryView);
        menu.addCommandItem (manager, Commands::reloadDirectoryView);
        menu.addSeparator();
        menu.addCommandItem (manager, Commands::increaseFontSize);
        menu.addCommandItem (manager, Commands::decreaseFontSize);
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

    currentDirectory = settings.getValue ("CurrentDirectory", File::getSpecialLocation (File::userHomeDirectory).getFullPathName());
    auto defaultFont = settings.getValue ("Font", Font().toString());
    auto directoryTreeState = std::unique_ptr<XmlElement> (settings.getXmlValue ("DirectoryTreeState"));
    auto userExtensionDirectories = std::unique_ptr<XmlElement> (settings.getXmlValue ("UserExtensionDirectories"));

    configureLookAndFeel();
    lookAndFeel.setDefaultFont (Font::fromString (defaultFont));
    LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    commandManager = std::make_unique<ApplicationCommandManager>();
    menu           = std::make_unique<MainMenu>();
    mainWindow     = std::make_unique<MainWindow> (getApplicationName());
    mainWindow->content->setCurrentDirectory (currentDirectory);

    if (directoryTreeState)
        mainWindow->content->getDirectoryTree().restoreRootOpenness (*directoryTreeState);
    if (userExtensionDirectories)
        mainWindow->content->getViewerCollection().setWatchedDirectories (*userExtensionDirectories);

    commandManager->registerAllCommandsForTarget (this);
    Viewer::registerCommands (*commandManager);

    MenuBarModel::setMacMainMenu (menu.get(), nullptr);
}

void PatchViewApplication::shutdown()
{
    assert(mainWindow != nullptr); // The main window should not be closed before shutdown

    auto& settings = *applicationProperties.getUserSettings();

    settings.setValue ("CurrentDirectory", currentDirectory.getFullPathName());
    settings.setValue ("Font", lookAndFeel.getDefaultFont().toString());
    settings.setValue ("DirectoryTreeState",  mainWindow->content->getDirectoryTree().getRootOpennessState().get());
    settings.setValue ("UserExtensionDirectories",  mainWindow->content->getViewerCollection().getWatchedDirectoriesAsXml().get());

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
        Commands::toggleUserExtensionsDirectoryEditor,
        Commands::toggleDirectoryView,
        Commands::reloadDirectoryView,
        Commands::toggleEnvironmentView,
        Commands::toggleKernelRuleEntry,
        Commands::increaseFontSize,
        Commands::decreaseFontSize,
        Commands::makeAnimationAndOpen,
        Commands::saveAnimationAs,
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
        case Commands::toggleUserExtensionsDirectoryEditor:
            result.setInfo ("Edit Extensions Directories", "", "View",
                            mainWindow && mainWindow->content->isUserExtensionsDirectoryEditorShowing() ? ApplicationCommandInfo::isTicked : 0);
            result.defaultKeypresses.add (KeyPress ('<', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
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
        case Commands::toggleKernelRuleEntry:
            result.setInfo ("Show Rule Entry", "", "View",
                            (  mainWindow &&   mainWindow->content->isKernelRuleEntryShowing()       ? ApplicationCommandInfo::isTicked : 0 |
                             ! mainWindow || ! mainWindow->content->canSendMessagesToCurrentViewer() ? ApplicationCommandInfo::isDisabled : 0));
            result.defaultKeypresses.add (KeyPress ('G', ModifierKeys::commandModifier, 0));
            break;
        case Commands::increaseFontSize:
            result.setInfo ("Increase Font Size", "", "View", 0);
            result.defaultKeypresses.add (KeyPress ('=', ModifierKeys::commandModifier, '+'));
            break;
        case Commands::decreaseFontSize:
            result.setInfo ("Decrease Font Size", "", "View", 0);
            result.defaultKeypresses.add (KeyPress ('-', ModifierKeys::commandModifier, '-'));
            break;
        case Commands::makeAnimationAndOpen:
            result.setInfo ("Create Animation", "", "File", 0);
            result.defaultKeypresses.add (KeyPress ('a', ModifierKeys::altModifier | ModifierKeys::commandModifier, 0));
            break;
        case Commands::saveAnimationAs:
            result.setInfo ("Save Animation As...", "", "File", 0);
            result.defaultKeypresses.add (KeyPress ('a', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
            break;
        default:
            JUCEApplication::getCommandInfo (commandID, result);
            break;
    }
}

bool PatchViewApplication::perform (const InvocationInfo& info)
{
    auto main = mainWindow->content.get();

    switch (info.commandID)
    {
        case Commands::openDirectory:             return presentOpenDirectoryDialog();
        case Commands::toggleUserExtensionsDirectoryEditor: main->toggleUserExtensionsDirectoryEditor(); return true;
        case Commands::toggleEnvironmentView:     main->toggleEnvironmentViewShown(); return true;
        case Commands::toggleKernelRuleEntry:     main->toggleKernelRuleEntryShown(); return true;
        case Commands::toggleDirectoryView:       main->toggleDirectoryTreeShown(); return true;
        case Commands::reloadDirectoryView:       main->reloadDirectoryTree(); return true;
        case Commands::reloadCurrentFile:         main->reloadCurrentFile(); return true;
        case Commands::increaseFontSize:          lookAndFeel.incrementFontSize (+1); mainWindow->sendLookAndFeelChange(); return true;
        case Commands::decreaseFontSize:          lookAndFeel.incrementFontSize (-1); mainWindow->sendLookAndFeelChange(); return true;
        case Commands::makeAnimationAndOpen:      main->createAnimation (true); return true;
        case Commands::saveAnimationAs:           main->createAnimation (false); return true;
        default:                                  return JUCEApplication::perform (info);
    }
}




//=============================================================================
void PatchViewApplication::configureLookAndFeel()
{
    auto& laf = lookAndFeel;
    laf.setColour (TextEditor::backgroundColourId, Colours::white);
    laf.setColour (TextEditor::textColourId, Colours::black);
    laf.setColour (TextEditor::highlightColourId, Colours::lightblue);
    laf.setColour (TextEditor::highlightedTextColourId, Colours::black);
    laf.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    laf.setColour (TextEditor::focusedOutlineColourId, Colours::lightblue);

    laf.setColour (Label::ColourIds::textColourId, Colours::lightgrey);
    laf.setColour (Label::ColourIds::textWhenEditingColourId, Colours::lightgrey);
    laf.setColour (Label::ColourIds::backgroundWhenEditingColourId, Colours::white);
    laf.setColour (ScrollBar::ColourIds::thumbColourId, Colours::cornflowerblue.withAlpha (0.5f));

    AppLookAndFeel::setLookAndFeelDefaults (laf, AppLookAndFeel::BackgroundScheme::dark);
    AppLookAndFeel::setLookAndFeelDefaults (laf, AppLookAndFeel::TextColourScheme::pastels1);
    FigureView    ::setLookAndFeelDefaults (laf, FigureView::ColourScheme::dark);
}

bool PatchViewApplication::presentOpenDirectoryDialog()
{
    FileChooser chooser ("Open directory...", mainWindow->content->getCurrentDirectory(), "", true, false, nullptr);

    if (chooser.browseForDirectory())
    {
        currentDirectory = chooser.getResult();
        mainWindow->content->setCurrentDirectory (currentDirectory);
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
