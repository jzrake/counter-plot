#pragma once
#include "JuceHeader.h"




//=============================================================================
class PDFViewComponent : public juce::NSViewComponent
{
public:
    PDFViewComponent();
    ~PDFViewComponent();
    void setViewedFile (juce::File fileToView);
private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
