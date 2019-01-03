#pragma once
#include <MetalKit/MetalKit.h>




// =============================================================================
struct MetalDomain
{
    float xmin;
    float xmax;
    float ymin;
    float ymax;
};




// =============================================================================
@interface MetalNode : NSObject
{
    id<MTLBuffer> _vertexPositions;
    id<MTLBuffer> _vertexColors;
    id<MTLBuffer> _vertexScalars;
    id<MTLTexture> _scalarMapping;
    float _scalarDomainLower;
    float _scalarDomainUpper;
    size_t _vertexCount;
}
- (nonnull instancetype)init;
@property(nullable, retain) id<MTLBuffer> vertexPositions;
@property(nullable, retain) id<MTLBuffer> vertexColors;
@property(nullable, retain) id<MTLBuffer> vertexScalars;
@property(nullable, retain) id<MTLTexture> scalarMapping;
@property float scalarDomainLower;
@property float scalarDomainUpper;
@property size_t vertexCount;
@end




// =============================================================================
@interface MetalScene : NSObject
- (nonnull instancetype)init;
- (void)clear;
- (void)addNode:(nonnull MetalNode*)node;
- (nonnull NSMutableArray<MetalNode*>*) nodes;
- (void)setDomain:(struct MetalDomain)domain;
- (struct MetalDomain)domain;
@end




// =============================================================================
@interface MetalViewController : NSViewController
- (nullable MetalScene*)scene;
- (void) setScene:(nullable MetalScene*)newScene;
@end




// =============================================================================
@interface MetalRenderer : NSObject<MTKViewDelegate>
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)mtkView;
- (void) setScene:(nullable MetalScene*)sceneToDisplay;
- (nullable MetalScene*)scene;
@end
