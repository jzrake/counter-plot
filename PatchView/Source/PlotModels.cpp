#include "PlotModels.hpp"




//==============================================================================
Rectangle<int> FigureModel::getTopMargin (const Rectangle<int>& area) const
{
    return {
        margin.getLeft(),
        0,
        area.getRight() - margin.getLeftAndRight(),
        margin.getTop()
    };
}

Rectangle<int> FigureModel::getBottomMargin (const Rectangle<int>& area) const
{
    return {
        margin.getLeft(),
        area.getBottom() - margin.getBottom(),
        area.getRight() - margin.getLeftAndRight(),
        margin.getBottom()
    };
}

Rectangle<int> FigureModel::getLeftMargin (const Rectangle<int>& area) const
{
    return {
        0,
        margin.getTop(),
        margin.getLeft(),
        area.getBottom() - margin.getTopAndBottom()
    };
}

Rectangle<int> FigureModel::getRightMargin (const Rectangle<int>& area) const
{
    return {
        area.getRight() - margin.getRight(),
        margin.getTop(),
        margin.getRight(),
        area.getBottom() - margin.getTopAndBottom()
    };
}

Rectangle<double> FigureModel::getDomain() const
{
    return Rectangle<double>(xmin, ymin, xmax - xmin, ymax - ymin);
}




//==============================================================================
std::vector<uint32> ColormapHelpers::fromRGBTable (const juce::String &string)
{
    auto tokens = StringArray::fromTokens (string, "\n ", "");
    tokens.removeEmptyStrings();

    if (tokens.size() % 3 != 0)
    {
        throw std::invalid_argument ("ASCII colormap table must have 3 columns");
    }
    auto res = std::vector<uint32> (tokens.size() / 3);

    for (int n = 0; n < res.size(); ++n)
    {
        auto r = tokens[3 * n + 0].getIntValue();
        auto g = tokens[3 * n + 1].getIntValue();
        auto b = tokens[3 * n + 2].getIntValue();
        auto a = 255;
        res[n] = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    }
    return res;
}

std::vector<uint32> ColormapHelpers::fromColours (const Array<Colour>& colours)
{
    auto res = std::vector<uint32> (colours.size());

    for (std::size_t n = 0; n < res.size(); ++n)
    {
        res[n] = toRGBA (colours[int (n)]);
    }
    return res;
}

uint32 ColormapHelpers::toRGBA (const juce::Colour &c)
{
    return (c.getRed() << 0) | (c.getGreen() << 8) | (c.getBlue() << 16) | (c.getAlpha() << 24);
}
