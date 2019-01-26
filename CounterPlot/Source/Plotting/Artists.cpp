#include "Artists.hpp"




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

    if (model.lineStyle != LineStyle::none)
    {
        Path p;
        p.startNewSubPath (trans.fromDomainX (model.x (0)),
                           trans.fromDomainY (model.y (0)));

        for (int n = 1; n < model.x.size(); ++n)
        {
            p.lineTo (trans.fromDomainX (model.x(n)),
                      trans.fromDomainY (model.y(n)));
        }

        auto stroke = PathStrokeType (model.lineWidth);
        g.setColour (model.lineColour);

        switch (model.lineStyle)
        {
            case LineStyle::none: break;
            case LineStyle::solid:
            {
                g.strokePath (p, stroke);
                break;
            }
            case LineStyle::dash:
            {
                static const float dashLengths[] = {8.f, 8.f};
                stroke.createDashedStroke (p, p, dashLengths, 2);
                g.strokePath (p, stroke);
                break;
            }
            case LineStyle::dashdot:
            {
                static const float dashLengths[] = {8.f, 8.f, 2.f, 8.f};
                stroke.createDashedStroke (p, p, dashLengths, 4);
                g.strokePath (p, stroke);
                break;
            }
        }
    }
    if (model.markerStyle != MarkerStyle::none)
    {
        const auto ms = model.markerSize;

        for (int n = 0; n < model.x.size(); ++n)
        {
            const float X = trans.fromDomainX (model.x(n));
            const float Y = trans.fromDomainY (model.y(n));
            const auto glyphArea = Rectangle<float> (X - 0.5f * ms, Y - 0.5f * ms, ms, ms);

            switch (model.markerStyle)
            {
                case MarkerStyle::none: break;
                case MarkerStyle::circle:
                    g.setColour (model.markerFillColour);
                    g.fillEllipse (glyphArea);
                    g.setColour (model.markerEdgeColour);
                    g.drawEllipse (glyphArea, model.markerEdgeWidth);
                    break;
                case MarkerStyle::square:
                    g.setColour (model.markerFillColour);
                    g.fillRect (glyphArea);
                    g.setColour (model.markerEdgeColour);
                    g.drawRect (glyphArea, model.markerEdgeWidth);
                    break;
                case MarkerStyle::plus:
                    g.setColour (model.markerEdgeColour);
                    g.fillRect (glyphArea.reduced (0.f, 0.5f * (ms - model.markerEdgeWidth)));
                    g.fillRect (glyphArea.reduced (0.5f * (ms - model.markerEdgeWidth), 0.f));
                    break;
                case MarkerStyle::cross:
                    g.setColour (model.markerEdgeColour);
                    g.drawLine (Line<float>(glyphArea.getTopLeft(), glyphArea.getBottomRight()), model.markerEdgeWidth);
                    g.drawLine (Line<float>(glyphArea.getTopRight(), glyphArea.getBottomLeft()), model.markerEdgeWidth);
                    break;
                case MarkerStyle::diamond:
                {
                    Path p;
                    p.addQuadrilateral (glyphArea.getCentreX(), glyphArea.getY(),
                                        glyphArea.getRight(), glyphArea.getCentreY(),
                                        glyphArea.getCentreX(), glyphArea.getBottom(),
                                        glyphArea.getX(), glyphArea.getCentreY());
                    g.setColour (model.markerEdgeColour);
                    g.strokePath (p, PathStrokeType (model.markerEdgeWidth));
                    g.setColour (model.markerFillColour);
                    g.fillPath (p);
                    break;
                }
            }
        }
    }
}




//=============================================================================
ColourGradientArtist::ColourGradientArtist()
{
}

ColourGradientArtist::ColourGradientArtist (const Array<Colour>& stops) : stops (stops)
{
}

void ColourGradientArtist::setStops (const Array<Colour>& newStopsToUse)
{
    stops = newStopsToUse;
}

void ColourGradientArtist::setOrientation (Orientation orientationToUse)
{
    orientation = orientationToUse;
}

void ColourGradientArtist::setOrientation (const String& name, bool throwIfNotFound)
{
    if (false) {}
    else if (name == "vertical")   orientation = Orientation::vertical;
    else if (name == "horizontal") orientation = Orientation::horizontal;
    else if (throwIfNotFound) throw std::invalid_argument ("orientation must be vertical or horizontal");
}

void ColourGradientArtist::setGradientFollowsTransform (bool shouldGradientBeTransformed)
{
    transformGradient = shouldGradientBeTransformed;
}




//=============================================================================
void ColourGradientArtist::paint (Graphics& g, const PlotTransformer& trans)
{
    auto gradient = ColourGradient();
    auto n = 0;
    auto ds = 1.0 / (stops.size() - 1);

    for (auto c : stops)
    {
        gradient.addColour (n++ * ds, c);
    }

    switch (orientation)
    {
        case Orientation::horizontal:
            if (transformGradient)
            {
                gradient.point1 = {float (trans.fromDomainX (0.f)), 0.f};
                gradient.point2 = {float (trans.fromDomainX (1.f)), 0.f};
            }
            else
            {
                gradient.point1 = trans.getRange().getBottomLeft().toFloat();
                gradient.point2 = trans.getRange().getBottomRight().toFloat();
            }
            break;

        case Orientation::vertical:
            if (transformGradient)
            {
                gradient.point1 = {0.f, float (trans.fromDomainY (0.f))};
                gradient.point2 = {0.f, float (trans.fromDomainY (1.f))};
            }
            else
            {
                gradient.point1 = trans.getRange().getBottomLeft().toFloat();
                gradient.point2 = trans.getRange().getTopLeft().toFloat();
            }
            break;
    }
    g.setGradientFill (gradient);
    g.fillAll();
}




//=============================================================================
TriangleMeshArtist::TriangleMeshArtist (DeviceBufferFloat2 vertices,
                                        DeviceBufferFloat1 scalars,
                                        ScalarMapping mapping)
: vertices (vertices)
, scalars (scalars)
, mapping (mapping)
{
}

void TriangleMeshArtist::render (RenderingSurface& surface)
{
    surface.renderTriangles (vertices, scalars, mapping);
}
