#include "MainComponent.hpp"
#include "../Core/LookAndFeel.hpp"
#include "../Core/Main.hpp"
#include "../Viewers/JetInCloudView.hpp"
#include "../Viewers/BinaryTorquesView.hpp"
#include "../Viewers/Viewer.hpp"
#include "../Viewers/ColourMapViewer.hpp"
#include "../Viewers/UserExtensionView.hpp"




//=============================================================================
class DefaultView : public Viewer
{
public:

    //=========================================================================
    bool isInterestedInFile (File) const override { return true; }
    void loadFile (File) override {}
    String getViewerName() const override { return String(); }

    //=========================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (findColour (LookAndFeelHelpers::propertyViewBackground));
    }
};




//=============================================================================
StatusBar::EnvironmentViewToggleButton::EnvironmentViewToggleButton() : Button ("")
{
    setWantsKeyboardFocus (false);
}

void StatusBar::EnvironmentViewToggleButton::paintButton (Graphics& g,
                                                          bool highlighted,
                                                          bool /*down*/)
{
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        auto text = String (main->isEnvironmentViewShowing() ? "Hide" : "Show") + " Viewer Environment";
        g.setFont (Font().withHeight (11));
        g.setColour (findColour (LookAndFeelHelpers::statusBarText).brighter (highlighted ? 0.2f : 0.0f));
        g.drawText (text, getLocalBounds().withTrimmedLeft(8), Justification::centredLeft);
    }
}




//=============================================================================
StatusBar::ViewerNamePopupButton::ViewerNamePopupButton() : Button ("")
{
    setWantsKeyboardFocus (false);
}

void StatusBar::ViewerNamePopupButton::paintButton (Graphics& g,
                                                    bool highlighted,
                                                    bool /*down*/)
{
    g.setFont (Font().withHeight (11));
    g.setColour (findColour (LookAndFeelHelpers::statusBarText).brighter (highlighted ? 0.2f : 0.0f));
    g.drawText (currentViewerName, getLocalBounds().withTrimmedRight(8), Justification::centredRight);
}

void StatusBar::ViewerNamePopupButton::clicked()
{
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        PopupMenu menu;
        StringArray names;
        File currentFile = main->getCurrentFile();
        const Viewer* currentViewer = main->getCurrentViewer();

        for (auto viewer : main->getViewerCollection().getAllComponents())
        {
            auto name = viewer->getViewerName();
            names.add (name);
            menu.addItem (names.size(), name, viewer->isInterestedInFile (currentFile), viewer == currentViewer);
        }
        int result = menu.show();

        if (result != 0)
        {
            main->setCurrentViewer (names[result - 1]);
        }
    }
}




//=============================================================================
StatusBar::StatusBar()
{
    environmentViewToggleButton.setCommandToTrigger (&PatchViewApplication::getApp().getCommandManager(),
                                                     PatchViewApplication::Commands::toggleEnvironmentView,
                                                     true);
    addAndMakeVisible (environmentViewToggleButton);
    addAndMakeVisible (viewerNamePopupButton);
}

StatusBar::~StatusBar()
{
}

void StatusBar::incrementAsyncTaskCount()
{
    ++numberOfAsyncTasks;
    repaint();
}

void StatusBar::decrementAsyncTaskCount()
{
    --numberOfAsyncTasks;
    repaint();
}

void StatusBar::setMousePositionInFigure (Point<double> position)
{
    mousePositionInFigure = position;
    repaint();
}

void StatusBar::setCurrentViewerName (const String& viewerName)
{
    viewerNamePopupButton.currentViewerName = viewerName;
    viewerNamePopupButton.repaint();
}

void StatusBar::setCurrentErrorMessage (const String& what)
{
    currentErrorMessage = what;
    repaint();
}




//=============================================================================
void StatusBar::paint (Graphics& g)
{
    auto geom = computeGeometry();
    auto backgroundColour    = findColour (LookAndFeelHelpers::statusBarBackground);
    auto fontColour          = findColour (LookAndFeelHelpers::statusBarText);
    auto errorColour         = findColour (LookAndFeelHelpers::statusBarErrorText);
    auto busyIndicatorColour = numberOfAsyncTasks ? Colours::yellow : Colours::transparentBlack;

    g.setColour (backgroundColour);
    g.fillAll();

    g.setColour (busyIndicatorColour);
    g.fillEllipse (geom.busyIndicatorArea.reduced (5).toFloat());

    g.setColour (fontColour);
    g.setFont (Font().withHeight (11));

    g.drawText (mousePositionInFigure.isOrigin() ? "" : mousePositionInFigure.toString(), geom.mousePositionArea, Justification::centredLeft);

    g.setColour (errorColour);
    g.drawText (currentErrorMessage, geom.errorMessageArea, Justification::centredLeft);
}

