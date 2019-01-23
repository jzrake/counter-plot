#include "PDFViewerMacOS.hpp"
#include "PDFViewerImplementation.h"




//=========================================================================
class PDFViewer::Impl
{
public:
    Impl()
    {
        pdfViewer = [[PDFViewerImplementation alloc] init];
    }
    PDFViewerImplementation* pdfViewer = nil;
};




//=========================================================================
PDFViewer::PDFViewer()
{
    impl = std::make_unique<Impl>();
    view.setView ([impl->pdfViewer getView]);
    addAndMakeVisible (view);
}

PDFViewer::~PDFViewer()
{
}

bool PDFViewer::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".pdf");
}

void PDFViewer::loadFile (File file)
{
    currentFile = file;
    reloadFile();
}

void PDFViewer::reloadFile()
{
    [impl->pdfViewer setFile:currentFile.getFullPathName().toRawUTF8()];
}




//=========================================================================
void PDFViewer::resized()
{
    view.setBounds (getLocalBounds());
}
