#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"
#include "DirectoryTree.hpp"




//==============================================================================
class MainComponent
: public Component
, public FigureView::Listener
, public DirectoryTree::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //==============================================================================
    void figureViewSetMargin (FigureView*, const BorderSize<int>&) override;
    void figureViewSetDomain (FigureView*, const Rectangle<double>&) override;
    void figureViewSetXlabel (FigureView*, const String&) override;
    void figureViewSetYlabel (FigureView*, const String&) override;
    void figureViewSetTitle (FigureView*, const String&) override;

    //==============================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

private:
    //==============================================================================
    FigureView figure;
    FigureModel model;
    DirectoryTree directoryTree;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
