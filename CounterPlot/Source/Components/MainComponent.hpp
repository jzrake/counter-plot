#pragma once
#include "JuceHeader.h"
#include "DirectoryTree.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Viewers/Viewer.hpp"
#include "../Viewers/UserExtensionView.hpp"
#include "../Core/ViewerCollection.hpp"
#include "../Core/Runtime.hpp"




//=============================================================================
class StatusBar : public Component
{
public:

    //=========================================================================
    class EnvironmentViewToggleButton : public Button
    {
    public:
        EnvironmentViewToggleButton();
        void paintButton (Graphics& g,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
    };

    //=========================================================================
    StatusBar();

    //=========================================================================
    void incrementAsyncTaskCount();
    void decrementAsyncTaskCount();
    void setMousePositionInFigure (Point<double> position);
    void setCurrentViewerName (const String& viewerName);
    void setCurrentErrorMessage (const String& what);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    EnvironmentViewToggleButton environmentViewToggleButton;
    Point<double> mousePositionInFigure;
    String currentViewerName;
    String currentErrorMessage;
    int numberOfAsyncTasks = 0;
};




//=============================================================================
class EnvironmentView : public Component, public ListBoxModel
{
public:

    //=========================================================================
    EnvironmentView();
    void setKernel (const Runtime::Kernel* kernelToView);
    void refresh();

    //=========================================================================
    void resized() override;
    void colourChanged() override;
    void lookAndFeelChanged() override;

    //=========================================================================
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;

private:
    //=========================================================================
    void setColours();

    //=========================================================================
    ListBox list;
    const Runtime::Kernel* kernel = nullptr;
    Array<String> keys;
};




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
, public FigureView::MessageSink
, public Viewer::MessageSink
, public ViewerCollection::Listener
{
public:

    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);
    void reloadCurrentFile();
    void reloadDirectoryTree();
    void toggleDirectoryTreeShown();
    void toggleEnvironmentViewShown();
    bool isDirectoryTreeShowing() const;
    bool isEnvironmentViewShowing() const;
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
    void viewerAsyncTaskStarted() override;
    void viewerAsyncTaskFinished() override;
    void viewerLogErrorMessage (const String&) override;
    void viewerIndicateSuccess() override;
    void viewerEnvironmentChanged() override;

    //=========================================================================
    void extensionViewerReconfigured (UserExtensionView*) override;

private:
    //=========================================================================
    void layout (bool animated);

    //=========================================================================
    File currentFile;
    bool directoryTreeShowing = true;
    bool environmentViewShowing = false;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    ViewerCollection viewers;
    EnvironmentView environmentView;
};
