#include "ResizerFrame.hpp"




//=============================================================================
ResizerFrame::ResizerFrame()
{
    mouseCursors[0] = MouseCursor::TopLeftCornerResizeCursor;
    mouseCursors[1] = MouseCursor::TopEdgeResizeCursor;
    mouseCursors[2] = MouseCursor::TopRightCornerResizeCursor;
    mouseCursors[3] = MouseCursor::LeftEdgeResizeCursor;
    mouseCursors[4] = MouseCursor::NormalCursor;
    mouseCursors[5] = MouseCursor::RightEdgeResizeCursor;
    mouseCursors[6] = MouseCursor::BottomLeftCornerResizeCursor;
    mouseCursors[7] = MouseCursor::BottomEdgeResizeCursor;
    mouseCursors[8] = MouseCursor::BottomRightCornerResizeCursor;
}

void ResizerFrame::setCallback (std::function<void(Rectangle<int>)> callbackToInvoke)
{
    callback = callbackToInvoke;
}




//=============================================================================
void ResizerFrame::resized()
{
    auto area      = getLocalBounds();
    auto topRow    = area.removeFromTop (width);
    auto bottomRow = area.removeFromBottom (width);
    auto middleRow = area;

    zoneRectangles[0] = topRow.removeFromLeft (width);
    zoneRectangles[2] = topRow.removeFromRight (width);
    zoneRectangles[1] = topRow;
    zoneRectangles[3] = middleRow.removeFromLeft (width);
    zoneRectangles[5] = middleRow.removeFromRight (width);
    zoneRectangles[4] = middleRow;
    zoneRectangles[6] = bottomRow.removeFromLeft (width);
    zoneRectangles[8] = bottomRow.removeFromRight (width);
    zoneRectangles[7] = bottomRow;
}

void ResizerFrame::mouseMove (const MouseEvent& e)
{
    setMouseCursor (mouseCursors[getZoneForPoint (e.getPosition())]);
}

void ResizerFrame::mouseDrag (const MouseEvent& e)
{
    auto b = boundsOnMouseDown;
    auto z = zoneCurrentlyDragging;

    if (z == 0 || z == 3 || z == 6) b.setLeft   (boundsOnMouseDown.getX()      + e.getDistanceFromDragStartX());
    if (z == 2 || z == 5 || z == 8) b.setRight  (boundsOnMouseDown.getRight()  + e.getDistanceFromDragStartX());
    if (z == 0 || z == 1 || z == 2) b.setTop    (boundsOnMouseDown.getY()      + e.getDistanceFromDragStartY());
    if (z == 6 || z == 7 || z == 8) b.setBottom (boundsOnMouseDown.getBottom() + e.getDistanceFromDragStartY());

    if (callback)
        callback (b);
}

void ResizerFrame::mouseDown (const MouseEvent& e)
{
    boundsOnMouseDown = getParentComponent()->getBounds();
    zoneCurrentlyDragging = getZoneForPoint (e.getPosition());
    setMouseCursor (mouseCursors[zoneCurrentlyDragging]);
}

void ResizerFrame::mouseUp (const MouseEvent&)
{
    zoneCurrentlyDragging = 4;
}

bool ResizerFrame::hitTest (int x, int y)
{
    for (const auto& rect : zoneRectangles)
        if (rect.contains (x, y) && rect != zoneRectangles[4])
            return true;

    return false;
}

//MouseCursor ResizerFrame::getMouseCursor()
//{
//    return mouseCursors[getZoneForPoint (getMouseXYRelative())];
//}

int ResizerFrame::getZoneForPoint (Point<int> p) const
{
    if (zoneRectangles[0].contains(p)) return 0;
    if (zoneRectangles[2].contains(p)) return 2;
    if (zoneRectangles[6].contains(p)) return 6;
    if (zoneRectangles[8].contains(p)) return 8;
    if (zoneRectangles[1].contains(p)) return 1;
    if (zoneRectangles[3].contains(p)) return 3;
    if (zoneRectangles[5].contains(p)) return 5;
    if (zoneRectangles[7].contains(p)) return 7;
    return 4;
}




//=============================================================================
ResizerFrameTestComponent::ResizerFrameTestComponent()
{
    frame.setCallback ([this] (auto b) { setBounds (b); });
    addAndMakeVisible (frame);
}




//=========================================================================
void ResizerFrameTestComponent::resized()
{
    frame.setBounds (getLocalBounds());
}

void ResizerFrameTestComponent::paint (Graphics& g)
{
    g.fillAll (Colours::brown);
}
