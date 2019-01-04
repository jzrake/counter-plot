#include "FigureView.hpp"




//=============================================================================
static std::vector<Rectangle<float>> makeRectanglesInColumn (const Rectangle<int>& column,
                                                             const std::vector<float>& midpoints,
                                                             float height)
{
    std::vector<Rectangle<float>> rectangles;

    for (auto y : midpoints)
    {
        rectangles.push_back (Rectangle<float> (column.getX(), y - height * 0.5f, column.getWidth(), height));
    }
    return rectangles;
}

static std::vector<Rectangle<float>> makeRectanglesInRow (const Rectangle<int>& row,
                                                          const std::vector<float>& midpoints,
                                                          float width)
{
    std::vector<Rectangle<float>> rectangles;

    for (auto x : midpoints)
    {
        rectangles.push_back (Rectangle<float> (x - width * 0.5f, row.getY(), width, row.getHeight()));
    }
    return rectangles;
}




// ============================================================================
class Ticker
{
public:
    struct Tick
    {
        double value = 0.0;      /**< normalized data coordinate (0, 1) relative to limits */
        float pixel  = 0.f;      /**< position in pixels on axes content */
        std::string label;
    };
    static std::vector<Tick> createTicks (double l0, double l1, int p0, int p1);
    static std::vector<Tick> formatTicks (const std::vector<double>& locations, double l0, double l1, int p0, int p1);
    static std::vector<double> locateTicksLog (double l0, double l1);
    static std::vector<double> locateTicks (double l0, double l1);
    static std::vector<float> getPixelLocations (const std::vector<Tick>& ticks);
};




//=============================================================================
std::vector<Ticker::Tick> Ticker::createTicks (double l0, double l1, int p0, int p1)
{
    return formatTicks (locateTicks (l0, l1), l0, l1, p0, p1);
}

std::vector<Ticker::Tick> Ticker::formatTicks (const std::vector<double>& locations,
                                               double l0, double l1, int p0, int p1)
{
    auto ticks = std::vector<Tick>();
    char buffer[256];

    for (auto x : locations)
    {
        std::snprintf (buffer, 256, "%.8lf", x);

        Tick t;
        t.value = (x - l0) / (l1 - l0);
        t.pixel = p0 + t.value * (p1 - p0);
        t.label = buffer;

        while (t.label.back() == '0')
        {
            t.label.pop_back();
        }

        if (t.label.back() == '.')
        {
            t.label.push_back ('0');
        }
        ticks.push_back (t);
    }
    return ticks;
}

std::vector<double> Ticker::locateTicksLog (double l0, double l1)
{
    auto N0 = 8;
    auto x0 = std::floor (std::min (l0, l1));
    auto x1 = std::ceil  (std::max (l0, l1));
    auto decskip = 1 + (x1 - x0) / N0;

    auto loc = std::vector<double>();

    for (int n = x0; n < x1; n += decskip)
    {
        loc.push_back (n);
    }
    return loc;
}

std::vector<double> Ticker::locateTicks (double l0, double l1)
{
    auto N0 = 10;
    auto x0 = std::min (l0, l1);
    auto x1 = std::max (l0, l1);
    auto dx = std::pow (10, -1 + std::floor (std::log10 (x1 - x0) + 1e-8));
    auto Nx = (x1 - x0) / dx;

    while (Nx <= N0) { Nx *= 2; dx /= 2; }
    while (Nx >  N0) { Nx /= 2; dx *= 2; }

    auto start = int (x0 / dx) * dx;
    auto loc = std::vector<double>();

    for (int n = 0; n <= Nx + 1; ++n)
    {
        auto x = start + n * dx;

        if (x0 + 0.1 * dx <= x && x <= x1 - 0.1 * dx)
        {
            loc.push_back (x);
        }
    }
    return loc;
}

std::vector<float> Ticker::getPixelLocations (const std::vector<Tick>& ticks)
{
    std::vector<float> pixels;
    std::transform (ticks.begin(), ticks.end(), std::back_inserter (pixels), [] (const auto& t) { return t.pixel; });
    return pixels;
}




//=============================================================================
LinePlotArtist::LinePlotArtist (LinePlotModel model) : model (model)
{
}

void LinePlotArtist::paint (Graphics& g, const PlotTransformer& trans)
{
    jassert (model.x.size() == model.y.size());
    
    if (model.x.empty())
    {
        return;
    }
    
    Path p;
    p.startNewSubPath (trans.fromDomainX (model.x(0)),
                       trans.fromDomainY (model.y(0)));
    
    for (int n = 1; n < model.x.size(); ++n)
    {
        p.lineTo (trans.fromDomainX (model.x(n)),
                  trans.fromDomainY (model.y(n)));
    }
    g.setColour (model.lineColour);
    g.strokePath (p, PathStrokeType (model.lineWidth));
}




