#pragma once
#include <memory>
#include <array>
#include "JuceHeader.h"




// ===========================================================================
namespace metal {
    class Buffer;
    class Device;
    class Node;
    class Scene;
    class MetalComponent;
    using namespace juce;
}




// ===========================================================================
class metal::Buffer
{
private:
    friend class Device;
    friend class Node;
    struct Impl;
    std::shared_ptr<Impl> impl;
};




// ===========================================================================
class metal::Device
{
public:
    static Buffer makeBuffer (const void* data, std::size_t size);
};




// ===========================================================================
class metal::Node
{
public:
    Node();
    void setVertexPositions (Buffer data);
    void setVertexColors (Buffer data);
    void setVertexCount (std::size_t numberOfVertices);
private:
    friend class Scene;
    struct Impl;
    std::shared_ptr<Impl> impl;
};




// ===========================================================================
class metal::Scene
{
public:
    Scene();
    void setDomain(std::array<float, 4> domain);
    void clear();
    void addNode (Node node);
private:
    friend class MetalComponent;
    struct Impl;
    std::shared_ptr<Impl> impl;
};



// ===========================================================================
class metal::MetalComponent : public juce::Component
{
public:
    MetalComponent();
    ~MetalComponent();
    void setScene (metal::Scene sceneToDisplay);

    // =======================================================================
    void resized() override;
    void paint (Graphics&) override;
    void paintOverChildren (Graphics&) override;
    void mouseDown (const MouseEvent&) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
    juce::NSViewComponent view;
};

