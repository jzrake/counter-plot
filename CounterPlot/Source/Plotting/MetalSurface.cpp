#include "MetalSurface.hpp"




//=============================================================================
MetalRenderingSurface::MetalRenderingSurface()
{
    setInterceptsMouseClicks (false, false);
    addAndMakeVisible (metal);
}

MetalRenderingSurface::~MetalRenderingSurface()
{
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
