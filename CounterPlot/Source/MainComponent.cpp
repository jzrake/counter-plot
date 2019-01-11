#include "MainComponent.hpp"
#include "Views/LookAndFeel.hpp"
#include "Views/JetInCloudView.hpp"
#include "Views/BinaryTorquesView.hpp"
#include "Views/FileBasedView.hpp"
#include "Views/ColourMapView.hpp"
#include "Views/UserExtensionView.hpp"




//=============================================================================
class DefaultView : public FileBasedView
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




//=============================================================================
void StatusBar::paint (Graphics& g)
{
    auto area = getLocalBounds();
    auto indicatorArea       = area.removeFromRight (getHeight());
    auto viewerNameArea      = area.removeFromRight (200);
    auto mousePositionArea   = area.removeFromRight (200);
    auto backgroundColour    = findColour (LookAndFeelHelpers::statusBarBackground);
    auto fontColour          = findColour (LookAndFeelHelpers::statusBarText);
    auto busyIndicatorColour = numberOfAsyncTasks ? Colours::yellow : Colours::transparentBlack;

    g.setColour (backgroundColour);
    g.fillAll();

    g.setColour (busyIndicatorColour);
    g.fillEllipse (indicatorArea.reduced (5).toFloat());

    g.setColour (fontColour);
    g.setFont (Font ("Monaco", 11, 0));

    if (! mousePositionInFigure.isOrigin())
    {
        g.drawText (mousePositionInFigure.toString(), mousePositionArea, Justification::centredLeft);
    }
    g.drawText (currentViewerName, viewerNameArea, Justification::centredRight);
}

void StatusBar::resized()
{
}




//=============================================================================
MainComponent::MainComponent()
{
    directoryTree.addListener (this);

    addAndMakeVisible (statusBar);
    addAndMakeVisible (directoryTree);

    views.add (new JsonFileViewer);
    views.add (new ImageFileViewer);
    views.add (BinaryTorques::create());
    views.add (JetInCloud::create());
    views.add (new ColourMapView);
    // views.add (new UserExtensionView);
    views.add (new DefaultView);

    for (const auto& view : views)
    {
        addChildComponent (*view);
    }

    views.getLast()->setVisible (true);
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
    bool found = false;
    File file = currentFile;

    for (const auto& view : views)
    {
        if (! found && view->isInterestedInFile (file))
        {
            found = true;
            view->setVisible (true);
            view->loadFile (file);
            statusBar.setCurrentViewerName (view->getViewerName());
        }
        else
        {
            view->setVisible (false);
        }
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
//    if (key == KeyPress::returnKey)
//    {
//        auto view = dynamic_cast<UserExtensionView*>(views.getLast());
//        view->configure (File ("/Users/jzrake/Work/CounterPlot/view.yaml"));
//        view->setVisible (true);
//        statusBar.setCurrentViewerName (view->getViewerName());
//        return true;
//    }
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
void MainComponent::fileBasedViewAsyncTaskStarted()
{
    statusBar.incrementAsyncTaskCount();
}

void MainComponent::fileBasedViewAsyncTaskFinished()
{
    statusBar.decrementAsyncTaskCount();
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
    setBounds (statusBar, area.removeFromBottom (22));

    if (directoryTreeShowing)
        setBounds (directoryTree, area.removeFromLeft (300));
    else
        setBounds (directoryTree, area.withWidth (300).translated (-300, 0));

    for (const auto& view : views)
        setBounds (*view, area);
}
