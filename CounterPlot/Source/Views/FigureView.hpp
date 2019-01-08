#pragma once
#include "JuceHeader.h"
#include "../PlotModels.hpp"




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
    ColourGradientArtist (ScalarMapping model);

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
    ScalarMapping model;
    Orientation orientation = Orientation::vertical;
    bool transformGradient = false;
};




//=============================================================================
class LinePlotArtist : public PlotArtist
{
public:
    LinePlotArtist (LinePlotModel model);
    void paint (Graphics& g, const PlotTransformer& trans) override;
private:
    LinePlotModel model;
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
        virtual void figureViewSetMargin (FigureView*, const BorderSize<int>&) = 0;
        virtual void figureViewSetDomain (FigureView*, const Rectangle<double>&) = 0;
        virtual void figureViewSetXlabel (FigureView*, const String&) = 0;
        virtual void figureViewSetYlabel (FigureView*, const String&) = 0;
        virtual void figureViewSetTitle  (FigureView*, const String&) = 0;
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

        //=====================================================================
        double toDomainX (double x) const override;
        double toDomainY (double y) const override;
        double fromDomainX (double x) const override;
        double fromDomainY (double y) const override;
        std::array<float, 4> getDomain() const override;
        Rectangle<int> getRange() const override;

    private:
        //=====================================================================
        BorderSize<int> computeMargin() const;
        void sendSetMarginIfNeeded();
        void sendSetDomain (const Rectangle<double>& domain);

        //=====================================================================
        FigureView& figure;
        ComponentBoundsConstrainer constrainer;
        ResizableBorderComponent resizer;
        Rectangle<double> domainBeforePan;

        friend class FigureView;
    };

    //=========================================================================
    FigureView();
    void setRenderingSurface (std::unique_ptr<RenderingSurface> surfaceToRenderInto);
    void setModel (const FigureModel&);
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    const FigureModel& getModel() const { return model; }
    Rectangle<int> getPlotAreaBounds() const { return plotArea.getBounds(); }
    RenderingSurface* getRenderingSurface() { return surface.get(); }
    GridItem& getGridItem() { return gridItem; }
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
