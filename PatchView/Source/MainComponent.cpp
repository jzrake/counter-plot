#include "MainComponent.hpp"
#include "Views/LookAndFeel.hpp"
#include "Views/VariantView.hpp"
#include "Views/JetInCloudView.hpp"
#include "Views/BinaryTorquesView.hpp"
#include "Views/FileBasedView.hpp"




//=============================================================================
void StatusBar::setBusyIndicatorStatus (BusyIndicatorStatus newStatus)
{
    status = newStatus;
    repaint();
}

void StatusBar::paint (Graphics& g)
{
    g.setColour (findColour (LookAndFeelHelpers::statusBarBackground));
    g.fillAll();

    auto box = getLocalBounds().removeFromRight (getHeight()).reduced (5);
    auto colour = Colour();

    switch (status)
    {
        case BusyIndicatorStatus::idle   : colour = Colours::transparentBlack; break;
        case BusyIndicatorStatus::running: colour = Colours::yellow; break;
        case BusyIndicatorStatus::waiting: colour = Colours::red; break;
    }

    g.setColour (colour);
    g.fillEllipse (box.toFloat());
}

void StatusBar::resized()
{
}




//=============================================================================
class JsonFileViewer : public FileBasedView
{
public:
    JsonFileViewer()
    {
        addAndMakeVisible (view);
    }

    bool isInterestedInFile (File file) const override
    {
        return file.hasFileExtension (".json");
    }

    bool loadFile (File fileToDisplay) override
    {
        view.setData (JSON::parse (fileToDisplay));
        return true;
    }

    void resized() override
    {
        view.setBounds (getLocalBounds());
    }
private:
    VariantView view;
};




//=============================================================================
class ImageFileViewer : public FileBasedView
{
public:
    ImageFileViewer()
    {
        addAndMakeVisible (view);
    }

    bool isInterestedInFile (File file) const override
    {
        return ImageFileFormat::findImageFormatForFileExtension (file);
    }

    bool loadFile (File file) override
    {
        if (auto format = ImageFileFormat::findImageFormatForFileExtension (file))
        {
            view.setImage (format->loadFrom (file));
        }
        return true;
    }

    void resized() override
    {
        view.setBounds (getLocalBounds());
    }
private:
    ImageComponent view;
};




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

    for (const auto& view : views)
    {
        addChildComponent (*view);
    }
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

void MainComponent::paint (Graphics& g)
{
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    statusBar.setBounds (area.removeFromBottom (22));
    directoryTree.setBounds (area.removeFromLeft(300));

    for (const auto& view : views)
    {
        view->setBounds (area);
    }
}

bool MainComponent::keyPressed (const juce::KeyPress &key)
{
    return false;
}




//=============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    bool found = false;

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
