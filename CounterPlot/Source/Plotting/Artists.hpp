#pragma once
#include "PlotModels.hpp"




//=============================================================================
class ColourGradientArtist : public PlotArtist
{
public:

    //=========================================================================
    enum class Orientation
    {
        horizontal, vertical,
    };

    //=========================================================================
    /** Default constructor. */
    ColourGradientArtist();

    /** Construct with color stops. */
    ColourGradientArtist (const Array<Colour>& stops);

    /**
     * Replace the color stops defining the gradient drawn by this artist.
     */
    void setStops (const Array<Colour>& stops);

    /**
     * Set the orientation of the gradient: left-to-right or top-to-bottom.
     */
    void setOrientation (Orientation orientationToUse);

    /**
     * Set the orientation of the gradient using a string version of the enum
     * key. Can throw an invalid_argument exception if the name does not exist.
     */
    void setOrientation (const String& name, bool throwIfNotFound);

    /**
     * If this is set to true, then the gradient will be drawn in the domain
     * [0, 1], transformed to the target pixel range. By default it is false:
     * the gradient is drawn from edge-to-edge in the target range.
     */
    void setGradientFollowsTransform (bool shouldGradientBeTransformed);

    //=========================================================================
    void paint (Graphics& g, const PlotTransformer& trans) override;

private:
    //=========================================================================
    Array<Colour> stops;
    Orientation orientation = Orientation::vertical;
    bool transformGradient = false;
};




//=============================================================================
class LinePlotArtist : public PlotArtist
{
public:
    LinePlotArtist() {}
    LinePlotArtist (LinePlotModel model);
    void paint (Graphics& g, const PlotTransformer& trans) override;
private:
    LinePlotModel model;
};




//=============================================================================
class TriangleMeshArtist : public PlotArtist
{
public:
    TriangleMeshArtist (DeviceBufferFloat2 vertices, DeviceBufferFloat1 scalars, ScalarMapping mapping);
    void render (RenderingSurface& surface) override;
    bool wantsSurface() const override { return true; }

private:
    DeviceBufferFloat2 vertices;
    DeviceBufferFloat1 scalars;
    ScalarMapping mapping;
};
