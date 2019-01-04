#include "PlotModels.hpp"




//=============================================================================
PlotGeometry PlotGeometry::compute (Rectangle<int> area, BorderSize<int> margin,
                                    float tickLabelWidth, float tickLabelHeight,
                                    float tickLabelPadding, float tickLength)
{
    PlotGeometry g;
    g.marginT         = topMargin    (area, margin);
    g.marginB         = bottomMargin (area, margin);
    g.marginL         = leftMargin   (area, margin);
    g.marginR         = rightMargin  (area, margin);
    g.ytickAreaL      = g.marginL.removeFromRight   (tickLength);
    g.ytickAreaR      = g.marginR.removeFromLeft    (tickLength);
    g.xtickAreaT      = g.marginT.removeFromBottom  (tickLength);
    g.xtickAreaB      = g.marginB.removeFromTop     (tickLength);
    g.ytickLabelAreaL = g.marginL.removeFromRight   (tickLabelWidth).withTrimmedRight   (tickLabelPadding);
    g.ytickLabelAreaR = g.marginR.removeFromLeft    (tickLabelWidth).withTrimmedLeft    (tickLabelPadding);
    g.xtickLabelAreaT = g.marginT.removeFromBottom  (tickLabelHeight).withTrimmedTop    (tickLabelPadding);
    g.xtickLabelAreaB = g.marginB.removeFromTop     (tickLabelHeight).withTrimmedBottom (tickLabelPadding);
    return g;
}

Rectangle<int> PlotGeometry::topMargin (Rectangle<int> area, BorderSize<int> margin)
{
    return {
        margin.getLeft(),
        0,
        area.getRight() - margin.getLeftAndRight(),
        margin.getTop()
    };
}

Rectangle<int> PlotGeometry::bottomMargin (Rectangle<int> area, BorderSize<int> margin)
{
    return {
        margin.getLeft(),
        area.getBottom() - margin.getBottom(),
        area.getRight() - margin.getLeftAndRight(),
        margin.getBottom()
    };
}

Rectangle<int> PlotGeometry::leftMargin (Rectangle<int> area, BorderSize<int> margin)
{
    return {
        0,
        margin.getTop(),
        margin.getLeft(),
        area.getBottom() - margin.getTopAndBottom()
    };
}

Rectangle<int> PlotGeometry::rightMargin (Rectangle<int> area, BorderSize<int> margin)
{
    return {
        area.getRight() - margin.getRight(),
        margin.getTop(),
        margin.getRight(),
        area.getBottom() - margin.getTopAndBottom()
    };
}




//=============================================================================
Rectangle<double> FigureModel::getDomain() const
{
    return Rectangle<double> (xmin, ymin, xmax - xmin, ymax - ymin);
}




//=============================================================================
bool ColourmapHelpers::looksLikeRGBTable (File file)
{
    return file.hasFileExtension (".cmap");
}

Array<Colour> ColourmapHelpers::coloursFromRGBTable (const String& string)
{
    auto tokens = StringArray::fromTokens (string, "\n ", "");
    tokens.removeEmptyStrings();

    if (tokens.size() % 3 != 0)
    {
        throw std::invalid_argument ("ASCII colormap table must have 3 columns");
    }

    auto res = Array<Colour>();
    res.resize (tokens.size() / 3);

    for (int n = 0; n < res.size(); ++n)
    {
        auto r = tokens[3 * n + 0].getIntValue();
        auto g = tokens[3 * n + 1].getIntValue();
        auto b = tokens[3 * n + 2].getIntValue();
        res.setUnchecked (n, Colour::fromRGB (r, g, b));
    }
    return res;
}

std::vector<uint32> ColourmapHelpers::textureFromRGBTable (const juce::String &string)
{
    return fromColours (coloursFromRGBTable (string));
}

std::vector<uint32> ColourmapHelpers::fromColours (const Array<Colour>& colours)
{
    auto res = std::vector<uint32> (colours.size());

    for (std::size_t n = 0; n < res.size(); ++n)
    {
        res[n] = toRGBA (colours[int (n)]);
    }
    return res;
}

uint32 ColourmapHelpers::toRGBA (const juce::Colour &c)
{
    return (c.getRed() << 0) | (c.getGreen() << 8) | (c.getBlue() << 16) | (c.getAlpha() << 24);
}