//=============================================================================
ColourGradientArtist::ColourGradientArtist (ScalarMapping model) : model (model)
{
}

void ColourGradientArtist::paint (Graphics& g, const PlotTransformer& trans)
{
    auto gradient = ColourGradient();
    auto n = 0;
    auto ds = 1.0 / (model.stops.size() - 1);
    auto range = trans.getRange();
    
    for (auto c : model.stops)
    {
        gradient.addColour (n++ * ds, c);
    }

    gradient.point1 = range.getBottomLeft().toFloat();
    gradient.point2 = range.getTopLeft().toFloat();

    g.setGradientFill (gradient);
    g.fillAll();
}




//=============================================================================
FigureView::PlotArea::PlotArea (FigureView& figure)
: figure (figure)
, resizer (this, &constrainer)
{
    constrainer.setMinimumOnscreenAmounts (0xffffff, 0xffffff, 0xffffff, 0xffffff);
    constrainer.setMinimumSize (100, 100);
    addChildComponent (resizer);
}

void FigureView::PlotArea::paint (Graphics& g)
{
    if (figure.paintMarginsAndBackground)
    {
        g.fillAll (figure.model.backgroundColour);
    }


    auto xticks = Ticker::createTicks (figure.model.xmin, figure.model.xmax, 0, getWidth());
    auto yticks = Ticker::createTicks (figure.model.ymin, figure.model.ymax, getHeight(), 0);


    // Draw gridlines
    // ========================================================================
    g.setColour (figure.model.gridlinesColour);
    for (const auto& tick : xticks) g.drawVerticalLine (tick.pixel, 0, getHeight());
    for (const auto& tick : yticks) g.drawHorizontalLine (tick.pixel, 0, getWidth());


    // Draw content
    // ========================================================================
    for (const auto& p : figure.model.content)
    {
        p->paint (g, *this);
    }
}

void FigureView::PlotArea::resized()
{
    resizer.setBounds (getLocalBounds());
    sendSetMarginIfNeeded();
}

void FigureView::PlotArea::mouseDown (const MouseEvent&)
{
    domainBeforePan = figure.model.getDomain();
}

void FigureView::PlotArea::mouseDrag (const MouseEvent& e)
{
    const auto D = Point<double> (getWidth(), getHeight());
    const auto m = Point<double> (e.getDistanceFromDragStartX(), -e.getDistanceFromDragStartY());
    const auto p = domainBeforePan.getTopLeft();
    const auto q = domainBeforePan.getBottomRight();
    const auto d = q - p;
    sendSetDomain (figure.model.getDomain().withPosition (p - d * m / D));
}

void FigureView::PlotArea::mouseMagnify (const MouseEvent& e, float scaleFactor)
{
    const double xlim[2] = {figure.model.xmin, figure.model.xmax};
    const double ylim[2] = {figure.model.ymin, figure.model.ymax};
    const double Dx = getWidth();
    const double Dy = getHeight();
    const double dx = xlim[1] - xlim[0];
    const double dy = ylim[1] - ylim[0];
    const double newdx = dx / scaleFactor;
    const double newdy = dy / scaleFactor;
    const double fixedx = toDomainX (e.position.x);
    const double fixedy = toDomainY (e.position.y);
    const double newx0 = e.mods.isAltDown()  ? xlim[0] : fixedx - newdx * (0 + e.position.x / Dx);
    const double newx1 = e.mods.isAltDown()  ? xlim[1] : fixedx + newdx * (1 - e.position.x / Dx);
    const double newy0 = e.mods.isCtrlDown() ? ylim[0] : fixedy - newdy * (1 - e.position.y / Dy);
    const double newy1 = e.mods.isCtrlDown() ? ylim[1] : fixedy + newdy * (0 + e.position.y / Dy);
    sendSetDomain (Rectangle<double>::leftTopRightBottom (newx0, newy0, newx1, newy1));
}




//=============================================================================
BorderSize<int> FigureView::PlotArea::computeMargin() const
{
    return {getY(), getX(), getParentHeight() - getBottom(), getParentWidth() - getRight()};
}

double FigureView::PlotArea::fromDomainX (double x) const
{
    return jmap (x, figure.model.xmin, figure.model.xmax, 0.0, double (getWidth()));
}

double FigureView::PlotArea::fromDomainY (double y) const
{
    return jmap (y, figure.model.ymin, figure.model.ymax, double (getHeight()), 0.0);
}

