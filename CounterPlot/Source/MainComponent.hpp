#pragma once
#include "JuceHeader.h"
#include "Views/DirectoryTree.hpp"
#include "Views/FigureView.hpp"




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
    void setMousePositionInFigure (Point<double> position);
    void paint (Graphics& g) override;
    void resized() override;
private:
    BusyIndicatorStatus status = BusyIndicatorStatus::idle;
    Point<double> mousePositionInFigure;
};




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
, public FigureView::MessageSink
{
public:

    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);
    void reloadCurrentFile();
    void toggleDirectoryTreeShown();
    bool isDirectoryTreeShowing() const;

    //=========================================================================
    void paint (Graphics&) override;
    void paintOverChildren (Graphics& g) override;
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

    //=========================================================================
    void figureMousePosition (Point<double> position) override;

private:
    //=========================================================================
    void layout (bool animated);
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
    File currentFile;
    bool directoryTreeShowing = true;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    DataLoadingThread dataLoadingThread;
    OwnedArray<FileBasedView> views;
};
