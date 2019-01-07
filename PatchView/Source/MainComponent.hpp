#pragma once
#include "JuceHeader.h"
#include "Views/DirectoryTree.hpp"
#include "Views/VariantView.hpp"
#include "Views/JetInCloudView.hpp"
#include "Views/BinaryTorquesView.hpp"




//=============================================================================
class StatusBar : public Component
{
public:

    //=========================================================================
    enum class BusyIndicatorStatus
    {
        running, waiting, idle,
    };

    //=========================================================================
    void setBusyIndicatorStatus (BusyIndicatorStatus newStatus);
    void paint (Graphics& g) override;
    void resized() override;
private:
    BusyIndicatorStatus status = BusyIndicatorStatus::idle;
};




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
{
public:

    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);

    //=========================================================================
    void paint (Graphics&) override;
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

private:
    //=========================================================================
    void dataLoadingThreadWaiting();
    void dataLoadingThreadRunning();
    void dataLoadingThreadFinished();

    //=========================================================================
    class DataLoadingThread : public Thread
    {
    public:
        DataLoadingThread (MainComponent&);
        void loadFileAsync (File fileToLoad);
        void run() override;
        MainComponent& main;
        File file;
    };

    StatusBar statusBar;
    ImageComponent imageView;
    VariantView variantView;
    JetInCloudView jetInCloudView;
    BinaryTorquesView binaryTorquesView;
    DirectoryTree directoryTree;
    DataLoadingThread dataLoadingThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
