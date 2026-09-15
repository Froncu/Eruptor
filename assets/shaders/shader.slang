struct VertexInput
{
    float3 position;
    float3 color;
    float2 textureCoordinate;
};

struct FragmentInput
{
    float4 position : SV_Position;
    float3 color;
    float2 textureCoordinate;
};

struct UniformBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};
[[vk::binding(0, 0)]] ConstantBuffer<UniformBuffer> ubo;

[shader("vertex")]
FragmentInput vertMain(VertexInput input)
{
    FragmentInput output;
    output.position = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(input.position, 1.0))));
    output.color = input.color;
    output.textureCoordinate = input.textureCoordinate;
    return output;
}

[[vk::binding(0, 1)]] SamplerState sampler;
[[vk::binding(1, 1)]] Texture2D texture;

[shader("fragment")]
float4 fragMain(FragmentInput input) : SV_TARGET
{
    return texture.Sample(sampler, input.textureCoordinate);
}