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
    BorderSize<int>         margin           = BorderSize<int> (40, 90, 50, 30);
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
    Rectangle<int> getTopMargin (const Rectangle<int>& area) const;
    Rectangle<int> getBottomMargin (const Rectangle<int>& area) const;
    Rectangle<int> getLeftMargin (const Rectangle<int>& area) const;
    Rectangle<int> getRightMargin (const Rectangle<int>& area) const;
    Rectangle<double> getDomain() const;
};
