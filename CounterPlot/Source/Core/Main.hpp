#pragma once
#include "JuceHeader.h"
#include "LookAndFeel.hpp"
#include "../Viewers/UserExtensionView.hpp"




//=============================================================================
class MainComponent;




//=============================================================================
class PatchViewApplication : public JUCEApplication
{
public:


    //=========================================================================
    enum Commands
    {
        openDirectory                       = 101,
        reloadCurrentFile                   = 102,
        toggleDirectoryView                 = 103,
        reloadDirectoryView                 = 104,
        toggleEnvironmentView               = 105,
        toggleKernelRuleEntry               = 106,
        toggleUserExtensionsDirectoryEditor = 107,
        increaseFontSize                    = 108,
        decreaseFontSize                    = 109,
        makeAnimationAndOpen                = 110,
        saveAnimationAs                     = 111,
    };


    //=========================================================================
    PatchViewApplication();
    virtual ~PatchViewApplication() {}
    static PatchViewApplication& getApp();
    ApplicationCommandManager& getCommandManager();


    //=========================================================================
    const String getApplicationName() override;
    const String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override;
    void initialise (const String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void anotherInstanceStarted (const String& commandLine) override;


    //==========================================================================
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;


    //=========================================================================
    class MainWindow : public DocumentWindow, public FocusChangeListener
    {
    public:
        MainWindow (String name);
        ~MainWindow();
        bool keyPressed (const KeyPress& key) override;
        void closeButtonPressed() override;
        void paintOverChildren (Graphics& g) override;
        void globalFocusChanged (Component* focusedComponent) override;
        std::unique_ptr<MainComponent> content;
    };


    //=========================================================================
    class MainMenu : public MenuBarModel
    {
    public:
        MainMenu();
        StringArray getMenuBarNames() override;
        PopupMenu getMenuForIndex (int /*topLevelMenuIndex*/, const String& menuName) override;
        void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override;
    };


private:
    //=========================================================================
    void configureLookAndFeel();
    bool presentOpenDirectoryDialog();
    PropertiesFile::Options makePropertiesFileOptions();

    //=========================================================================
    std::unique_ptr<ApplicationCommandManager> commandManager;
    std::unique_ptr<MainMenu> menu;
    std::unique_ptr<MainWindow> mainWindow;
    // TooltipWindow tooltipWindow;
    AppLookAndFeel lookAndFeel;
    ApplicationProperties applicationProperties;
    File currentDirectory;
};
