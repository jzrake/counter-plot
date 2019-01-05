#include "MetalSurface.hpp"




//=============================================================================
MetalRenderingSurface::MetalRenderingSurface()
{
    setColorMap (0);
    setInterceptsMouseClicks (false, false);
    addAndMakeVisible (metal);
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
                                             float vmin, float vmax)
{
    assert(vertices.size() == scalars.size());
    auto node = metal::Node();
    node.setVertexPositions (getOrCreateBuffer (vertices));
    node.setVertexScalars (getOrCreateBuffer (scalars));
    node.setScalarMapping (colormap);
    node.setScalarDomain (vmin, vmax);
    node.setVertexCount (vertices.size());
    scene.addNode (node);
}

void MetalRenderingSurface::nextColorMap()
{
    setColorMap ((colorMapIndex + 1) % 8);
}

void MetalRenderingSurface::prevColorMap()
{
    setColorMap ((colorMapIndex - 1 + 8) % 8);
}

void MetalRenderingSurface::setColorMap (int index)
{
    std::vector<uint32> data;

    switch (colorMapIndex = index)
    {
        case 0: data = ColourmapHelpers::textureFromRGBTable (BinaryData::cividis_cmap); break;
        case 1: data = ColourmapHelpers::textureFromRGBTable (BinaryData::dawn_cmap); break;
        case 2: data = ColourmapHelpers::textureFromRGBTable (BinaryData::fire_cmap); break;
        case 3: data = ColourmapHelpers::textureFromRGBTable (BinaryData::inferno_cmap); break;
        case 4: data = ColourmapHelpers::textureFromRGBTable (BinaryData::magma_cmap); break;
        case 5: data = ColourmapHelpers::textureFromRGBTable (BinaryData::plasma_cmap); break;
        case 6: data = ColourmapHelpers::textureFromRGBTable (BinaryData::seashore_cmap); break;
        case 7: data = ColourmapHelpers::textureFromRGBTable (BinaryData::viridis_cmap); break;
    }
    colormap = metal::Device::makeTexture1d (data.data(), data.size());
}




//=============================================================================
void MetalRenderingSurface::resized()
{
    metal.setBounds (getLocalBounds());
}




//=============================================================================
metal::Buffer MetalRenderingSurface::getOrCreateBuffer (const std::vector<simd::float1>& data)
{
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
