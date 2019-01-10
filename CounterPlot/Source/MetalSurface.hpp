#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"




//=============================================================================
class MetalRenderingSurface : public RenderingSurface
{
public:

    //=========================================================================
    MetalRenderingSurface();
    void setContent (std::vector<std::shared_ptr<PlotArtist>> artists,
                     const PlotTransformer& trans) override;
    void renderTriangles (const std::vector<simd::float2>& vertices,
                          const std::vector<simd::float4>& colors) override;
    void renderTriangles (const std::vector<simd::float2>& vertices,
                          const std::vector<simd::float1>& scalars,
                          const ScalarMapping& mapping) override;
    Image createSnapshot() const override;

    //=========================================================================
    void resized() override;

private:
    //=========================================================================
    metal::Buffer getOrCreateBuffer (const std::vector<simd::float1>& data);
    metal::Buffer getOrCreateBuffer (const std::vector<simd::float2>& data);
    metal::Buffer getOrCreateBuffer (const std::vector<simd::float4>& data);

//    std::map<const std::vector<simd::float1>*, metal::Buffer> cachedBuffers1;
//    std::map<const std::vector<simd::float2>*, metal::Buffer> cachedBuffers2;
//    std::map<const std::vector<simd::float4>*, metal::Buffer> cachedBuffers4;
    std::map<const simd::float1*, metal::Buffer> cachedBuffers1;
    std::map<const simd::float2*, metal::Buffer> cachedBuffers2;
    std::map<const simd::float4*, metal::Buffer> cachedBuffers4;

    void cleanBufferCaches();

    metal::Texture colormap;
    metal::Scene scene;
    metal::MetalComponent metal;

    int maxBuffersInCache = 3;
};
