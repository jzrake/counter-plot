#pragma once
#include "JuceHeader.h"
#include "Views/DirectoryTree.hpp"




//=============================================================================
class FileBasedView;




//=============================================================================
class StatusBar : public Component
{
public:

    //=========================================================================
    enum class BusyIndicatorStatus
    {
        running, waiting, idle,
    };

    //=========================================================================
    void setBusyIndicatorStatus (BusyIndicatorStatus newStatus);
    void paint (Graphics& g) override;
    void resized() override;
private:
    BusyIndicatorStatus status = BusyIndicatorStatus::idle;
};




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
{
public:

    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);

    //=========================================================================
    void paint (Graphics&) override;
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

private:
    //=========================================================================
    void dataLoadingThreadWaiting();
    void dataLoadingThreadRunning();
    void dataLoadingThreadFinished();

    //=========================================================================
    class DataLoadingThread : public Thread
    {
    public:
        DataLoadingThread (MainComponent&);
        void loadFileToView (File fileToLoad, FileBasedView* targetView);
        void run() override;
        MainComponent& main;
        File file;
        FileBasedView* view = nullptr;
    };

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    DataLoadingThread dataLoadingThread;
    OwnedArray<FileBasedView> views;
};
