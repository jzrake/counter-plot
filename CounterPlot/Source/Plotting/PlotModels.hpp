#pragma once
#include "JuceHeader.h"
#include <array>
#include <simd/simd.h>




//=============================================================================
enum class LineStyle { none, solid, dash, dashdot };
enum class MarkerStyle { none, circle, square, diamond, plus, cross };

class RenderingSurface;
class PlotTransformer;
class PlotArtist;
class FigureModel;




//=============================================================================
struct LinePlotModel
{
    nd::array<double, 1> x;
    nd::array<double, 1> y;
    LineStyle     lineStyle        = LineStyle::solid;
    float         lineWidth        = 1.f;
    Colour        lineColour       = Colours::black;
    MarkerStyle   markerStyle      = MarkerStyle::none;
    float         markerSize       = 1.f;
    float         markerEdgeWidth  = 1.f;
    Colour        markerFillColour = Colours::transparentBlack;
    Colour        markerEdgeColour = Colours::black;
};




//=============================================================================
struct ScalarMapping
{
    Array<Colour> stops;
    float vmin = 0.f;
    float vmax = 1.f;
};




//=============================================================================
class PlotTransformer
{
public:
    virtual ~PlotTransformer() {}
    virtual double toDomainX (double x) const = 0;
    virtual double toDomainY (double y) const = 0;
    virtual double fromDomainX (double x) const = 0;
    virtual double fromDomainY (double y) const = 0;
    virtual std::array<float, 4> getDomain() const = 0;
    virtual Rectangle<int> getRange() const = 0;
};




//=============================================================================
class PlotArtist
{
public:
    virtual ~PlotArtist() {}
    virtual void paint (Graphics& g, const PlotTransformer& trans) {}
    virtual void render (RenderingSurface& surface) {}
    virtual bool isScalarMappable() const { return false; }
    virtual ScalarMapping getScalarMapping() const { return ScalarMapping(); }
    virtual std::array<float, 2> getScalarExtent() const { return {0, 1}; }
    virtual std::array<float, 4> getSpatialExtent() const { return {0, 1, 0, 1}; }
};




//=============================================================================
class RenderingSurface : public Component
{
public:
    virtual ~RenderingSurface() {}
    virtual void setContent (std::vector<std::shared_ptr<PlotArtist>> content, const PlotTransformer& trans) = 0;
    virtual void renderTriangles (const std::vector<simd::float2>& vertices,
                                  const std::vector<simd::float4>& colors) = 0;
    virtual void renderTriangles (const std::vector<simd::float2>& vertices,
                                  const std::vector<simd::float1>& scalars,
                                  const ScalarMapping& mapping) = 0;
    virtual Image createSnapshot() const = 0;
};




//=============================================================================
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

    //=========================================================================
    static PlotGeometry compute (Rectangle<int> area, BorderSize<int> margin,
                                 float tickLabelWidth, float tickLabelHeight,
                                 float tickLabelPadding, float tickLength);
    static Rectangle<int> topMargin    (Rectangle<int> area, BorderSize<int> margin);
    static Rectangle<int> bottomMargin (Rectangle<int> area, BorderSize<int> margin);
    static Rectangle<int> leftMargin   (Rectangle<int> area, BorderSize<int> margin);
    static Rectangle<int> rightMargin  (Rectangle<int> area, BorderSize<int> margin);
};




//=============================================================================
struct FigureModel
{
    //=========================================================================
    std::vector<std::shared_ptr<PlotArtist>> content;
    double                  xmin             = 0.0;
    double                  xmax             = 1.0;
    double                  ymin             = 0.0;
    double                  ymax             = 1.0;
    String                  id               = "";
    String                  title            = "Figure";
    String                  xlabel           = "X Axis";
    String                  ylabel           = "Y Axis";
    bool                    titleShowing     = true;
    bool                    xlabelShowing    = true;
    bool                    ylabelShowing    = true;
    bool                    canEditMargin    = true;
    bool                    canEditTitle     = true;
    bool                    canEditXlabel    = true;
    bool                    canEditYlabel    = true;
    bool                    canDeformDomain  = true;
    BorderSize<int>         margin           = BorderSize<int> (90, 90, 60, 60);
    float                   borderWidth      = 1.f;
    float                   axesWidth        = 1.f;
    float                   gridlinesWidth   = 1.f;
    float                   tickLength       = 5.f;
    float                   tickWidth        = 1.f;
    float                   tickLabelPadding = 4.f;
    float                   tickLabelWidth   = 40.f;
    float                   tickLabelHeight  = 20.f;
    int                     xtickCount       = 10;
    int                     ytickCount       = 10;
    Colour                  marginColour     = Colours::transparentWhite; /**< Default to LAF value if (1,1,1,0). */
    Colour                  borderColour     = Colours::transparentWhite;
    Colour                  backgroundColour = Colours::transparentWhite;
    Colour                  gridlinesColour  = Colours::transparentWhite;
    StringPairArray         capture;


