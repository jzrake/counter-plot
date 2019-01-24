#include <Quartz/Quartz.h>
#include "PDFComponentMacOS.hpp"




//=========================================================================
class PDFViewComponent::Impl
{
public:
    Impl()
    {
        view = [[PDFView alloc] init];
    }
    PDFView* view;
};




//=========================================================================
PDFViewComponent::PDFViewComponent()
{
    impl = std::make_unique<Impl>();
    setView (impl->view);
}

PDFViewComponent::~PDFViewComponent()
{
}

void PDFViewComponent::setViewedFile (File fileToView)
{
    NSString* path = [[NSString alloc] initWithUTF8String:fileToView.getFullPathName().toRawUTF8()];
    impl->view.document = [[PDFDocument alloc] initWithURL:[[NSURL alloc] initFileURLWithPath:path]];
}
