#pragma once
#include "Viewer.hpp"
#include "../Plotting/FigureView.hpp"




//=============================================================================
class ColourMapViewer : public Viewer, public FigureView::Listener
{
public:
    ColourMapViewer();

    //=========================================================================
    bool isInterestedInFile (File file) const override;
    void loadFile (File fileToDisplay) override;
    String getViewerName() const override { return "Color Map Viewer"; }

    //=========================================================================
    void resized() override;

    //=========================================================================
    void figureViewSetMargin (FigureView*, const BorderSize<int>&) override;
    void figureViewSetDomain (FigureView*, const Rectangle<double>&) override;
    void figureViewSetXlabel (FigureView*, const String&) override;
    void figureViewSetYlabel (FigureView*, const String&) override;
    void figureViewSetTitle (FigureView*, const String&) override;

private:
    //=========================================================================
    void updateFigures();
    static nd::array<double, 1> smooth (const nd::array<double, 1>&);

    //=========================================================================
    ScalarMapping mapping;
    FigureModel lineModel;
    FigureModel cmapModel;
    FigureView lineFigure;
    FigureView cmapFigure;
    Grid layout;
};
