#include <metal_stdlib>
#include <simd/simd.h>




// ============================================================================
typedef struct
{
    vector_float4 position [[position]];
    vector_float4 color;
} RasterizerData;




// ============================================================================
constexpr metal::sampler colorMapSampler (metal::mag_filter::linear,
                                          metal::min_filter::linear);




// ============================================================================
vertex RasterizerData
vertexFunctionLiteralColors(unsigned int                     vertexID        [[ vertex_id  ]],
                            device const vector_float2      *vertexPositions [[ buffer (0) ]],
                            device const vector_float4      *vertexColors    [[ buffer (1) ]],
                            device const vector_float4      &domain          [[ buffer (2) ]])
{
    RasterizerData out;
    out.position.x = -1.f + 2.f * (vertexPositions[vertexID].x - domain[0]) / (domain[1] - domain[0]);
    out.position.y = -1.f + 2.f * (vertexPositions[vertexID].y - domain[2]) / (domain[3] - domain[2]);
    out.position.zw = vector_float2(0, 1);
    out.color = vertexColors[vertexID];
    return out;
}




// ============================================================================
vertex RasterizerData
vertexFunctionScalarMapping(unsigned int                    vertexID         [[ vertex_id    ]],
                            device const vector_float2     *vertexPositions  [[ buffer  (0)  ]],
                            device const float             *vertexScalars    [[ buffer  (1)  ]],
                            device const vector_float4     &domain           [[ buffer  (2)  ]],
                            device const vector_float2     &scalarDomain     [[ buffer  (3)  ]],
                            metal::texture1d<float>         colormap         [[ texture (4)  ]])
{
    float s = (vertexScalars[vertexID] - scalarDomain[0]) / (scalarDomain[1] - scalarDomain[0]);
    RasterizerData out;
    out.position.x = -1.f + 2.f * (vertexPositions[vertexID].x - domain[0]) / (domain[1] - domain[0]);
    out.position.y = -1.f + 2.f * (vertexPositions[vertexID].y - domain[2]) / (domain[3] - domain[2]);
    out.position.zw = vector_float2(0, 1);
    out.color = colormap.sample(colorMapSampler, s);
    return out;
}




// ============================================================================
fragment vector_float4
fragmentFunction(RasterizerData in [[stage_in]])
{
    return in.color;
};
