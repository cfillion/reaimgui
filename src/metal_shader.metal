#include <metal_stdlib>

using namespace metal;

struct Uniforms {
  float4x4 projectionMatrix;
};

struct VertexIn {
  float2 position  [[attribute(0)]];
  float2 texCoords [[attribute(1)]];
  uchar4 color     [[attribute(2)]];
};

struct VertexOut {
  float4 position [[position]];
  float2 texCoords;
  float4 color;
};

vertex VertexOut vertex_main(VertexIn in [[stage_in]],
  constant Uniforms &uniforms [[buffer(1)]])
{
  return {
    .position  = uniforms.projectionMatrix * float4(in.position, 0, 1),
    .texCoords = in.texCoords,
    .color     = float4(in.color) / float4(255.0),
  };
}

fragment half4 fragment_main(VertexOut in [[stage_in]],
  texture2d<half, access::sample> texture [[texture(0)]])
{
  // sampler parameters are documented at page 39
  // "Table 2.7. Sampler state enumeration values"
  // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
  constexpr sampler linearSampler {address::repeat, filter::linear};
  const half4 texColor = texture.sample(linearSampler, in.texCoords);
  return half4(in.color) * texColor;
}
