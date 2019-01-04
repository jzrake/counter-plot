#pragma once
#include "JuceHeader.h"




//==============================================================================
class OutlineView : public Component
{
public:
    OutlineView();
    void resized() override;
    void paint (Graphics& g) override;
    void mouseMove (const MouseEvent& event) override;
    void mouseExit (const MouseEvent& event) override;
    void mouseDown (const MouseEvent& event) override;

private:
    var model;
    int mouseOverRow = -1;
    int selectedRow = -1;
};
