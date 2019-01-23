#include <Quartz/Quartz.h>
#include "PDFViewerImplementation.h"




// =============================================================================
@implementation PDFViewerImplementation
{
    PDFView* _view;
}

- (instancetype) init
{
    if (self = [super init])
    {
        _view = [[PDFView alloc] init];
    }
    return self;
}

- (nonnull NSView*)getView
{
    return _view;
}

- (void)setFile:(const char*)newFileToView
{
    if (! newFileToView)
    {
        _view.document = nil;
    }
    else
    {
        NSString* path = [[NSString alloc] initWithUTF8String:newFileToView];
        _view.document = [[PDFDocument alloc] initWithURL:[[NSURL alloc] initFileURLWithPath:path]];
    }
}

@end
