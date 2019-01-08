#include "ColourMapView.hpp"




//=============================================================================
ColourMapView::ColourMapView()
{
    layout.templateRows    = { Grid::TrackInfo (4_fr), Grid::TrackInfo (3_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr) };
    layout.items.add (lineFigure.getGridItem());
    layout.items.add (cmapFigure.getGridItem());

    lineModel.titleShowing = true;
    lineModel.xlabelShowing = false;
    lineModel.ylabelShowing = false;
    lineModel.margin.setBottom (40);
    lineModel.xtickCount = 10;
    lineModel.ytickCount = 5;
    lineModel.xmin = 0.f;
    lineModel.xmax = 1.f;
    lineModel.ymin = 0.f;
    lineModel.ymax = 1.f;

    cmapModel.titleShowing = false;
    cmapModel.xlabelShowing = false;
    cmapModel.ylabelShowing = false;
    cmapModel.margin.setTop (0);
    cmapModel.gridlinesColour = Colours::transparentBlack;
    cmapModel.xtickCount = 10;
    cmapModel.ytickCount = 0;
    cmapModel.allowUserResize = true;
    cmapModel.xmin = 0.f;
    cmapModel.xmax = 1.f;
    cmapModel.ymin = 0.f;
    cmapModel.ymax = 1.f;

    updateFigures();
    addAndMakeVisible (cmapFigure);
    addAndMakeVisible (lineFigure);
}

bool ColourMapView::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".cmap");
}

bool ColourMapView::loadFile (File fileToDisplay)
{
    lineModel.title = fileToDisplay.getFileName();
    mapping.stops = ColourMapHelpers::coloursFromRGBTable (fileToDisplay.loadFileAsString());
    updateFigures();
    return true;
}

void ColourMapView::resized()
{
    layout.performLayout (getLocalBounds());
}

void ColourMapView::updateFigures()
{
    if (! mapping.stops.isEmpty())
    {
        auto artist = std::make_shared<ColourGradientArtist> (mapping);
        artist->setOrientation (ColourGradientArtist::Orientation::horizontal);
        cmapModel.content = {artist};
    }
    cmapFigure.setModel (cmapModel);
    lineFigure.setModel (lineModel);
}
