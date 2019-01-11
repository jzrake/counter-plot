#pragma once
#include "JuceHeader.h"
#include "Views/DirectoryTree.hpp"
#include "Views/FigureView.hpp"
#include "Views/FileBasedView.hpp"




//=============================================================================
class StatusBar : public Component
{
public:

    //=========================================================================
    void incrementAsyncTaskCount();
    void decrementAsyncTaskCount();
    void setMousePositionInFigure (Point<double> position);
    void setCurrentViewerName (const String& viewerName);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    Point<double> mousePositionInFigure;
    String currentViewerName;
    int numberOfAsyncTasks = 0;
};




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
, public FigureView::MessageSink
, public FileBasedView::MessageSink
{
public:

    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);
    void reloadCurrentFile();
    void reloadDirectoryTree();
    void toggleDirectoryTreeShown();
    bool isDirectoryTreeShowing() const;
    File getCurrentDirectory() const;

    //=========================================================================
    void paint (Graphics&) override;
    void paintOverChildren (Graphics& g) override;
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

    //=========================================================================
    void figureMousePosition (Point<double> position) override;

    //=========================================================================
    void fileBasedViewAsyncTaskStarted() override;
    void fileBasedViewAsyncTaskFinished() override;

private:
    //=========================================================================
    void layout (bool animated);

    //=========================================================================
    File currentFile;
    bool directoryTreeShowing = true;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    OwnedArray<FileBasedView> views;
};
