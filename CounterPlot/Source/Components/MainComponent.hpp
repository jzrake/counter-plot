#pragma once
#include "JuceHeader.h"
#include "DirectoryTree.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Viewers/Viewer.hpp"
#include "../Viewers/UserExtensionView.hpp"
#include "../Core/ViewerCollection.hpp"
#include "../Core/Runtime.hpp"
#include "../Core/TaskPool.hpp"
#include "../Core/DataHelpers.hpp" // FilePoller
#include "../Core/EditorKeyMappings.hpp"
#include "../Plotting/ResizerFrame.hpp"




//=============================================================================
class StatusBar : public Component, public Timer
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
    class ViewerNamePopupButton : public Button
    {
    public:
        ViewerNamePopupButton();
        void paintButton (Graphics& g,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
        void clicked() override;
        String currentViewerName;
    };

    //=========================================================================
    struct Geometry
    {
        Rectangle<int> environmentViewToggleArea;
        Rectangle<int> messageArea;
        Rectangle<int> mousePositionArea;
        Rectangle<int> viewerNamePopupArea;
        Rectangle<int> busyIndicatorArea;
    };

    //=========================================================================
    StatusBar();
    ~StatusBar();

    //=========================================================================
    void incrementAsyncTaskCount();
    void decrementAsyncTaskCount();
    void setMousePositionInFigure (Point<double> position);
    void setCurrentViewerName (const String& viewerName);
    void setCurrentErrorMessage (const String& what);
    void setCurrentInfoMessage (const String& info, int millisecondsToDisplay=0);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //=========================================================================
    void timerCallback() override;

private:

    //=========================================================================
    Geometry computeGeometry() const;

    //=========================================================================
    EnvironmentViewToggleButton environmentViewToggleButton;
    ViewerNamePopupButton viewerNamePopupButton;
    Point<double> mousePositionInFigure;
    String currentErrorMessage;
    String currentInfoMessage;
    int millisecondsToDisplayInfo = 0;
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
    ListBox& getListBox() { return list; }

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
class KernelRuleEntry : public Component, public TextEditor::Listener
{
public:
    KernelRuleEntry();

    //=========================================================================
    void resized() override;

    //=========================================================================
    void textEditorTextChanged (TextEditor&) override;
    void textEditorReturnKeyPressed (TextEditor&) override;
    void textEditorEscapeKeyPressed (TextEditor&) override;
    void textEditorFocusLost (TextEditor&) override;

private:
    TextEditor editor;
    EditorKeyMappings keyMappings;
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
    void toggleDirectoryTreeShown (bool animated=true);
    void toggleEnvironmentViewShown (bool animated=true);
    void toggleKernelRuleEntryShown();
    bool hideExtraComponents();
    bool isDirectoryTreeShowing() const;
    bool isEnvironmentViewShowing() const;
    bool isKernelRuleEntryShowing() const;
    File getCurrentDirectory() const;
    File getCurrentFile() const;
    const Viewer* getCurrentViewer() const;
    const ViewerCollection& getViewerCollection() const;
    DirectoryTree& getDirectoryTree();
    void setCurrentViewer (const String& viewerName);
    void makeViewerCurrent (Viewer* viewer);
    void refreshCurrentViewerName();
    void sendMessageToCurrentViewer (String& message);
    bool canSendMessagesToCurrentViewer() const;

    //=========================================================================
    void resized() override;

    //=========================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

    //=========================================================================
    void figureMousePosition (Point<double> position) override;

    //=========================================================================
    void viewerAsyncTaskStarted (const String& name) override;
    void viewerAsyncTaskCompleted (const String& name) override;
    void viewerAsyncTaskCancelled (const String& name) override;
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
    bool kernelRuleEntryShowing = false;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    ViewerCollection viewers;
    Viewer* currentViewer = nullptr;
    EnvironmentView environmentView;
    KernelRuleEntry kernelRuleEntry;
    FilePoller filePoller;
};
