cbuffer AllInfo : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projMatrix;

    float4 DirectionLight;
    float4 LightColor;
    float4 ViewPos;
    float4 KaSpecPowKsX;

    float4 Ka;
    float4 Kd;
    float4 KsX;
    
    float4x4 InverseTransposeWorldMatrix;
    
    float time;
    
    float4x4 lightViewMatrix;
    float4x4 lightProjMatrix;
};

struct VS_Input
{
    float4 pos : POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
};

struct PS_Input
{
    float4 pos : SV_POSITION;
    float4 depth_pos : TEXTURE0;
};

PS_Input VS_Shadow(VS_Input input)
{
    PS_Input output;

    output.pos = mul(mul(float4(input.pos.rgb, 1), worldMatrix), lightViewMatrix);
    output.pos = mul(output.pos, lightProjMatrix);
    output.depth_pos = output.pos;
    return output;
}

float4 PS_Shadow(PS_Input input) : SV_TARGET
{
    float depthValue = input.depth_pos.z / input.depth_pos.w;
    return float4(depthValue, depthValue, depthValue, 1.0f);
}