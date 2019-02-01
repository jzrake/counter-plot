#pragma once
#include "JuceHeader.h"
#include "DirectoryTree.hpp"
#include "TableView.hpp"
#include "../Core/EditorKeyMappings.hpp"
#include "../Core/FileWatcher.hpp"
#include "../Core/Program.hpp"
#include "../Plotting/FigureView.hpp"




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
    Array<File> getSelectedSources() const;

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
        Image thumbnail;
    };

    void sendSelectedSourceChanged (int row);
    ListenerList<Listener> listeners;
    Array<File> sources;
    Array<SourceAssets> assets;
    ListBox list;
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
class MainComponent
: public Component
, public DirectoryTree::Listener
, public SourceList::Listener
, public FigureView::MessageSink
{
public:

    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);
    void setCurrentFile (File newCurrentFile);
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
    DirectoryTree& getDirectoryTree();
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


private:
    //=========================================================================
    void layout (bool animated);

    //=========================================================================
    bool directoryTreeShowing = true;
    bool environmentViewShowing = false;
    bool kernelRuleEntryShowing = false;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    SourceList sourceList;
    EitherOrComponent sidebar;
    FileWatcher fileWatcher;
    File currentFile;
    cp::Program program;
};
