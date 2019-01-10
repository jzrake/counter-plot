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
    void setCurrentViewerName (const String& viewerName);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;
private:
    BusyIndicatorStatus status = BusyIndicatorStatus::idle;
    Point<double> mousePositionInFigure;
    String currentViewerName;
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
    File currentFile;
    bool directoryTreeShowing = true;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    OwnedArray<FileBasedView> views;
};
