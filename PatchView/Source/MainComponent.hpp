#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"
#include "Views/DirectoryTree.hpp"
#include "Views/VariantView.hpp"



class PatchesQuadMeshArtist;



//=============================================================================
class PatchesView : public Component, public FigureView::Listener
{
public:

    //=========================================================================
    PatchesView();
    void setDocumentFile (File viewedDocument);
    void nextColorMap();
    void prevColorMap();
    void setColorMap (int index);

    //=========================================================================
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    void figureViewSetMargin (FigureView*, const BorderSize<int>&) override;
    void figureViewSetDomain (FigureView*, const Rectangle<double>&) override;
    void figureViewSetXlabel (FigureView*, const String&) override;
    void figureViewSetYlabel (FigureView*, const String&) override;
    void figureViewSetTitle (FigureView*, const String&) override;

private:
    void mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void reloadFigures();
    Array<Colour> getColorMap() const;

    int colorMapIndex = 0;
    std::array<float, 2> scalarExtent;
    Grid layout;
    OwnedArray<FigureView> figures;
    std::shared_ptr<PatchesQuadMeshArtist> artist;
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
    ImageComponent imageView;
    VariantView variantView;
    PatchesView patchesView;

    DirectoryTree directoryTree;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
