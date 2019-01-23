#pragma once


@class NSView;


// =============================================================================
@interface PDFViewerImplementation : NSObject
- (nonnull instancetype)init;
- (nonnull NSView*)getView;
- (void) setFile:(nullable const char*)newFileToView;
@end
