#pragma once
#include "JuceHeader.h"
#include "Viewer.hpp"




//=============================================================================
class PDFViewer : public Viewer
{
public:

    //=========================================================================
    PDFViewer();
    ~PDFViewer();
    bool isInterestedInFile (File file) const override;
    void loadFile (File file) override;
    void reloadFile() override;
    String getViewerName() const override { return "PDF Document"; }

    //=========================================================================
    void resized() override;

private:
    class Impl;
    File currentFile;
    NSViewComponent view;
    std::unique_ptr<Impl> impl;
};
