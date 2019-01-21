#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"
#include "ResizerFrame.hpp"




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
private:
    DeviceBufferFloat2 vertices;
    DeviceBufferFloat1 scalars;
    ScalarMapping mapping;
};




//=============================================================================
class FigureView : public Component, private Label::Listener
{
public:
    enum ColourIds
    {
        marginColourId     = 0x0771201,
        borderColourId     = 0x0771202,
        backgroundColourId = 0x0771203,
        gridlinesColourId  = 0x0771204,
    };

    enum class ColourScheme
    {
        dark, light
    };

    static void setLookAndFeelDefaults (LookAndFeel&, ColourScheme scheme);
    static void setComponentColours (Component&, const FigureModel& model);

    //=========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void figureViewSetDomain (FigureView*, const Rectangle<double>&) = 0;
        virtual void figureViewSetMargin (FigureView*, const BorderSize<int>&) = 0;
        virtual void figureViewSetXlabel (FigureView*, const String&) = 0;
        virtual void figureViewSetYlabel (FigureView*, const String&) = 0;
        virtual void figureViewSetTitle  (FigureView*, const String&) = 0;

        /* DEP */
        virtual void figureViewSetDomainAndMargin (FigureView*, const Rectangle<double>&, const BorderSize<int>&) {}
    };

    //=========================================================================
    class MessageSink
    {
    public:
        virtual ~MessageSink() {}
        virtual void figureMousePosition (Point<double> position) = 0;
    };

    //=========================================================================
    class PlotArea : public Component, public PlotTransformer
    {
    public:
        PlotArea (FigureView&);
        void paint (Graphics&) override;
        void resized() override;
        void mouseMove (const MouseEvent&) override;
        void mouseExit (const MouseEvent&) override;
        void mouseDown (const MouseEvent&) override;
        void mouseDrag (const MouseEvent&) override;
        void mouseMagnify (const MouseEvent&, float) override;
        void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;

        //=====================================================================
        double toDomainX (double x) const override;
        double toDomainY (double y) const override;
        double fromDomainX (double x) const override;
        double fromDomainY (double y) const override;
        std::array<float, 4> getDomain() const override;
        Rectangle<int> getRange() const override;

    private:
        //=====================================================================
        void handleResizer (const Rectangle<int>& proposedBounds);
        void sendSetMargin (const BorderSize<int>& margin);
        void sendSetDomain (const Rectangle<double>& domain);
        void sendSetDomainAndMargin (const Rectangle<double>& domain, const BorderSize<int>& margin);
        Rectangle<double> computeZoomedDomain (const MouseEvent&, float scaleFactor) const;

        //=====================================================================
        FigureView& figure;
        ResizerFrame resizer;
        Rectangle<double> domainBeforePan;
        friend class FigureView;
    };

    //=========================================================================


    /**
     * Construct a figure view with a default model.
     */
    FigureView();


    /**
     * Constructor a figure view with the given model.
     */
    FigureView (const FigureModel& model);


    /**
     * Set a hardware rendering surface for this figure to use. You should do
     * this if any of the plot artists in the model override the render
     * method, otherwise those artists will not be drawn. However note that
     * there may be some overhead in the creation of the rendering surface,
     * and in the responsiveness of the app if your figures have one when it's
     * not needed.
     */
    void setRenderingSurface (std::unique_ptr<RenderingSurface> surfaceToRenderInto);


    /**
     * Set the model to use. This will trigger resizing of the plot area and
     * repaints as needed.
     */
    void setModel (const FigureModel&);


    /**
     * Return the current figure model.
     */
    const FigureModel& getModel() const { return model; }


    /**
     * This method should be used instead of Component::setBounds if the model
     * might have canDeformDomain = false. In that case, the resize will also
     * require a change to the plot domain. This method will notify listeners
     * via Listener::figureViewSetDomain, passing the recommended domain size.
     */
    void setBoundsAndSendDomainResizeIfNeeded (const Rectangle<int>& newBounds);


    /**
     * If this figure might have canDeformDomain = false, and a new margin
     * value will be given to the figure, this method should first be called
     * to request that a new recommended domain size be sent to the listeners.
     */
    bool sendDomainResizeForNewMargin (const BorderSize<int>& newMargin);


    /**
     * Add a listener. Note that this is really a controller pattern, as there
     * should probably only be a single listener. That listener should update
     * the figure model as it sees fit in response to the messages it receives,
     * and then call setModel when it's ready to update the figure.
     */
    void addListener (Listener* listener);


    /**
     * Remove a listener.
     */
    void removeListener (Listener* listener);


    /**
     * Return the subset of the figure bounds containing the plot area itself.
     */
    Rectangle<int> getPlotAreaBounds() const { return plotArea.getBounds(); }


    /**
     * Return the rendering surface currently in use.
     */
    RenderingSurface* getRenderingSurface() { return surface.get(); }


    /**
     * Return an image of the figure, including a frame capture of the rendering
     * surface, if there is one.
     */
    Image createSnapshot();


    //=========================================================================
    void paint (Graphics&) override;
    void paintOverChildren (Graphics&) override;
    void resized() override;
    void mouseEnter (const MouseEvent&) override;
    void mouseExit (const MouseEvent&) override;
    void mouseDown (const MouseEvent&) override;

private:
    //=========================================================================
    void layout();
    void refreshModes (bool alsoRepaint=true);
    PlotGeometry computeGeometry() const;
    Rectangle<double> undeformedDomain (const Rectangle<int>& newPlotAreaBounds) const;
    void labelTextChanged (Label* labelThatHasChanged) override;

    //=========================================================================
    FigureModel model;
    PlotArea plotArea;
    Label xlabel;
    Label ylabel;
    Label title;
    GridItem gridItem;
    std::unique_ptr<RenderingSurface> surface;
    ListenerList<Listener> listeners;
    bool annotateGeometry = false;
    bool allowPlotAreaResize = true;
    bool paintAxisLabels = true;
    bool paintTickLabels = true;
    bool paintMarginsAndBackground = true;
};
