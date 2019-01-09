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

void FigureModel::setDomain (const Rectangle<double>& domain)
{
    xmin = domain.getX();
    xmax = domain.getRight();
    ymin = domain.getY();
    ymax = domain.getBottom();
}




//=============================================================================
ColourMapCollection::ColourMapCollection (bool loadDefaults)
{
    if (loadDefaults)
    {
        add ("cividis",  ColourMapHelpers::coloursFromRGBTable (BinaryData::cividis_cmap));
        add ("dawn",     ColourMapHelpers::coloursFromRGBTable (BinaryData::dawn_cmap));
        add ("fire",     ColourMapHelpers::coloursFromRGBTable (BinaryData::fire_cmap));
        add ("inferno",  ColourMapHelpers::coloursFromRGBTable (BinaryData::inferno_cmap));
        add ("magma",    ColourMapHelpers::coloursFromRGBTable (BinaryData::magma_cmap));
        add ("plasma",   ColourMapHelpers::coloursFromRGBTable (BinaryData::plasma_cmap));
        add ("seashore", ColourMapHelpers::coloursFromRGBTable (BinaryData::seashore_cmap));
        add ("viridis",  ColourMapHelpers::coloursFromRGBTable (BinaryData::viridis_cmap));
    }
}

void ColourMapCollection::clear()
{
    names.clear();
    stops.clear();
    currentIndex = 0;
}

void ColourMapCollection::add (const String& nameToAdd, const Array<Colour>& stopsToAdd)
{
    if (names.contains (nameToAdd))
    {
        int n = names.indexOf (nameToAdd);
        names.remove (n);
        stops.remove (n);
    }
    names.add (nameToAdd);
    stops.add (stopsToAdd);
}

void ColourMapCollection::setCurrent (const int newIndex)
{
    currentIndex = newIndex;
}

void ColourMapCollection::setCurrent (const String& name)
{
    setCurrent (names.indexOf (name));
}

Array<Colour> ColourMapCollection::next()
{
    setCurrent ((currentIndex + 1) % names.size());
    return getCurrentStops();
}

Array<Colour> ColourMapCollection::prev()
{
    setCurrent ((currentIndex - 1 + names.size()) % names.size());
    return getCurrentStops();
}

Array<Colour> ColourMapCollection::getCurrentStops() const
{
    return stops[currentIndex];
}

int ColourMapCollection::getCurrentIndex() const
{
    return currentIndex;
}

String ColourMapCollection::getCurrentName() const
{
    return names[currentIndex];
}




//=============================================================================
bool ColourMapHelpers::looksLikeRGBTable (File file)
{
    return file.hasFileExtension (".cmap");
}

Array<Colour> ColourMapHelpers::coloursFromRGBTable (const String& string)
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

Array<double> ColourMapHelpers::extractChannelAsDouble (const Array<Colour>& colours, char channel)
{
    Array<double> res;
    res.ensureStorageAllocated (colours.size());

    for (const auto& c : colours)
    {
        switch (channel)
        {
            case 'r': res.add (c.getFloatRed()); break;
            case 'g': res.add (c.getFloatGreen()); break;
            case 'b': res.add (c.getFloatBlue()); break;
            case 'a': res.add (c.getFloatAlpha()); break;
        }
    }
    return res;
}

std::vector<uint32> ColourMapHelpers::textureFromRGBTable (const juce::String &string)
{
    return fromColours (coloursFromRGBTable (string));
}

std::vector<uint32> ColourMapHelpers::fromColours (const Array<Colour>& colours)
{
    auto res = std::vector<uint32> (colours.size());

    for (std::size_t n = 0; n < res.size(); ++n)
    {
        res[n] = toRGBA (colours[int (n)]);
    }
    return res;
}

uint32 ColourMapHelpers::toRGBA (const juce::Colour &c)
{
    return (c.getRed() << 0) | (c.getGreen() << 8) | (c.getBlue() << 16) | (c.getAlpha() << 24);
}




//=============================================================================
std::vector<simd::float2> MeshHelpers::triangulateUniformRectilinearMesh (int ni, int nj, std::array<float, 4> extent, Bailout bailout)
{
    if (bailout())
    {
        return {};
    }
    std::vector<simd::float2> verts;
    verts.reserve (ni * nj * 6);
    int iter = 0;

    for (int i = 0; i < ni; ++i)
    {
        for (int j = 0; j < nj; ++j)
        {
            const float x0 = extent[0] + (i + 0) * (extent[1] - extent[0]) / ni;
            const float y0 = extent[2] + (j + 0) * (extent[1] - extent[0]) / nj;
            const float x1 = extent[0] + (i + 1) * (extent[3] - extent[2]) / ni;
            const float y1 = extent[2] + (j + 1) * (extent[3] - extent[2]) / nj;

            verts.push_back (simd::float2 {x0, y0});
            verts.push_back (simd::float2 {x0, y1});
            verts.push_back (simd::float2 {x1, y0});
            verts.push_back (simd::float2 {x0, y1});
            verts.push_back (simd::float2 {x1, y0});
            verts.push_back (simd::float2 {x1, y1});

            if (++iter % 1000 == 0 && bailout != nullptr && bailout())
            {
                return {};
            }
        }
    }
    return verts;
}

//std::vector<simd::float2> MeshHelpers::triangulateGeneralRectilinearGrid (const nd::array<double, 3>& vertices, Bailout bailout)
//{
//    return {};
//}

std::vector<simd::float1> MeshHelpers::makeRectilinearGridScalars (const nd::array<double, 2>& scalar, Bailout bailout)
{
    if (bailout())
    {
        return {};
    }
    std::vector<simd::float1> result;
    result.reserve (scalar.size() * 6);
    int iter = 0;

    for (const auto& s : scalar)
    {
        for (int n = 0; n < 6; ++n)
        {
            result.push_back (s);
        }
        if (++iter % 1000 == 0 && bailout != nullptr && bailout())
        {
            return {};
        }
    }
    return result;
}

std::array<float, 2> MeshHelpers::findScalarExtent (const nd::array<double, 2>& scalar, Bailout bailout)
{
    if (bailout())
    {
        return {0.f, 1.f};
    }
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::min();
    int iter = 0;

    for (const auto& s : scalar)
    {
        if (s < min) min = s;
        if (s > max) max = s;

        if (++iter % 1000 == 0 && bailout != nullptr && bailout())
        {
            return {0.f, 1.f};
        }
    }
    return {min, max};
}

nd::array<double, 2> MeshHelpers::scaleByLog10 (const nd::array<double, 2>& scalar, Bailout bailout)
{
    if (bailout())
    {
        return nd::array<double, 2>();
    }
    auto result = scalar.copy();
    int iter = 0;

    for (auto& x : result)
    {
        x = std::logf (x);

        if (++iter % 1000 == 0 && bailout != nullptr && bailout())
        {
            return nd::array<double, 2>();
        }
    }
    return result;
}
