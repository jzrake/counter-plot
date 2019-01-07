#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"




//=============================================================================
class BinaryTorquesView : public Component, public FigureView::Listener
{
public:

    //=========================================================================
    BinaryTorquesView();
    ~BinaryTorquesView();
    void setDocumentFile (File viewedDocument, std::function<bool()> bailout);

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
	class QuadmeshArtist;
    void mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void reloadFigures();

    std::array<float, 2> scalarExtent;
    Grid layout;
    OwnedArray<FigureView> figures;
    std::shared_ptr<QuadmeshArtist> artist;
    ColourMapCollection cmaps;
};
