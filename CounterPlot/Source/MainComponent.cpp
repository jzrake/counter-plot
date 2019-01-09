#include "MainComponent.hpp"
#include "Views/LookAndFeel.hpp"
#include "Views/JetInCloudView.hpp"
#include "Views/BinaryTorquesView.hpp"
#include "Views/FileBasedView.hpp"
#include "Views/ColourMapView.hpp"




//=============================================================================
class DefaultView : public FileBasedView
{
public:
    bool isInterestedInFile (File) const override { return true; }
    bool loadFile (File) override { return true; }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (LookAndFeelHelpers::propertyViewBackground));
    }
};




//=============================================================================
void StatusBar::setBusyIndicatorStatus (BusyIndicatorStatus newStatus)
{
    status = newStatus;
    repaint();
}

void StatusBar::paint (Graphics& g)
{
    auto area = getLocalBounds();

    auto indicatorArea = area.removeFromRight (getHeight());
    auto mousePositionArea = area.removeFromRight (200);
    auto colour = Colour();

    switch (status)
    {
        case BusyIndicatorStatus::idle   : colour = Colours::transparentBlack; break;
        case BusyIndicatorStatus::running: colour = Colours::yellow; break;
        case BusyIndicatorStatus::waiting: colour = Colours::red; break;
    }

    g.setColour (findColour (LookAndFeelHelpers::statusBarBackground));
    g.fillAll();

    g.setColour (colour);
    g.fillEllipse (indicatorArea.reduced (5).toFloat());

    if (! mousePositionInFigure.isOrigin())
    {
        g.setColour (findColour (LookAndFeelHelpers::statusBarBackground).contrasting());
        g.setFont (Font ("Monaco", 11, 0));
        g.drawText (mousePositionInFigure.toString(), mousePositionArea, Justification::centredLeft);
    }
}

void StatusBar::setMousePositionInFigure (Point<double> position)
{
    mousePositionInFigure = position;
    repaint();
}

void StatusBar::resized()
{
}




//=============================================================================
MainComponent::DataLoadingThread::DataLoadingThread (MainComponent& main)
: Thread ("dataLoadingThread")
, main (main)
{
}

void MainComponent::DataLoadingThread::loadFileToView (File fileToLoad, FileBasedView* targetView)
{
    main.dataLoadingThreadWaiting();
    stopThread (-1);
    main.dataLoadingThreadRunning();

    file = fileToLoad;
    view = targetView;

    try {
        if (view->loadFile (file))
        {
            main.dataLoadingThreadFinished();
        }
        else
        {
            startThread();
        }
    }
    catch (std::exception& e)
    {
        DBG("failed to open: " << e.what());
    }
}

void MainComponent::DataLoadingThread::run()
{
    try {
        view->loadFileAsync (file, [this] { return threadShouldExit(); });
    }
    catch (const std::exception& e)
    {
        DBG("failed to open: " << e.what());
    }

    if (! threadShouldExit())
    {
        MessageManager::callAsync ([m = SafePointer<MainComponent> (&main)]
        {
            if (m.getComponent())
                m.getComponent()->dataLoadingThreadFinished();
        });
    }
}




//=============================================================================
MainComponent::MainComponent() : dataLoadingThread (*this)
{
    directoryTree.setDirectoryToShow (File::getSpecialLocation (File::userHomeDirectory));
    directoryTree.addListener (this);

    addAndMakeVisible (statusBar);
    addAndMakeVisible (directoryTree);

    views.add (new JsonFileViewer);
    views.add (new ImageFileViewer);
    views.add (new JetInCloudView);
    views.add (new BinaryTorquesView);
    views.add (new ColourMapView);
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
    dataLoadingThread.stopThread (-1);
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
            dataLoadingThread.loadFileToView (file, view);
        }
        else
        {
            view->setVisible (false);
        }
    }
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
void MainComponent::dataLoadingThreadWaiting()
{
    statusBar.setBusyIndicatorStatus (StatusBar::BusyIndicatorStatus::waiting);
}

void MainComponent::dataLoadingThreadRunning()
{
    statusBar.setBusyIndicatorStatus (StatusBar::BusyIndicatorStatus::running);
}

void MainComponent::dataLoadingThreadFinished()
{
    statusBar.setBusyIndicatorStatus (StatusBar::BusyIndicatorStatus::idle);
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
