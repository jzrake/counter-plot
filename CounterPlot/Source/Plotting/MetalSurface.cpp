#include "MetalSurface.hpp"




//=============================================================================
MetalRenderingSurface::MetalRenderingSurface()
{
    // DBG("MetalRenderingSurface::MetalRenderingSurface()");
    setInterceptsMouseClicks (false, false);
    addAndMakeVisible (metal);
}

MetalRenderingSurface::~MetalRenderingSurface()
{
    // DBG("MetalRenderingSurface::~MetalRenderingSurface()");
}

void MetalRenderingSurface::setContent (std::vector<std::shared_ptr<PlotArtist>> artists, const PlotTransformer& trans)
{
    scene.clear();
    scene.setDomain (trans.getDomain());

    for (auto artist : artists)
    {
        artist->render (*this);
    }
    metal.setScene (scene);
}

void MetalRenderingSurface::renderTriangles (const std::vector<simd::float2>& vertices,
                                             const std::vector<simd::float4>& colors)
{
    assert(vertices.size() == colors.size());
    auto node = metal::Node();
    node.setVertexPositions (getOrCreateBuffer (vertices));
    node.setVertexColors (getOrCreateBuffer (colors));
    node.setVertexCount (vertices.size());
    scene.addNode (node);
}

void MetalRenderingSurface::renderTriangles (const std::vector<simd::float2>& vertices,
                                             const std::vector<simd::float1>& scalars,
                                             const ScalarMapping& mapping)
{
    assert(vertices.size() == scalars.size());
    auto data = ColourMapHelpers::fromColours (mapping.stops);
    auto texture = metal::Device::makeTexture1d (data.data(), data.size());
    auto node = metal::Node();

    node.setVertexPositions (getOrCreateBuffer (vertices));
    node.setVertexScalars (getOrCreateBuffer (scalars));
    node.setScalarMapping (texture);
    node.setScalarDomain (mapping.vmin, mapping.vmax);
    node.setVertexCount (vertices.size());
    scene.addNode (node);
}

void MetalRenderingSurface::renderTriangles (DeviceBufferFloat2 vertices, DeviceBufferFloat4 colors)
{
    assert(vertices.size == colors.size);
    auto node = metal::Node();

    node.setVertexPositions (vertices.metal);
    node.setVertexColors (colors.metal);
    node.setVertexCount (vertices.size);

    scene.addNode (node);
}

void MetalRenderingSurface::renderTriangles (DeviceBufferFloat2 vertices, DeviceBufferFloat1 scalars, const ScalarMapping& mapping)
{
    assert(vertices.size == scalars.size);
    auto data = ColourMapHelpers::fromColours (mapping.stops);
    auto texture = metal::Device::makeTexture1d (data.data(), data.size());
    auto node = metal::Node();

    node.setVertexPositions (vertices.metal);
    node.setVertexScalars (scalars.metal);
    node.setScalarMapping (texture);
    node.setScalarDomain (mapping.vmin, mapping.vmax);
    node.setVertexCount (vertices.size);

    scene.addNode (node);
}

Image MetalRenderingSurface::createSnapshot() const
{
    return metal.createSnapshot();
}




//=============================================================================
void MetalRenderingSurface::resized()
{
    metal.setBounds (getLocalBounds());
}




//=============================================================================
metal::Buffer MetalRenderingSurface::getOrCreateBuffer (const std::vector<simd::float1>& data)
{
    if (data.empty())
    {
        return metal::Buffer();
    }
    if (cachedBuffers1.count (&data))
    {
        return cachedBuffers1.at (&data);
    }
    auto newBuffer = metal::Device::makeBuffer (data.data(), data.size() * sizeof (simd::float1));
    cleanBufferCaches();
    cachedBuffers1[&data] = newBuffer;
    return newBuffer;
}

metal::Buffer MetalRenderingSurface::getOrCreateBuffer (const std::vector<simd::float2>& data)
{
    if (data.empty())
    {
        return metal::Buffer();
    }
    if (cachedBuffers2.count (&data))
    {
        return cachedBuffers2.at (&data);
    }
    auto newBuffer = metal::Device::makeBuffer (data.data(), data.size() * sizeof (simd::float2));
    cleanBufferCaches();
    cachedBuffers2[&data] = newBuffer;
    return newBuffer;
}

metal::Buffer MetalRenderingSurface::getOrCreateBuffer (const std::vector<simd::float4>& data)
{
    if (data.empty())
    {
        return metal::Buffer();
    }
    if (cachedBuffers4.count (&data))
    {
        return cachedBuffers4.at (&data);
    }
    auto newBuffer = metal::Device::makeBuffer (data.data(), data.size() * sizeof (simd::float4));
    cleanBufferCaches();
    cachedBuffers4[&data] = newBuffer;
    return newBuffer;
}

void MetalRenderingSurface::cleanBufferCaches()
{
    if (cachedBuffers1.size() >= maxBuffersInCache)
    {
        cachedBuffers1.clear();
    }
    if (cachedBuffers2.size() >= maxBuffersInCache)
    {
        cachedBuffers2.clear();
    }
    if (cachedBuffers4.size() >= maxBuffersInCache)
    {
        cachedBuffers4.clear();
    }
}




//=============================================================================
DeviceBufferFloat1::DeviceBufferFloat1 (const std::vector<simd::float1>& data)
{
    size = data.size();
    metal = metal::Device::makeBuffer (data.data(), size * sizeof (simd::float1));
}

DeviceBufferFloat2::DeviceBufferFloat2 (const std::vector<simd::float2>& data)
{
    size = data.size();
    metal = metal::Device::makeBuffer (data.data(), size * sizeof (simd::float2));
}

DeviceBufferFloat4::DeviceBufferFloat4 (const std::vector<simd::float4>& data)
{
    size = data.size();
    metal = metal::Device::makeBuffer (data.data(), size * sizeof (simd::float4));
}
