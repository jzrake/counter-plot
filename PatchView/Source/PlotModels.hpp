#pragma once
#include "JuceHeader.h"
#include <array>
#include <simd/simd.h>




//==============================================================================
class RenderingSurface;
class PlotTransformer;
class PlotArtist;
class FigureModel;




//==============================================================================
class PlotTransformer
{
public:
    virtual ~PlotTransformer() {}
    virtual double toDomainX (double x) const = 0;
    virtual double toDomainY (double y) const = 0;
    virtual double fromDomainX (double x) const = 0;
    virtual double fromDomainY (double y) const = 0;
    virtual std::array<float, 4> getDomain() const = 0;
};




//==============================================================================
class PlotArtist
{
public:
    virtual ~PlotArtist() {}
    virtual void paint (Graphics& g, const PlotTransformer& trans) {}
    virtual void render (RenderingSurface& surface) {}
};




//==============================================================================
class RenderingSurface : public Component
{
public:
    virtual ~RenderingSurface() {}
    virtual void setContent (std::vector<std::shared_ptr<PlotArtist>> content, const PlotTransformer& trans) = 0;
    virtual void renderTriangles (const std::vector<simd::float2>& vertices,
                                  const std::vector<simd::float4>& colors) = 0;
    virtual void renderTriangles (const std::vector<simd::float2>& vertices,
                                  const std::vector<simd::float1>& scalars,
                                  float vmin, float vmax) = 0;
};




//==========================================================================
struct PlotGeometry
{
    Rectangle<int> marginT;
    Rectangle<int> marginB;
    Rectangle<int> marginL;
    Rectangle<int> marginR;
    Rectangle<int> xtickAreaT;
    Rectangle<int> xtickAreaB;
    Rectangle<int> ytickAreaL;
    Rectangle<int> ytickAreaR;
    Rectangle<int> xtickLabelAreaT;
    Rectangle<int> xtickLabelAreaB;
    Rectangle<int> ytickLabelAreaL;
    Rectangle<int> ytickLabelAreaR;

    //==========================================================================
    static PlotGeometry compute (Rectangle<int> area, BorderSize<int> margin,
                                 float tickLabelWidth, float tickLabelHeight,
                                 float tickLabelPadding, float tickLength);
    static Rectangle<int> topMargin    (Rectangle<int> area, BorderSize<int> margin);
    static Rectangle<int> bottomMargin (Rectangle<int> area, BorderSize<int> margin);
    static Rectangle<int> leftMargin   (Rectangle<int> area, BorderSize<int> margin);
    static Rectangle<int> rightMargin  (Rectangle<int> area, BorderSize<int> margin);
};




//==============================================================================
struct ColorbarModel
{
    Array<Colour> stops;
    String title;
    float lower = 0.f;
    float upper = 1.f;
};




//==============================================================================
struct FigureModel
{
    std::vector<std::shared_ptr<PlotArtist>> content;
    double                  xmin             = 0.0;
    double                  xmax             = 1.0;
    double                  ymin             = 0.0;
    double                  ymax             = 1.0;
    String                  title            = "Figure";
    String                  xlabel           = "X Axis";
    String                  ylabel           = "Y Axis";
    BorderSize<int>         margin           = BorderSize<int> (90, 90, 60, 60);
    float                   borderWidth      = 1.f;
    float                   axesWidth        = 1.f;
    float                   gridlinesWidth   = 1.f;
    float                   tickLength       = 5.f;
    float                   tickWidth        = 1.f;
    float                   tickLabelPadding = 4.f;
    float                   tickLabelWidth   = 40.f;
    float                   tickLabelHeight  = 20.f;
    Colour                  marginColour     = Colours::whitesmoke;
    Colour                  borderColour     = Colours::black;
    Colour                  backgroundColour = Colours::white;
    Colour                  gridlinesColour  = Colours::lightgrey;

    //==========================================================================
    Rectangle<double> getDomain() const;
};




//==============================================================================
/**
 * This class provides helpers for loading color map data into a format compatible
 * with GPU texture objects with a uint32 RGBA pixel format.
 */
class ColormapHelpers
{
public:

    /**
     * Load a sequence of colors a whitespace-seperated ASCII table. The string
     * must contain whitespace-separated entries e.g. r1 g1 b1 r2 g2 b2, where
     * r, g, b are values in the range [0, 255].
     */
    static Array<Colour> coloursFromRGBTable (const String& string);

    /**
     * Load RGBA texture data from a whitespace-seperated ASCII table. The string
     * must contain whitespace-separated entries e.g. r1 g1 b1 r2 g2 b2, where
     * r, g, b are values in the range [0, 255].
     */
    static std::vector<uint32> textureFromRGBTable (const String& string);

    /**
     * Convert an array of JUCE colours to uint32 RGBA texture data.
     */
    static std::vector<uint32> fromColours (const Array<Colour>& colours);

    /**
     * Return an RBGA-formatted integer for the given JUCE::Colour. JUCE only
     * provides an ARGB method for some reason.
     */
    static uint32 toRGBA (const Colour& c);
};
