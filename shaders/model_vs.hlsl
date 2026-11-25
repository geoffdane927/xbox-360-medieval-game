float4x4 ViewProjection : register(c0);

struct VS_IN {
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};
struct VS_OUT {
    float4 position : POSITION0;
    float3 normal : TEXCOORD0;
    float2 uv : TEXCOORD1;
};
VS_OUT main(VS_IN input) {
    VS_OUT output;
    output.position = mul(ViewProjection, float4(input.position, 1.0f));
    output.normal = normalize(input.normal);
    output.uv = input.uv;
    return output;
}
