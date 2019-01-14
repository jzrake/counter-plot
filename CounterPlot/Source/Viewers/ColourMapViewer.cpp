#include "ColourMapViewer.hpp"




//=============================================================================
ColourMapViewer::ColourMapViewer()
{
    layout.templateRows    = { Grid::TrackInfo (4_fr), Grid::TrackInfo (3_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr) };
    layout.items.add (lineFigure.getGridItem());
    layout.items.add (cmapFigure.getGridItem());

    lineModel.titleShowing = true;
    lineModel.xlabelShowing = false;
    lineModel.ylabelShowing = false;
    lineModel.canEditMargin = true;
    lineModel.canEditXlabel = false;
    lineModel.canEditYlabel = false;
    lineModel.canEditTitle = false;
    lineModel.margin.setBottom (40);
    lineModel.xtickCount = 10;
    lineModel.ytickCount = 5;
    lineModel.xmin = -0.1f;
    lineModel.xmax = +1.1f;
    lineModel.ymin = -0.1f;
    lineModel.ymax = +1.1f;
    lineModel.backgroundColour = Colours::lightblue;
    lineModel.gridlinesColour = Colours::lightblue.darker();

    cmapModel.titleShowing = false;
    cmapModel.xlabelShowing = false;
    cmapModel.ylabelShowing = false;
    cmapModel.canEditMargin = false;
    cmapModel.canEditXlabel = false;
    cmapModel.canEditYlabel = false;
    cmapModel.canEditTitle = false;
    cmapModel.margin.setTop (0);
    cmapModel.xtickCount = 10;
    cmapModel.ytickCount = 0;
    cmapModel.xmin = -0.1f;
    cmapModel.xmax = +1.1f;
    cmapModel.ymin = -0.1f;
    cmapModel.ymax = +1.1f;
    cmapModel.gridlinesColour = Colours::transparentBlack;

    lineFigure.addListener (this);
    cmapFigure.addListener (this);

    updateFigures();
    addAndMakeVisible (lineFigure);
    addAndMakeVisible (cmapFigure);
}

bool ColourMapViewer::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".cmap");
}

void ColourMapViewer::loadFile (File fileToDisplay)
{
    lineModel.title = fileToDisplay.getFileName();
    mapping.stops = ColourMapHelpers::coloursFromRGBTable (fileToDisplay.loadFileAsString());
    updateFigures();
}

void ColourMapViewer::resized()
{
    layout.performLayout (getLocalBounds());
}

void ColourMapViewer::updateFigures()
{
    if (! mapping.stops.isEmpty())
    {
        auto lineR = LinePlotModel();
        auto lineG = LinePlotModel();
        auto lineB = LinePlotModel();
        lineR.lineColour = Colours::red;
        lineG.lineColour = Colours::green;
        lineB.lineColour = Colours::blue;
        lineR.lineWidth = 2.f;
        lineG.lineWidth = 2.f;
        lineB.lineWidth = 2.f;
        lineR.x.become (nd::linspace (0.0, 1.0, mapping.stops.size()));
        lineG.x.become (nd::linspace (0.0, 1.0, mapping.stops.size()));
        lineB.x.become (nd::linspace (0.0, 1.0, mapping.stops.size()));
        lineR.y.become (smooth (ColourMapHelpers::extractChannelAsDouble (mapping.stops, 'r')));
        lineG.y.become (smooth (ColourMapHelpers::extractChannelAsDouble (mapping.stops, 'g')));
        lineB.y.become (smooth (ColourMapHelpers::extractChannelAsDouble (mapping.stops, 'b')));

        auto artist = std::make_shared<ColourGradientArtist> (mapping);
        artist->setOrientation (ColourGradientArtist::Orientation::horizontal);
        artist->setGradientFollowsTransform (true);

        cmapModel.content = {artist};
        lineModel.content = {
            std::make_shared<LinePlotArtist> (lineR),
            std::make_shared<LinePlotArtist> (lineG),
            std::make_shared<LinePlotArtist> (lineB),
        };
    }
    cmapFigure.setModel (cmapModel);
    lineFigure.setModel (lineModel);
}




//=============================================================================
void ColourMapViewer::figureViewSetMargin (FigureView* figure, const BorderSize<int>& margin)
{
    if (figure == &cmapFigure)
    {
        cmapModel.margin = margin;
        lineModel.margin.setLeft (margin.getLeft());
        lineModel.margin.setRight (margin.getRight());
        updateFigures();
        return;
    }
    if (figure == &lineFigure)
    {
        lineModel.margin = margin;
        cmapModel.margin.setLeft (margin.getLeft());
        cmapModel.margin.setRight (margin.getRight());
        updateFigures();
        return;
    }
}

void ColourMapViewer::figureViewSetDomain (FigureView*, const Rectangle<double>& domain)
{
    cmapModel.setDomain (domain);
    lineModel.setDomain (domain);
    updateFigures();
}

void ColourMapViewer::figureViewSetXlabel (FigureView* figure, const String& xlabel)
{
}

void ColourMapViewer::figureViewSetYlabel (FigureView* figure, const String& ylabel)
{
}

void ColourMapViewer::figureViewSetTitle (FigureView* figure, const String& title)
{
}




//=============================================================================
//Array<double> ColourMapViewer::linspace (double x0, double x1, int num)
//{
//    Array<double> res;
//    res.ensureStorageAllocated (num);
//
//    for (int n = 0; n < num; ++n)
//    {
//        res.add (x0 + (x1 - x0) * n / (num - 1));
//    }
//    return res;
//}

nd::array<double, 1> ColourMapViewer::smooth (const nd::array<double, 1>& x)
{
    auto res = x.copy();

    for (int n = 2; n < x.size() - 2; ++n)
    {
        const float a = res (n - 2);
        const float b = res (n - 1);
        const float c = res (n + 0);
        const float d = res (n + 1);
        const float e = res (n + 2);
        res (n) = (a + 2 * b + 3 * c + 2 * d + e) / 9;
    }
    return res;
}
