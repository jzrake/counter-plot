#include "MetalRendering.h"




// =============================================================================
static NSString* _Nonnull shaderSource = @""
"#include <metal_stdlib>\n"
"#include <simd/simd.h>\n"
"\n"
"\n"
"\n"
"typedef struct\n"
"{\n"
"    vector_float4 position [[position]];\n"
"    vector_float4 color;\n"
"} RasterizerData;\n"
"\n"
"\n"
"\n"
"\n"
"vertex RasterizerData\n"
"vertexShader(unsigned int                     vertexID        [[vertex_id]],\n"
"             device const vector_float2      *vertexPositions [[buffer(0)]],\n"
"             device const vector_float4      *vertexColors    [[buffer(1)]],\n"
"             device const vector_float4      &domain          [[buffer(2)]])\n"
"{\n"
"    RasterizerData out;\n"
"    out.position.x = -1.f + 2.f * (vertexPositions[vertexID].x - domain[0]) / (domain[1] - domain[0]);\n"
"    out.position.y = -1.f + 2.f * (vertexPositions[vertexID].y - domain[2]) / (domain[3] - domain[2]);\n"
"    out.position.zw = vector_float2(0, 1);\n"
"    out.color = vertexColors[vertexID];\n"
"    return out;\n"
"}\n"
"\n"
"fragment vector_float4 fragmentShader(RasterizerData in [[stage_in]])\n"
"{\n"
"    return in.color;\n"
"}\n";




// =============================================================================
@implementation MetalRenderer
{
    id<MTLDevice> _device;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLCommandQueue> _commandQueue;
    vector_float2 _viewportSize;
    MetalScene* _scene;
}




// =============================================================================
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)mtkView
{
    if (self = [super init])
    {
        NSError *error = NULL;
        _device = mtkView.device;

        id<MTLLibrary> library         = [_device newLibraryWithSource:shaderSource options:nil error:&error];
        id<MTLFunction> vertexFunction   = [library newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

        if (vertexFunction == nil || fragmentFunction == nil)
        {
            NSLog(@"%@ ", error.userInfo);
        }
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;

        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
        _commandQueue  = [_device newCommandQueue];
    }
    return self;
}

- (void)setScene:(nullable MetalScene*)sceneToDisplay
{
    _scene = sceneToDisplay;
}

- (nullable MetalScene*)scene
{
    return _scene;
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size
{
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
}

- (void)drawInMTKView:(nonnull MTKView*)view
{
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;

    if (renderPassDescriptor == nil)
    {
        return;
    }
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 0.0);
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

    simd_float4 domain = simd_make_float4(_scene.domain.xmin, _scene.domain.xmax,
                                          _scene.domain.ymin, _scene.domain.ymax);

    [renderEncoder setRenderPipelineState:_pipelineState];
    [renderEncoder setVertexBytes:&domain length:sizeof(domain) atIndex:2];

    for (MetalNode* node in _scene.nodes)
    {
        [renderEncoder setVertexBuffer:node.vertexPositions offset:0 atIndex:0];
        [renderEncoder setVertexBuffer:node.vertexColors offset:0 atIndex:1];
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:node.vertexCount];
    }
    [renderEncoder endEncoding];
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
}

@end




// =============================================================================
@implementation MetalViewController
{
    MetalRenderer* _renderer;
}

- (nonnull instancetype)init
{
    if (self = [super init])
    {
        MTKView* view = [[MTKView alloc] init];
        view.layer.opaque = false;
        view.enableSetNeedsDisplay = true;
        view.paused = true;
        view.device = MTLCreateSystemDefaultDevice();
        view.delegate = _renderer = [[MetalRenderer alloc] initWithMetalKitView:view];
        self.view = view;
    }
    return self;
}

- (nullable MetalScene*)scene
{
    return _renderer.scene;
}

- (void)setScene:(nullable MetalScene*)newScene
{
    _renderer.scene = newScene;

    // The render callback sometimes gets low priority using this:
    self.view.needsDisplay = true;

    // This guarantees immediate rendering but fails on some systems:
    // [_renderer drawInMTKView:(MTKView*)self.view];
}

@end




// =============================================================================
@implementation MetalNode
{
    id<MTLBuffer> _vertexPositions;
    id<MTLBuffer> _vertexColors;
    size_t _vertexCount;
}

- (nonnull instancetype)init
{
    if (self = [super init])
    {
        _vertexPositions = nil;
        _vertexColors = nil;
        _vertexCount = 0;
    }
    return self;
}

- (void)setVertexPositions:(nullable id<MTLBuffer>)vertexPositions
{
    _vertexPositions = vertexPositions;
}

- (void)setVertexColors:(nullable id<MTLBuffer>)vertexColors
{
    _vertexColors = vertexColors;
}

- (void)setVertexCount:(size_t)vertexCount
{
    _vertexCount = vertexCount;
}

- (nullable id<MTLBuffer>)vertexPositions
{
    return _vertexPositions;
}

- (nullable id<MTLBuffer>)vertexColors
{
    return _vertexColors;
}

- (size_t)vertexCount
{
    return _vertexCount;
}

@end




// =============================================================================
@implementation MetalScene
{
    NSMutableArray<MetalNode*>* _nodes;
    struct MetalDomain _domain;
}

- (nonnull instancetype)init
{
    if (self = [super init])
    {
        _nodes = [[NSMutableArray<MetalNode*> alloc] init];
        _domain.xmin = 0;
        _domain.xmax = 1;
        _domain.ymin = 0;
        _domain.ymax = 1;
    }
    return self;
}

- (void) clear
{
    [_nodes removeAllObjects];
}

- (void) addNode:(nonnull MetalNode*)node
{
    [_nodes addObject:node];
}

- (nonnull NSMutableArray<MetalNode*>*) nodes
{
    return _nodes;
}

- (void)setDomain:(struct MetalDomain)domain
{
    _domain = domain;
}

- (struct MetalDomain)domain
{
    return _domain;
}

@end
