#pragma once
#include "JuceHeader.h"




//=============================================================================
class ResizerFrame : public Component
{
public:

    //=========================================================================
    ResizerFrame();
    void setCallback (std::function<void(Rectangle<int>)> callbackToInvoke);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;
    void mouseDown (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;
    bool hitTest (int x, int y) override;
    MouseCursor getMouseCursor() override;

private:
    //=========================================================================
    int getZoneForPoint (Point<int> p) const;
    std::function<void(Rectangle<int>)> callback = nullptr;
    MouseCursor mouseCursors[9];
    Rectangle<int> zoneRectangles[9];
    Rectangle<int> boundsOnMouseDown;
    int zoneOnMouseDown = 4;
    int width = 8;
};




//=============================================================================
class ResizerFrameTestComponent : public Component
{
public:

    //=========================================================================
    ResizerFrameTestComponent();

    //=========================================================================
    void resized() override;
    void paint (Graphics& g) override;

private:
    //=========================================================================
    ResizerFrame frame;
};
