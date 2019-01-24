#pragma once
#include "JuceHeader.h"
#include "PlotModels.hpp"




//=============================================================================
class MetalRenderingSurface : public RenderingSurface
{
public:

    //=========================================================================
    MetalRenderingSurface();
    ~MetalRenderingSurface();

    //=========================================================================
    void setContent (std::vector<std::shared_ptr<PlotArtist>> artists, const PlotTransformer& trans) override;
    void renderTriangles (DeviceBufferFloat2 vertices, DeviceBufferFloat4 colors) override;
    void renderTriangles (DeviceBufferFloat2 vertices, DeviceBufferFloat1 scalars, const ScalarMapping& mapping) override;
    Image createSnapshot() const override;

    //=========================================================================
    void resized() override;

private:
    //=========================================================================
    metal::Scene scene;
    metal::MetalComponent metal;
};
