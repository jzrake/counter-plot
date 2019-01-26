#pragma once
#include "JuceHeader.h"
#include "DirectoryTree.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Viewers/Viewer.hpp"
#include "../Viewers/UserExtensionView.hpp"
#include "../Core/ViewerCollection.hpp"
#include "../Core/Runtime.hpp"
#include "../Core/TaskPool.hpp"
#include "../Core/DataHelpers.hpp"
#include "../Core/EditorKeyMappings.hpp"
#include "../Plotting/ResizerFrame.hpp"




//=============================================================================
class EitherOrComponent : public Component
{
public:

    //=========================================================================
    EitherOrComponent();
    void setComponent1 (Component* componentToShow);
    void setComponent2 (Component* componentToShow);
    void showComponent1();
    void showComponent2();

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:

    //=========================================================================
    class TabButton : public Button
    {
    public:
        TabButton();
        void paintButton (Graphics& g, bool, bool) override;
    };

    WeakReference<Component> component1;
    WeakReference<Component> component2;
    TabButton button1;
    TabButton button2;
};




//=============================================================================
class SourceList : public Component, public ListBoxModel
{
public:

    //=========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void sourceListSelectedSourceChanged (SourceList*, File) = 0;
        virtual void sourceListWantsCaptureOfCurrent (SourceList*) = 0;
    };

    //=========================================================================
    SourceList();
    void addListener (Listener*);
    void removeListener (Listener*);
    void clear();
    void setSources (const Array<File>& sourcesToShow);
    void addSource (File source);
    void removeSource (File source);
    void removeSourceAtRow (int row);
    void setCaptureForSource (File source, Image capturedImage);
    Array<Image> getAllImageAssets() const;

    //=========================================================================
    void resized() override;
    void paint (Graphics& g) override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged (int lastRowSelected) override;
    void deleteKeyPressed (int row) override;
    void listBoxItemDoubleClicked (int row, const MouseEvent& e) override;

    String getTooltipForRow (int row) override;

private:
    //=========================================================================
    struct SourceAssets
    {
        Image capture;
    };

    void sendSelectedSourceChanged (int row);
    ListenerList<Listener> listeners;
    Array<File> sources;
    Array<SourceAssets> assets;
    ListBox list;
};




//=============================================================================
class UserExtensionsDirectoryEditor : public Component, public TextEditor::Listener
{
public:
    
    //=========================================================================
    UserExtensionsDirectoryEditor();
    void setDirectories (const Array<File>& directories);
    Array<File> getDirectories() const;

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    void colourChanged() override;
    void lookAndFeelChanged() override;

    //=========================================================================
    void textEditorTextChanged (TextEditor&) override;
    void textEditorReturnKeyPressed (TextEditor&) override;
    void textEditorEscapeKeyPressed (TextEditor&) override;
    void textEditorFocusLost (TextEditor&) override;

private:
    //=========================================================================
    void setColours();
    bool areContentsValid() const;
    bool sendContentsToMainViewerCollection();
    String toRelativePath (const File& file) const;
    File fromRelativePath (const String& path) const;
    TextEditor editor;
    EditorKeyMappings mappings;
};




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
    void colourChanged() override;
    void lookAndFeelChanged() override;

    //=========================================================================
    void timerCallback() override;

private:

    //=========================================================================
    Geometry computeGeometry() const;
    void setColours();

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
    void selectedRowsChanged (int lastRowSelected) override;
    String getTooltipForRow (int row) override;

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
    void loadRule (const std::string& rule, const Runtime::Kernel& kernel);
    void refresh (const Runtime::Kernel* kernel);
    bool recallNext();
    bool recallPrev();

    //=========================================================================
    void resized() override;
    void colourChanged() override;
    void lookAndFeelChanged() override;

    //=========================================================================
    void textEditorTextChanged (TextEditor&) override;
    void textEditorReturnKeyPressed (TextEditor&) override;
    void textEditorEscapeKeyPressed (TextEditor&) override;
    void textEditorFocusLost (TextEditor&) override;

private:
    //=========================================================================
    void setColours();

    TextEditor editor;
    EditorKeyMappings keyMappings;
    StringArray history;
    String loadedRule;
    String loadedText;
    int indexInHistory = 0;
};




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
, public SourceList::Listener
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
    void toggleUserExtensionsDirectoryEditor();
    bool hideExtraComponents();
    bool isDirectoryTreeShowing() const;
    bool isEnvironmentViewShowing() const;
    bool isKernelRuleEntryShowing() const;
    bool isUserExtensionsDirectoryEditorShowing() const;
    File getCurrentDirectory() const;
    File getCurrentFile() const;
    const Viewer* getCurrentViewer() const;
    ViewerCollection& getViewerCollection();
    DirectoryTree& getDirectoryTree();
    bool isViewerSuitable (Viewer*) const;
    void setCurrentViewer (const String& viewerName);
    void makeViewerCurrent (Viewer* viewer);
    void refreshCurrentViewerName();
    bool sendMessageToCurrentViewer (String& message);
    bool canSendMessagesToCurrentViewer() const;
    void showKernelRule (const String& rule);
    void indicateSuccess (const String& info);
    void logErrorMessage (const String& what);
    void createAnimation (bool toTempDirectory);

    //=========================================================================
    void resized() override;

    //=========================================================================
    void directoryTreeSelectedFileChanged (DirectoryTree*, File) override;
    void directoryTreeWantsFileToBeSource (DirectoryTree*, File) override;

    //=========================================================================
    void sourceListSelectedSourceChanged (SourceList*, File) override;
    void sourceListWantsCaptureOfCurrent (SourceList*) override;

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
    void viewerCollectionViewerReconfigured (Viewer*) override;
    void viewerCollectionViewerAdded (Viewer*) override;
    void viewerCollectionViewerRemoved (Viewer*) override;

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
    SourceList sourceList;
    ViewerCollection viewers;
    Viewer* currentViewer = nullptr;
    EnvironmentView environmentView;
    KernelRuleEntry kernelRuleEntry;
    UserExtensionsDirectoryEditor userExtensionsDirectoryEditor;
    EitherOrComponent sidebar;
    FilePoller filePoller;
};
