#include "PlotModels.hpp"
#include "../Core/DataHelpers.hpp"
#include "../Core/Runtime.hpp"




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

Array<Colour> ColourMapCollection::getStops (int index) const
{
    return stops[index];
}

Array<Colour> ColourMapCollection::getCurrentStops() const
{
    return stops[currentIndex];
}

int ColourMapCollection::getCurrentIndex() const
{
    return currentIndex;
}

int ColourMapCollection::size() const
{
    return names.size();
}

String ColourMapCollection::getCurrentName() const
{
    return names[currentIndex];
}

String ColourMapCollection::getName (int index) const
{
    return names[index];
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

nd::array<double, 1> ColourMapHelpers::extractChannelAsDouble (const Array<Colour>& colours, char channel)
{
    nd::array<double, 1> res (colours.size());
    int n = 0;

    for (const auto& c : colours)
    {
        switch (channel)
        {
            case 'r': res(n) = c.getFloatRed(); break;
            case 'g': res(n) = c.getFloatGreen(); break;
            case 'b': res(n) = c.getFloatBlue(); break;
            case 'a': res(n) = c.getFloatAlpha(); break;
        }
        ++n;
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
    if (bailout != nullptr && bailout())
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
    if (bailout != nullptr && bailout())
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
    if (bailout != nullptr && bailout())
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
    if (bailout != nullptr && bailout())
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




//=============================================================================
static void convert (const var& source, bool& value)
{
    if (! source.isVoid()) value = source;
}
static void convert (const var& source, int& value)
{
    if (! source.isVoid()) value = source;
}
static void convert (const var& source, float& value)
{
    if (! source.isVoid()) value = source;
}
static void convert (const var& source, double& value)
{
    if (! source.isVoid()) value = source;
}
static void convert (const var& source, String& value)
{
    if (! source.isVoid()) value = source;
}
static void convert (const var& source, BorderSize<int>& value)
{
    if (! source.isVoid()) value = DataHelpers::borderSizeFromVar (source);
}
static void convert (const var& source, Colour& value)
{
    if (! source.isVoid()) value = DataHelpers::colourFromVar (source);
}
static void convert (const var& source, std::map<std::string, std::string>& value)
{
    if (! source.isVoid()) value = DataHelpers::stringMapFromVar (source);
}
static void convert (const var& source, std::vector<std::shared_ptr<PlotArtist>>& value)
{
    if (auto content = source.getArray())
    {
        for (auto element : *content)
        {
            auto artist = Runtime::check_data<std::shared_ptr<PlotArtist>> (element);
            value.push_back (artist);
        }
    }
}




//=============================================================================
FigureModel FigureModel::withoutContent() const
{
    auto result = *this;
    result.content.clear();
    return result;
}

FigureModel FigureModel::fromVar (const var& value, const FigureModel& defaultModel)
{
    FigureModel model = defaultModel;

    if (auto obj = value.getDynamicObject())
    {
        for (const auto& item : obj->getProperties())
        {
            if (false) {}
            else if (item.name == Identifier ("xmin")) convert (item.value, model.xmin);
            else if (item.name == Identifier ("xmax")) convert (item.value, model.xmax);
            else if (item.name == Identifier ("ymin")) convert (item.value, model.ymin);
            else if (item.name == Identifier ("ymax")) convert (item.value, model.ymax);
            else if (item.name == Identifier ("id")) convert (item.value, model.id);
            else if (item.name == Identifier ("title")) convert (item.value, model.title);
            else if (item.name == Identifier ("xlabel")) convert (item.value, model.xlabel);
            else if (item.name == Identifier ("ylabel")) convert (item.value, model.ylabel);
            else if (item.name == Identifier ("title-showing")) convert (item.value, model.titleShowing);
            else if (item.name == Identifier ("xlabel-showing")) convert (item.value, model.xlabelShowing);
            else if (item.name == Identifier ("ylabel-showing")) convert (item.value, model.ylabelShowing);
            else if (item.name == Identifier ("can-edit-margin")) convert (item.value, model.canEditMargin);
            else if (item.name == Identifier ("can-edit-title")) convert (item.value, model.canEditTitle);
            else if (item.name == Identifier ("can-edit-xlabel")) convert (item.value, model.canEditXlabel);
            else if (item.name == Identifier ("can-edit-ylabel")) convert (item.value, model.canEditYlabel);
            else if (item.name == Identifier ("can-deform-domain")) convert (item.value, model.canDeformDomain);
            else if (item.name == Identifier ("margin")) convert (item.value, model.margin);
            else if (item.name == Identifier ("border-width")) convert (item.value, model.borderWidth);
            else if (item.name == Identifier ("axes-width")) convert (item.value, model.axesWidth);
            else if (item.name == Identifier ("gridlines-width")) convert (item.value, model.gridlinesWidth);
            else if (item.name == Identifier ("tick-length")) convert (item.value, model.tickLength);
            else if (item.name == Identifier ("tick-width")) convert (item.value, model.tickWidth);
            else if (item.name == Identifier ("tick-label-padding")) convert (item.value, model.tickLabelPadding);
            else if (item.name == Identifier ("tick-label-width")) convert (item.value, model.tickLabelWidth);
            else if (item.name == Identifier ("tick-label-height")) convert (item.value, model.tickLabelHeight);
            else if (item.name == Identifier ("xtick-count")) convert (item.value, model.xtickCount);
            else if (item.name == Identifier ("ytick-count")) convert (item.value, model.ytickCount);
            else if (item.name == Identifier ("margin-color")) convert (item.value, model.marginColour);
            else if (item.name == Identifier ("border-color")) convert (item.value, model.borderColour);
            else if (item.name == Identifier ("background-color")) convert (item.value, model.backgroundColour);
            else if (item.name == Identifier ("gridlines-color")) convert (item.value, model.gridlinesColour);
            else if (item.name == Identifier ("content")) convert (item.value, model.content);
            else if (item.name == Identifier ("capture")) convert (item.value, model.capture);
            else DBG("unknown figure property: " << item.name.toString());
        }
    }
    return model;
}

var FigureModel::toVar() const
{
    FigureModel ref;
    auto obj = std::make_unique<DynamicObject>();
    if (xmin != ref.xmin) obj->setProperty ("xmin", xmin);
    if (xmax != ref.xmax) obj->setProperty ("xmax", xmax);
    if (ymin != ref.ymin) obj->setProperty ("ymin", ymin);
    if (ymax != ref.ymax) obj->setProperty ("ymax", ymax);
    if (title != ref.id) obj->setProperty ("id", id);
    if (title != ref.title) obj->setProperty ("title", title);
    if (xlabel != ref.xlabel) obj->setProperty ("xlabel", xlabel);
    if (ylabel != ref.ylabel) obj->setProperty ("ylabel", ylabel);
    if (titleShowing != ref.titleShowing) obj->setProperty ("title-showing", titleShowing);
    if (xlabelShowing != ref.xlabelShowing) obj->setProperty ("xlabel-showing", xlabelShowing);
    if (ylabelShowing != ref.ylabelShowing) obj->setProperty ("ylabel-showing", ylabelShowing);
    if (canEditMargin != ref.canEditMargin) obj->setProperty ("can-edit-margin", canEditMargin);
    if (canEditTitle != ref.canEditTitle) obj->setProperty ("can-edit-title", canEditTitle);
    if (canEditXlabel != ref.canEditXlabel) obj->setProperty ("can-edit-xlabel", canEditXlabel);
    if (canEditYlabel != ref.canEditYlabel) obj->setProperty ("can-edit-ylabel", canEditYlabel);
    if (canDeformDomain != ref.canDeformDomain) obj->setProperty ("can-deform-domain", canDeformDomain);
    if (margin != ref.margin) obj->setProperty ("margin", DataHelpers::varFromBorderSize (margin));
    if (borderWidth != ref.borderWidth) obj->setProperty ("border-width", borderWidth);
    if (axesWidth != ref.axesWidth) obj->setProperty ("axes-width", axesWidth);
    if (gridlinesWidth != ref.gridlinesWidth) obj->setProperty ("gridlines-width", gridlinesWidth);
    if (tickLength != ref.tickLength) obj->setProperty ("tick-length", tickLength);
    if (tickWidth != ref.tickWidth) obj->setProperty ("tick-width", tickWidth);
    if (tickLabelPadding != ref.tickLabelPadding) obj->setProperty ("tick-label-padding", tickLabelPadding);
    if (tickLabelWidth != ref.tickLabelWidth) obj->setProperty ("tick-label-width", tickLabelWidth);
    if (tickLabelHeight != ref.tickLabelHeight) obj->setProperty ("tick-label-height", tickLabelHeight);
    if (xtickCount != ref.xtickCount) obj->setProperty ("xtick-count", xtickCount);
    if (ytickCount != ref.ytickCount) obj->setProperty ("ytick-count", ytickCount);
    if (marginColour != ref.marginColour) obj->setProperty ("margin-colour", marginColour.toString());
    if (borderColour != ref.borderColour) obj->setProperty ("border-colour", borderColour.toString());
    if (backgroundColour != ref.backgroundColour) obj->setProperty ("background-colour", backgroundColour.toString());
    if (gridlinesColour != ref.gridlinesColour) obj->setProperty ("gridlines-colour", gridlinesColour.toString());
    if (capture != ref.capture) obj->setProperty ("capture", DataHelpers::varFromStringMap (capture));
    return obj.release();
}