double FigureView::PlotArea::toDomainX (double x) const
{
    return jmap (x, 0.0, double (getWidth()), figure.model.xmin, figure.model.xmax);
}

double FigureView::PlotArea::toDomainY (double y) const
{
    return jmap (y, double (getHeight()), 0.0, figure.model.ymin, figure.model.ymax);
}

std::array<float, 4> FigureView::PlotArea::getDomain() const
{
    return {
        float(figure.model.xmin), float(figure.model.xmax),
        float(figure.model.ymin), float(figure.model.ymax)
    };
}

Rectangle<int> FigureView::PlotArea::getRange() const
{
    return getLocalBounds();
}

void FigureView::PlotArea::sendSetMarginIfNeeded()
{
    auto newMargin = computeMargin();

    if (newMargin != figure.model.margin)
    {
        figure.listeners.call (&Listener::figureViewSetMargin, &figure, newMargin);
    }
}

void FigureView::PlotArea::sendSetDomain (const Rectangle<double>& domain)
{
    figure.listeners.call (&Listener::figureViewSetDomain, &figure, domain);
}




//=============================================================================
FigureView::FigureView() : plotArea (*this)
{
    xlabel.setJustificationType (Justification::centred);
    ylabel.setJustificationType (Justification::centred);
    title .setJustificationType (Justification::centred);

    xlabel.setEditable (true);
    ylabel.setEditable (true);
    title .setEditable (true);
    xlabel.addListener (this);
    ylabel.addListener (this);
    title .addListener (this);
    xlabel.setFont (Font().withHeight (12));
    ylabel.setFont (Font().withHeight (12));
    title .setFont (Font().withHeight (16));

    xlabel.addMouseListener (this, false);
    ylabel.addMouseListener (this, false);
    title .addMouseListener (this, false);

    // So that we get popup menu clicks
    plotArea.addMouseListener (this, false);

    setModel (model);
    refreshModes();

    addAndMakeVisible (plotArea);
    addAndMakeVisible (title);
    addAndMakeVisible (xlabel);
    addAndMakeVisible (ylabel);
}

void FigureView::setRenderingSurface (std::unique_ptr<RenderingSurface> surfaceToRenderInto)
{
    if (surface)
    {
        removeChildComponent (surface.get());
    }
    surface = std::move (surfaceToRenderInto);

    if (surface)
    {
        addAndMakeVisible (surface.get());
        surface->setContent (model.content, plotArea);
    }
}

void FigureView::setModel (const FigureModel& newModel)
{
    model = newModel;

    if (surface)
    {
        surface->setContent (model.content, plotArea);
    }

    xlabel.setText (model.xlabel, NotificationType::dontSendNotification);
    ylabel.setText (model.ylabel, NotificationType::dontSendNotification);
    title .setText (model.title , NotificationType::dontSendNotification);
    layout();
    repaint();
}

void FigureView::addListener (Listener* listener)
{
    listeners.add (listener);
}

void FigureView::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

Rectangle<int> FigureView::getPlotAreaBounds() const
{
    return plotArea.getBounds();
}




//=============================================================================
void FigureView::paint (Graphics& g)
{
    if (paintMarginsAndBackground)
    {
        g.fillAll (model.marginColour);
    }
}

void FigureView::paintOverChildren (Graphics& g)
{
    auto geom = computeGeometry();


    // Compute tick geometry data
    // ========================================================================
    auto xticks          = Ticker::createTicks (model.xmin, model.xmax, plotArea.getX(), plotArea.getRight());
    auto yticks          = Ticker::createTicks (model.ymin, model.ymax, plotArea.getBottom(), plotArea.getY());
    auto xtickPixels     = Ticker::getPixelLocations (xticks);
    auto ytickPixels     = Ticker::getPixelLocations (yticks);
    auto xtickLabelBoxes = makeRectanglesInRow    (geom.xtickLabelAreaB, xtickPixels, model.tickLabelWidth);
    auto ytickLabelBoxes = makeRectanglesInColumn (geom.ytickLabelAreaL, ytickPixels, model.tickLabelHeight);
    auto xtickBoxes      = makeRectanglesInRow    (geom.xtickAreaB, xtickPixels, model.tickWidth);
    auto ytickBoxes      = makeRectanglesInColumn (geom.ytickAreaL, ytickPixels, model.tickWidth);


    // Extra geometry fills for debugging geometry
    // ========================================================================
    if (annotateGeometry)
    {
        g.setColour (Colours::blue.withAlpha (0.3f));
        g.fillRect (geom.xtickAreaB);
        g.fillRect (geom.ytickAreaL);

        g.setColour (Colours::red.withAlpha (0.3f));
        g.fillRect (geom.xtickLabelAreaB);
        g.fillRect (geom.ytickLabelAreaL);

        g.setColour (Colours::yellow.withAlpha (0.3f));
        g.fillRect (geom.marginT);
        g.fillRect (geom.marginB);
        g.fillRect (geom.marginL);
        g.fillRect (geom.marginR);

        g.setColour (Colours::purple.withAlpha (0.3f));
        for (auto box : ytickLabelBoxes) g.fillRect (box);
        for (auto box : xtickLabelBoxes) g.fillRect (box);
    }


    // Draw the ticks and labels
    // ========================================================================
    g.setColour (findColour (Label::textColourId));
    g.setFont (Font().withHeight (12));
    for (auto box : xtickBoxes) g.fillRect (box);
    for (auto box : ytickBoxes) g.fillRect (box);

    if (paintTickLabels)
    {
        for (int n = 0; n < xticks.size(); ++n) g.drawText (xticks[n].label, xtickLabelBoxes[n], Justification::centredTop);
        for (int n = 0; n < yticks.size(); ++n) g.drawText (yticks[n].label, ytickLabelBoxes[n], Justification::centredRight);
    }
}

