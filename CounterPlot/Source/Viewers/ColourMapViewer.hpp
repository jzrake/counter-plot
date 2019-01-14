#pragma once
#include "FileBasedView.hpp"
#include "../Plotting/FigureView.hpp"




//=============================================================================
class ColourMapViewer : public FileBasedView, public FigureView::Listener
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
    static Array<double> linspace (double x0, double x1, int num);
    static Array<double> smooth (const Array<double>&);

    //=========================================================================
    ScalarMapping mapping;
    FigureModel lineModel;
    FigureModel cmapModel;
    FigureView lineFigure;
    FigureView cmapFigure;
    Grid layout;
};
