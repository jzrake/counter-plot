#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"
#include "DirectoryTree.hpp"




//==============================================================================
class MainComponent : public Component, public FigureView::Listener
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
    void figureViewSetMargin (FigureView* figure, const BorderSize<int>& value) override;
    void figureViewSetDomain (FigureView* figure, const Rectangle<double>& value) override;
    void figureViewSetXlabel (FigureView* figure, const String& value) override;
    void figureViewSetYlabel (FigureView* figure, const String& value) override;
    void figureViewSetTitle (FigureView* figure, const String& value) override;

private:
    //==============================================================================
    FigureView figure;
    FigureModel model;
    DirectoryTree directoryTree;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