void StatusBar::resized()
{
    auto geom = computeGeometry();
    environmentViewToggleButton.setBounds (geom.environmentViewToggleArea);
    viewerNamePopupButton.setBounds (geom.viewerNamePopupArea);
}




//=============================================================================
StatusBar::Geometry StatusBar::computeGeometry() const
{
    auto geom = Geometry();
    auto area = getLocalBounds();
    geom.busyIndicatorArea         = area.removeFromRight (getHeight());
    geom.viewerNamePopupArea       = area.removeFromRight (200);
    geom.mousePositionArea         = area.removeFromRight (120);
    geom.environmentViewToggleArea = area.removeFromLeft  (150);
    geom.errorMessageArea          = area;
    return geom;
}




//=============================================================================
EnvironmentView::EnvironmentView()
{
    list.setModel (this);
    list.getViewport()->setWantsKeyboardFocus (false);
    setColours();
    addAndMakeVisible (list);
}

void EnvironmentView::setKernel (const Runtime::Kernel* kernelToView)
{
    kernel = kernelToView;
    refresh();
}

void EnvironmentView::refresh()
{
    keys.clear();

    if (kernel)
        for (const auto& item : *kernel)
            if ((item.second.flags & Runtime::builtin) == 0)
                keys.add (item.first);

    keys.sort();
    list.updateContent();
    repaint();
}




//=============================================================================
void EnvironmentView::resized()
{
    list.setBounds (getLocalBounds());
}

void EnvironmentView::colourChanged()
{
    setColours();
    repaint();
}

void EnvironmentView::lookAndFeelChanged()
{
    setColours();
    repaint();
}




//=============================================================================
void EnvironmentView::setColours()
{
    list.setColour (ListBox::backgroundColourId, findColour (LookAndFeelHelpers::environmentViewBackground));
}




//=============================================================================
int EnvironmentView::getNumRows()
{
    return keys.size();
}

void EnvironmentView::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    g.fillAll (rowIsSelected ? findColour (ListBox::backgroundColourId).darker() : Colours::transparentBlack);

    auto key = keys[rowNumber];
    auto err = kernel->error_at (key.toStdString());
    auto repr = Runtime::represent (kernel->at (keys[rowNumber].toStdString()));
    auto text1 = findColour (LookAndFeelHelpers::environmentViewText1);
    auto text2 = findColour (LookAndFeelHelpers::environmentViewText2);

    g.setFont (Font ("Monaco", 11, 0));
    g.setColour (text1);
    g.drawText (keys[rowNumber], 8, 0, width - 16, height, Justification::centredLeft);

    if (err.empty())
    {
        g.setColour (text2);
        g.drawText (repr, 8, 0, width - 16, height, Justification::centredRight);
    }
    else
    {
        g.setColour (Colours::orange);
        g.drawText (err, 8, 0, width - 16, height, Justification::centredRight);
    }
}




//=============================================================================
MainComponent::MainComponent()
{
    directoryTree.addListener (this);
    viewers.addListener (this);
    viewers.add (std::make_unique<JsonFileViewer>());
    viewers.add (std::make_unique<ImageFileViewer>());
    viewers.add (std::make_unique<ColourMapViewer>());
    viewers.add (std::unique_ptr<Viewer> (BinaryTorques::create()));
    viewers.add (std::unique_ptr<Viewer> (JetInCloud::create()));
    viewers.loadAllInDirectory (File ("/Users/jzrake/Work/CounterPlot/Viewers"));

    directoryTree.getTreeView().setWantsKeyboardFocus (directoryTreeShowing);
    environmentView.getListBox().setWantsKeyboardFocus (environmentViewShowing);

    addAndMakeVisible (directoryTree);

    for (auto view : viewers.getAllComponents())
        addChildComponent (view);

    addAndMakeVisible (environmentView);
    addAndMakeVisible (statusBar);

    setSize (1024, 768 - 64);
}

MainComponent::~MainComponent()
{
}

void MainComponent::setCurrentDirectory (File newCurrentDirectory)
{
    directoryTree.setDirectoryToShow (newCurrentDirectory);
}

void MainComponent::reloadCurrentFile()
{
    if (currentViewer)
        currentViewer->loadFile (currentFile);
}

void MainComponent::reloadDirectoryTree()
{
    directoryTree.reloadAll();
}

