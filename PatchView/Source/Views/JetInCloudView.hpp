#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"




//=============================================================================
class JetInCloudView : public Component, public FigureView::Listener
{
public:

    //=========================================================================
    JetInCloudView();
    ~JetInCloudView();
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
	class QuadmeshArtist;
    void mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void reloadFigures();
    Array<Colour> getColorMap() const;

    int colorMapIndex = 0;
    std::array<float, 2> scalarExtent;
    Grid layout;
    OwnedArray<FigureView> figures;
    std::shared_ptr<QuadmeshArtist> artist;
};
