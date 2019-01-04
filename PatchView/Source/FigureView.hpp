#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"




//=============================================================================
class ColourGradientArtist : public PlotArtist
{
public:
    ColourGradientArtist (ScalarMapping model);
    void paint (Graphics& g, const PlotTransformer& trans) override;
private:
    ScalarMapping model;
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

    //=========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void figureViewSetMargin (FigureView* figure, const BorderSize<int>& value) = 0;
        virtual void figureViewSetDomain (FigureView* figure, const Rectangle<double>& value) = 0;
        virtual void figureViewSetXlabel (FigureView* figure, const String& value) = 0;
        virtual void figureViewSetYlabel (FigureView* figure, const String& value) = 0;
        virtual void figureViewSetTitle (FigureView* figure, const String& value) = 0;
    };

    //=========================================================================
    class PlotArea : public Component, public PlotTransformer
    {
    public:
        PlotArea (FigureView&);
        void paint (Graphics&) override;
        void resized() override;
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
    Rectangle<int> getPlotAreaBounds() const;
    RenderingSurface* getRenderingSurface() { return surface.get(); }

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
    void refreshModes();
    PlotGeometry computeGeometry() const;
    void labelTextChanged (Label* labelThatHasChanged) override;

    //=========================================================================
    FigureModel model;
    PlotArea plotArea;
    Label xlabel;
    Label ylabel;
    Label title;

    std::unique_ptr<RenderingSurface> surface;
    ListenerList<Listener> listeners;
    bool annotateGeometry = false;
    bool allowPlotAreaResize = true;
    bool paintAxisLabels = true;
    bool paintTickLabels = true;
    bool paintMarginsAndBackground = true;
};
