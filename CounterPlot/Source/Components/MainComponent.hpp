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
    void incrementAsyncTaskCount();
    void decrementAsyncTaskCount();
    void setMousePositionInFigure (Point<double> position);
    void setCurrentViewerName (const String& viewerName);
    void setCurrentErrorMessage (const String& what);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
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

    //=========================================================================
    void resized() override;

    //=========================================================================
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;
    Component *refreshComponentForRow (int rowNumber, bool isRowSelected, Component *existingComponentToUpdate) override { return nullptr; }
    void listBoxItemClicked (int row, const MouseEvent &) override {}
    void listBoxItemDoubleClicked (int row, const MouseEvent &) override {}
    void backgroundClicked (const MouseEvent&) override {}
    void selectedRowsChanged (int lastRowSelected) override {}
    void deleteKeyPressed (int lastRowSelected) override {}
    void returnKeyPressed (int lastRowSelected) override {}
    void listWasScrolled () override {}
    var getDragSourceDescription (const SparseSet< int > &rowsToDescribe) override { return var(); }
    String getTooltipForRow (int row) override { return String(); }
    MouseCursor getMouseCursorForRow (int row) override { return MouseCursor::NormalCursor; }

private:
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
    void viewerAsyncTaskStarted() override;
    void viewerAsyncTaskFinished() override;
    void viewerLogErrorMessage (const String&) override;
    void viewerIndicateSuccess() override;

    //=========================================================================
    void extensionViewerReconfigured (UserExtensionView*) override;

private:
    //=========================================================================
    void layout (bool animated);

    //=========================================================================
    File currentFile;
    bool directoryTreeShowing = true;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    ViewerCollection viewers;
    EnvironmentView environmentView;
};
