#pragma once
#include "JuceHeader.h"
#include "Views/UserExtensionView.hpp"




//=============================================================================
class MainComponent;




//=============================================================================
class PatchViewApplication : public JUCEApplication, public Timer
{
public:


    //=========================================================================
    enum Commands
    {
        openDirectory       = 101,
        reloadCurrentFile   = 102,
        toggleDirectoryView = 103,
        reloadDirectoryView = 104,
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
    void timerCallback() override;


    //=========================================================================
    class MainWindow : public DocumentWindow
    {
    public:
        MainWindow (String name);
        ~MainWindow();
        void closeButtonPressed() override;
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
    ApplicationProperties applicationProperties;
    Time settingsLastPolled;
};
