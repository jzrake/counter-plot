#include "MainComponent.hpp"
#include "MetalSurface.hpp"
#include "Views/LookAndFeel.hpp"




//=============================================================================
MainComponent::DataLoadingThread::DataLoadingThread (MainComponent& main)
: Thread ("dataLoadingThread")
, main (main)
{
}

void MainComponent::DataLoadingThread::loadFileAsync (File fileToLoad)
{
    main.dataLoadingThreadWaiting();
    stopThread (-1);
    main.dataLoadingThreadRunning();

    file = fileToLoad;
    startThread();
}

void MainComponent::DataLoadingThread::run()
{
    main.binaryTorquesView.setDocumentFile (file, [this] { return threadShouldExit(); });

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
void StatusBar::setBusyIndicatorStatus (BusyIndicatorStatus newStatus)
{
    status = newStatus;
    repaint();
}

void StatusBar::paint (Graphics& g)
{
    g.setColour (findColour (LookAndFeelHelpers::statusBarBackground));
    g.fillAll();

    auto box = getLocalBounds().removeFromRight (getHeight()).reduced (4);
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
MainComponent::MainComponent() : dataLoadingThread (*this)
{
    directoryTree.setDirectoryToShow (File::getSpecialLocation (File::userHomeDirectory));
    directoryTree.addListener (this);

    addAndMakeVisible (statusBar);
    addAndMakeVisible (directoryTree);
    addChildComponent (imageView);
    addChildComponent (variantView);
    addAndMakeVisible (jetInCloudView);
    addChildComponent (binaryTorquesView);

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

    imageView        .setBounds (area);
    variantView      .setBounds (area);
    jetInCloudView   .setBounds (area);
    binaryTorquesView.setBounds (area);
}

bool MainComponent::keyPressed (const juce::KeyPress &key)
{
    return false;
}




//=============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    try
    {
        if (FileSystemSerializer::looksLikeDatabase (file))
        {
            jetInCloudView.setDocumentFile (file);

            imageView        .setVisible (false);
            variantView      .setVisible (false);
            jetInCloudView   .setVisible (true);
            binaryTorquesView.setVisible (false);
        }
        else if (auto format = ImageFileFormat::findImageFormatForFileExtension (file))
        {
            imageView.setImage (format->loadFrom (file));

            imageView        .setVisible (true);
            variantView      .setVisible (false);
            jetInCloudView   .setVisible (false);
            binaryTorquesView.setVisible (false);
        }
        else if (file.hasFileExtension (".json"))
        {
            variantView.setData (JSON::parse (file));

            imageView        .setVisible (false);
            variantView      .setVisible (true);
            jetInCloudView   .setVisible (false);
            binaryTorquesView.setVisible (false);
        }
        else if (file.hasFileExtension (".h5"))
        {
            dataLoadingThread.loadFileAsync (file);

            imageView        .setVisible (false);
            variantView      .setVisible (false);
            jetInCloudView   .setVisible (false);
            binaryTorquesView.setVisible (true);
        }
        else if (ColourMapHelpers::looksLikeRGBTable (file))
        {
            //        auto cb = ScalarMapping();
            //        cb.stops = ColourmapHelpers::coloursFromRGBTable (file.loadFileAsString());
            //        model.content.clear();
            //        model.content.push_back (std::make_shared<ColourGradientArtist> (cb));
            //        figure.setModel (model);
            //
            //        figure.setVisible (true);
            //        imageView.setVisible (false);
            //        variantView.setVisible (false);
            //        jetInCloudView.setVisible (false);
        }
    }
    catch (const std::exception& e)
    {
        DBG("failed to open: " << e.what());
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