    //=========================================================================
    /**
     * Create a figure model from a DynamicObject variant. Properties not found in the
     * given variant are taken from the model given in the optional second argument.
     */
    static FigureModel fromVar (const var&, const FigureModel& defaultModel);


    /**
     * Store the model as a DynamicObject variant. Properties not different from the
     * default model values are not written.
     */
    var toVar() const;


    /**
     * Retrieve the domain extent values [xmin, xmax, ymin, ymax] as a rectangle.
     */
    Rectangle<double> getDomain() const;


    /**
     * Set the domain extent values [xmin, xmax, ymin, ymax] from a rectangle.
     */
    void setDomain (const Rectangle<double>& domain);
};




//=============================================================================
/**
 * Encapsulates a named collection of color maps and a current index that can
 * be moved forward and backward. The constructor loads a collection of
 * application-wide color maps, but you can add your own as well. In the event
 * of name duplicates, the earlier color map with the given name is over-written.
 * The color maps stay in the order they were added.
 */
class ColourMapCollection
{
public:
    //=========================================================================
    ColourMapCollection (bool loadDefaults=true);
    void clear();
    void add (const String& nameToAdd, const Array<Colour>& stopsToAdd);
    void setCurrent (const int newIndex);
    void setCurrent (const String& name);
    Array<Colour> next();
    Array<Colour> prev();
    Array<Colour> getStops (int index) const;
    Array<Colour> getCurrentStops() const;
    int getCurrentIndex() const;
    int size() const;
    String getCurrentName() const;
    String getName (int index) const;

private:
    //=========================================================================
    StringArray names;
    Array<Array<Colour>> stops;
    int currentIndex = 0;
};




//=============================================================================
/**
 * This class provides helpers for loading color map data into a format compatible
 * with GPU texture objects with a uint32 RGBA pixel format.
 */
class ColourMapHelpers
{
public:

    /**
     * Return true if the given file is likely an ASCII table of RGB values. This probably
     * just checks that the file extension is ".cmap".
     */
    static bool looksLikeRGBTable (File);

    /**
     * Load a sequence of colors a whitespace-seperated ASCII table. The string
     * must contain whitespace-separated entries e.g. r1 g1 b1 r2 g2 b2, where
     * r, g, b are values in the range [0, 255].
     */
    static Array<Colour> coloursFromRGBTable (const String& string);

    /**
     * Return an array of doubles for a channel in the given array of colours.
     * The channel value must be 'r', 'g', 'b', or 'a'.
     */
    static nd::array<double, 1> extractChannelAsDouble (const Array<Colour>& colours, char channel);

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




//=============================================================================
class MeshHelpers
{
public:
    using Bailout = std::function<bool()>;

    /**
     * Return a list of 2D triangle vertices that cover a uniform rectilinear
     * mesh. ni and nj are the number cells the mesh has in each direction, and
     * extent is (xmin, xmax, ymin, ymax) of the mesh. It takes two triangles to
     * cover each cell, so a total of 6 * ni * nj vertices are returned. The bailout
     * callback is invoked periodically, and if it returns true then the loop is
     * terminated and an empty array is returned.
     */
    static std::vector<simd::float2> triangulateUniformRectilinearMesh (int ni, int nj, std::array<float, 4> extent, Bailout=nullptr);

    /**
     * Return a list of scalars corresponding to the triangulation of a rectilinear
     * mesh. The input array identifies scalar quantities at cell locations, and the
     * output data is just those scalars duplicated 6 times (once for each triangle
     * vertex) contiguously. The bailout callback is invoked periodically, and if it
     * returns true then the loop is terminated and an empty array is returned.
     */
    static std::vector<simd::float1> makeRectilinearGridScalars (const nd::array<double, 2>& scalar, Bailout=nullptr);

    /**
     * Return the minimum and maximum values of a scalar field. The bailout callback
     * is invoked periodically, and if it returns true then the loop is terminated
     * and (0, 1) is returned.
     */
    static std::array<float, 2> findScalarExtent (const nd::array<double, 2>& scalar, Bailout=nullptr);

    /**
     * Return the logarithm-base-10 of the given scalar data. The bailout callback is
     * invoked periodically, and if it returns true then the loop is terminated and an
     * empty array is returned.
     */
    static nd::array<double, 2> scaleByLog10 (const nd::array<double, 2>& scalar, Bailout=nullptr);
};
