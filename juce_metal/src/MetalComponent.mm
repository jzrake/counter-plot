#include "MetalComponent.hpp"
#include "MetalRendering.h"




// ===========================================================================
struct metal::Buffer::Impl
{
    id<MTLBuffer> buffer;
};

struct metal::Texture::Impl
{
    id<MTLTexture> texture;
};

struct metal::Node::Impl
{
    MetalNode* node;
};

struct metal::Scene::Impl
{
    MetalScene* scene;
};




// ===========================================================================
metal::Buffer metal::Device::makeBuffer (const void* data, std::size_t size)
{
    auto device = MTLCreateSystemDefaultDevice();
    auto buffer = Buffer();
    buffer.impl->buffer = [device newBufferWithBytes:data
                                              length:size
                                             options:MTLResourceStorageModeShared];
    return buffer;
}

metal::Texture metal::Device::makeTexture1d (const uint32* data, std::size_t width)
{
    auto device = MTLCreateSystemDefaultDevice();
    auto d = [[MTLTextureDescriptor alloc] init];
    d.textureType = MTLTextureType1D;
    d.pixelFormat = MTLPixelFormatRGBA8Unorm;
    d.width = width;
    d.height = 1;
    d.depth = 1;
    d.usage = MTLTextureUsageShaderRead;

    MTLRegion region = {
        { 0, 0, 0 },
        { width, 1, 1 },
    };

    auto texture = Texture();
    texture.impl->texture = [device newTextureWithDescriptor:d];
    [texture.impl->texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:4 * width];

    return texture;
}




// ===========================================================================
metal::Buffer::Buffer()
{
    impl = std::make_shared<Buffer::Impl>();
}

bool metal::Buffer::empty()
{
    return impl->buffer == nil;
}




// ===========================================================================
metal::Texture::Texture()
{
    impl = std::make_shared<Texture::Impl>();
}

bool metal::Texture::empty()
{
    return impl->texture == nil;
}




// ===========================================================================
metal::Node::Node()
{
    impl = std::make_shared<Impl>();
    impl->node = [[MetalNode alloc] init];
}

void metal::Node::setVertexPositions (Buffer data)
{
    impl->node.vertexPositions = data.impl->buffer;
}

void metal::Node::setVertexColors (Buffer data)
{
    impl->node.vertexColors = data.impl->buffer;
}

void metal::Node::setVertexScalars (Buffer data)
{
    impl->node.vertexScalars = data.impl->buffer;
}

void metal::Node::setScalarMapping (Texture mapping)
{
    impl->node.scalarMapping = mapping.impl->texture;
}

void metal::Node::setScalarDomain (float lower, float upper)
{
    impl->node.scalarDomainLower = lower;
    impl->node.scalarDomainUpper = upper;
}

void metal::Node::setVertexCount (std::size_t numberOfVertices)
{
    impl->node.vertexCount = numberOfVertices;
}




// ===========================================================================
metal::Scene::Scene()
{
    impl = std::make_shared<Impl>();
    impl->scene = [[MetalScene alloc] init];
}

void metal::Scene::setDomain (std::array<float, 4> domain)
{
    impl->scene.domain = {domain[0], domain[1], domain[2], domain[3]};
}

void metal::Scene::addNode (Node node)
{
    [impl->scene addNode:node.impl->node];
}

void metal::Scene::clear()
{
    [impl->scene clear];
}




// ===========================================================================
struct metal::MetalComponent::Impl
{
    MetalViewController* controller;
};




// ===========================================================================
metal::MetalComponent::MetalComponent()
{
    impl = std::make_unique<Impl>();
    impl->controller = [[MetalViewController alloc] init];

    view.setView (impl->controller.view);
    view.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (view);
}

metal::MetalComponent::~MetalComponent()
{
    view.setView (nullptr);
}

void metal::MetalComponent::setScene (metal::Scene sceneToDisplay)
{
    impl->controller.scene = sceneToDisplay.impl->scene;
}

void metal::MetalComponent::resized()
{
    view.setBounds (getLocalBounds());
}

void metal::MetalComponent::paint (juce::Graphics& g)
{
}

void metal::MetalComponent::paintOverChildren (juce::Graphics& g)
{
}

void metal::MetalComponent::mouseDown (const juce::MouseEvent&)
{
}
