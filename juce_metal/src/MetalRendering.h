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
- (nonnull instancetype)init;
- (void)setVertexPositions:(nullable id<MTLBuffer>)vertexPositions;
- (void)setVertexColors:(nullable id<MTLBuffer>)vertexColors;
- (void)setVertexCount:(size_t)vertexCount;
- (nullable id<MTLBuffer>)vertexPositions;
- (nullable id<MTLBuffer>)vertexColors;
- (size_t)vertexCount;
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