void MainComponent::toggleDirectoryTreeShown()
{
    directoryTreeShowing = ! directoryTreeShowing;
    directoryTree.getTreeView().setWantsKeyboardFocus (directoryTreeShowing);
    layout (true);
}

void MainComponent::toggleEnvironmentViewShown()
{
    environmentViewShowing = ! environmentViewShowing;
    environmentView.getListBox().setWantsKeyboardFocus (environmentViewShowing);
    layout (true);
}

bool MainComponent::isDirectoryTreeShowing() const
{
    return directoryTreeShowing;
}

bool MainComponent::isEnvironmentViewShowing() const
{
    return environmentViewShowing;
}

File MainComponent::getCurrentDirectory() const
{
    return directoryTree.getCurrentDirectory();
}

File MainComponent::getCurrentFile() const
{
    return currentFile;
}

const Viewer* MainComponent::getCurrentViewer() const
{
    return currentViewer;
}

const ViewerCollection& MainComponent::getViewerCollection() const
{
    return viewers;
}

void MainComponent::setCurrentViewer (const String& viewerName)
{
    if (auto viewer = viewers.findViewerWithName (viewerName))
    {
        makeViewerCurrent (viewer);
    }
}

void MainComponent::makeViewerCurrent (Viewer* viewer)
{
    if (viewer != currentViewer)
    {
        if (viewer)
        {
            viewer->loadFile (currentFile);
            statusBar.setCurrentViewerName (viewer->getViewerName());
            environmentView.setKernel (viewer->getKernel());
            viewers.showOnly (viewer);
        }
        else
        {
            statusBar.setCurrentViewerName (String());
            environmentView.setKernel (nullptr);
            viewers.showOnly (nullptr);
        }
        currentViewer = viewer;
    }
}




//=============================================================================
void MainComponent::resized()
{
    layout (false);
}




//=============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    currentFile = file;

    if (currentViewer && currentViewer->isInterestedInFile (currentFile))
    {
        currentViewer->loadFile (currentFile);
    }
    else if (auto viewer = viewers.findViewerForFile (file))
    {
        makeViewerCurrent (viewer);
    }
}




//=============================================================================
void MainComponent::figureMousePosition (Point<double> position)
{
    statusBar.setMousePositionInFigure (position);
}




//=============================================================================
void MainComponent::viewerAsyncTaskStarted()
{
    statusBar.incrementAsyncTaskCount();
}

void MainComponent::viewerAsyncTaskFinished()
{
    statusBar.decrementAsyncTaskCount();
}

void MainComponent::viewerLogErrorMessage (const String& what)
{
    statusBar.setCurrentErrorMessage (what);
}

void MainComponent::viewerIndicateSuccess()
{
    statusBar.setCurrentErrorMessage (String());
}

void MainComponent::viewerEnvironmentChanged()
{
    environmentView.refresh();
}




//=============================================================================
void MainComponent::extensionViewerReconfigured (UserExtensionView* viewer)
{
    if (viewer == currentViewer)
    {
        if (viewer->isInterestedInFile (currentFile))
        {
            statusBar.setCurrentViewerName (viewer->getViewerName());
            environmentView.setKernel (viewer->getKernel());
        }
        else // the viewer is no longer interested in the current file...
        {
            makeViewerCurrent (viewers.findViewerForFile (currentFile));
        }
    }
    else if (currentViewer == nullptr && viewer->isInterestedInFile (currentFile))
    {
        makeViewerCurrent (viewer);
    }
}




//=============================================================================
void MainComponent::layout (bool animated)
{
    auto setBounds = [animated] (Component& component, const Rectangle<int>& bounds)
    {
        if (animated && component.isVisible())
            Desktop::getInstance().getAnimator().animateComponent (&component, bounds, 1.f, 200, false, 1.f, 1.f);
        else
            component.setBounds (bounds);
    };

    auto area = getLocalBounds();
    auto statusBarArea = area.removeFromBottom (22);
    auto directoryTreeArea = directoryTreeShowing ? area.removeFromLeft (300) : area.withWidth (300).translated (-300, 0);
    auto environmentViewArea = Rectangle<int> (0, 0, 300, 330)
    .withBottomY (statusBarArea.getY())
    .translated (0, environmentViewShowing ? 0 : 330 + 22); // the 22 is to ensure it's offscreen, so not painted

    setBounds (statusBar, statusBarArea);
    setBounds (directoryTree, directoryTreeArea);
    setBounds (environmentView, environmentViewArea);
    viewers.setBounds (area, animated);
}
