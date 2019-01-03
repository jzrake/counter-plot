#include "MetalComponent.hpp"
#include "MetalRendering.h"




// ===========================================================================
struct metal::Buffer::Impl
{
    id<MTLBuffer> buffer;
};




// ===========================================================================
struct metal::Node::Impl
{
    MetalNode* node;
};




// ===========================================================================
struct metal::Scene::Impl
{
    MetalScene* scene;
};




// ===========================================================================
metal::Buffer metal::Device::makeBuffer (const void* data, std::size_t size)
{
    auto device = MTLCreateSystemDefaultDevice();
    auto buffer = Buffer();
    buffer.impl = std::make_shared<Buffer::Impl>();
    buffer.impl->buffer = [device newBufferWithBytes:data
                                              length:size
                                             options:MTLResourceStorageModeShared];
    return buffer;
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
