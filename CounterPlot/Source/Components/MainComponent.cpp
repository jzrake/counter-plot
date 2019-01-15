#include "MainComponent.hpp"
#include "../Core/LookAndFeel.hpp"
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
    currentViewerName = viewerName;
    repaint();
}

void StatusBar::setCurrentErrorMessage (const String& what)
{
    currentErrorMessage = what;
    repaint();
}




//=============================================================================
void StatusBar::paint (Graphics& g)
{
    auto area = getLocalBounds();
    auto indicatorArea       = area.removeFromRight (getHeight());
    auto viewerNameArea      = area.removeFromRight (200);
    auto mousePositionArea   = area.removeFromRight (200);
    auto errorMessageArea    = area.withTrimmedLeft (12);
    auto backgroundColour    = findColour (LookAndFeelHelpers::statusBarBackground);
    auto fontColour          = findColour (LookAndFeelHelpers::statusBarText);
    auto errorColour         = findColour (LookAndFeelHelpers::statusBarErrorText);
    auto busyIndicatorColour = numberOfAsyncTasks ? Colours::yellow : Colours::transparentBlack;

    g.setColour (backgroundColour);
    g.fillAll();

    g.setColour (busyIndicatorColour);
    g.fillEllipse (indicatorArea.reduced (5).toFloat());

    g.setColour (fontColour);
    g.setFont (Font ("Monaco", 11, 0));

    g.drawText (mousePositionInFigure.isOrigin() ? "" : mousePositionInFigure.toString(), mousePositionArea, Justification::centredLeft);
    g.drawText (currentViewerName, viewerNameArea, Justification::centredRight);

    g.setColour (errorColour);
    g.drawText (currentErrorMessage, errorMessageArea, Justification::centredLeft);
}

void StatusBar::resized()
{
}




//=============================================================================
EnvironmentView::EnvironmentView()
{
    list.setModel (this);
    addAndMakeVisible (list);
}

void EnvironmentView::setKernel (const Runtime::Kernel* kernelToView)
{
    kernel = kernelToView;
    keys.clear();

    if (kernel)
        for (const auto& item : *kernel)
            keys.add (item.first);

    list.updateContent();

    repaint(); // needed?
}




//=============================================================================
void EnvironmentView::resized()
{
    list.setBounds (getLocalBounds());
}




//=============================================================================
int EnvironmentView::getNumRows()
{
    return keys.size();
}

void EnvironmentView::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    g.fillAll (rowIsSelected ? findColour (ListBox::backgroundColourId).darker() : Colours::transparentBlack);

    auto repr = kernel->at (keys[rowNumber].toStdString()).toString();

    g.setFont (Font().withHeight (11));
    g.setColour (findColour (ListBox::textColourId));
    g.drawText (keys[rowNumber], 8, 0, width - 16, height, Justification::centredLeft);

    g.setColour (findColour (ListBox::textColourId).darker());
    g.drawText (repr, 8, 0, width - 16, height, Justification::centredRight);
}




//=============================================================================
MainComponent::MainComponent()
{
    directoryTree.addListener (this);

    addAndMakeVisible (statusBar);
    addAndMakeVisible (directoryTree);
    addAndMakeVisible (environmentView);

    viewers.addListener (this);
    viewers.add (std::make_unique<JsonFileViewer>());
    viewers.add (std::make_unique<ImageFileViewer>());
    viewers.add (std::make_unique<ColourMapViewer>());
    viewers.add (std::unique_ptr<Viewer> (BinaryTorques::create()));
    viewers.add (std::unique_ptr<Viewer> (JetInCloud::create()));
    viewers.loadAllInDirectory (File ("/Users/jzrake/Work/CounterPlot/Viewers"));

    for (auto view : viewers.getAllComponents())
    {
        addChildComponent (view);
    }

    setSize (1024, 768 - 64);
    reloadCurrentFile();
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
    if (auto component = viewers.findViewerForFile (currentFile))
    {
        component->loadFile (currentFile);
        statusBar.setCurrentViewerName (component->getViewerName());
        viewers.showOnly (component);
    }
}

void MainComponent::reloadDirectoryTree()
{
    directoryTree.reloadAll();
}

void MainComponent::toggleDirectoryTreeShown()
{
    directoryTreeShowing = ! directoryTreeShowing;
    layout (true);
}

bool MainComponent::isDirectoryTreeShowing() const
{
    return directoryTreeShowing;
}

File MainComponent::getCurrentDirectory() const
{
    return directoryTree.getCurrentDirectory();
}




//=============================================================================
void MainComponent::paint (Graphics& g)
{
}

void MainComponent::paintOverChildren (Graphics& g)
{
}

void MainComponent::resized()
{
    layout (false);
}

bool MainComponent::keyPressed (const KeyPress& key)
{
    return false;
}




//=============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    currentFile = file;
    reloadCurrentFile();
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




//=============================================================================
void MainComponent::extensionViewerReconfigured (UserExtensionView* viewer)
{
    if (viewer->isVisible())
    {
        statusBar.setCurrentViewerName (viewer->getViewerName());
    }
    environmentView.setKernel (viewer->getKernel());
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
    auto environmentViewArea = directoryTreeArea.removeFromBottom (300);

    setBounds (statusBar, statusBarArea);
    setBounds (directoryTree, directoryTreeArea);
    setBounds (environmentView, environmentViewArea);

    for (auto view : viewers.getAllComponents())
        setBounds (*view, area);
}