void FigureView::resized()
{
    layout();
}

void FigureView::mouseEnter (const MouseEvent& e)
{
    if (e.originalComponent == &xlabel ||
        e.originalComponent == &ylabel ||
        e.originalComponent == &title)
        e.originalComponent->setColour (Label::backgroundColourId, model.marginColour.darker (0.1f));
}

void FigureView::mouseExit (const MouseEvent& e)
{
    if (e.originalComponent == &xlabel ||
        e.originalComponent == &ylabel ||
        e.originalComponent == &title)
        e.originalComponent->setColour (Label::backgroundColourId, Colours::transparentBlack);
}

void FigureView::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        mouseExit (e); // work-around unpaired mouse enter/exit with popup menu

        PopupMenu menu;
        menu.addItem (1, "Annotate geometry", true, annotateGeometry);
        menu.addItem (2, "Plot area resizer", true, allowPlotAreaResize);
        menu.addItem (3, "Draw axis labels", true, paintAxisLabels);
        menu.addItem (4, "Draw tick labels", true, paintTickLabels);
        menu.addItem (5, "Fill backgrounds", true, paintMarginsAndBackground);

        menu.showMenuAsync (PopupMenu::Options(), [this] (int code)
        {
            switch (code)
            {
                case 1: annotateGeometry = ! annotateGeometry; repaint(); break;
                case 2: allowPlotAreaResize = ! allowPlotAreaResize; refreshModes(); break;
                case 3: paintAxisLabels = ! paintAxisLabels; refreshModes(); break;
                case 4: paintTickLabels = ! paintTickLabels; refreshModes(); break;
                case 5: paintMarginsAndBackground = ! paintMarginsAndBackground; refreshModes(); break;
                default: break;
            }
        });
    }
}




//=============================================================================
void FigureView::layout()
{
    auto g = computeGeometry();

    AffineTransform ylabelRot = AffineTransform::rotation (-M_PI_2, g.marginL.getCentreX(), g.marginL.getCentreY());
    ylabel.setTransform (ylabelRot);

    plotArea.setBounds (model.margin.subtractedFrom (getLocalBounds()));
    xlabel.setBounds (g.marginB);
    ylabel.setBounds (g.marginL.transformedBy (ylabelRot.inverted()));
    title.setBounds (g.marginT);

    if (surface)
    {
        surface->setBounds (model.margin.subtractedFrom (getLocalBounds()));
    }
}

void FigureView::refreshModes()
{
    plotArea.resizer.setVisible (allowPlotAreaResize);
    xlabel.setVisible (paintAxisLabels);
    ylabel.setVisible (paintAxisLabels);
    title .setVisible (paintAxisLabels);
    repaint();
}

PlotGeometry FigureView::computeGeometry() const
{
    return PlotGeometry::compute (getLocalBounds(), model.margin,
                                  model.tickLabelWidth, model.tickLabelHeight,
                                  model.tickLabelPadding, model.tickLength);
}

void FigureView::labelTextChanged (Label* label)
{
    if (label == &xlabel) listeners.call (&Listener::figureViewSetXlabel, this, xlabel.getText());
    if (label == &ylabel) listeners.call (&Listener::figureViewSetYlabel, this, ylabel.getText());
    if (label == &title ) listeners.call (&Listener::figureViewSetTitle,  this, title .getText());
}
