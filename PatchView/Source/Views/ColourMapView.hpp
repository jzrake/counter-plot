#pragma once
#include "FileBasedView.hpp"
#include "FigureView.hpp"




//=============================================================================
class ColourMapView : public FileBasedView
{
public:
    ColourMapView();
    bool isInterestedInFile (File file) const override;
    bool loadFile (File fileToDisplay) override;
    void resized() override;
private:
    void updateFigures();
    FigureView lineFigure;
    FigureView cmapFigure;
    FigureModel lineModel;
    FigureModel cmapModel;
    ScalarMapping mapping;
    Grid layout;
};
