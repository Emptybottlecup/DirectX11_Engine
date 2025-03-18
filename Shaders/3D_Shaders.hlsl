Texture2D ObjTexture;
SamplerState ObjSamplerState;

Texture2D ShadowMap;
SamplerState ShadowSamplerState;

Texture2D NewTexture;

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
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float4 WorldPos : TEXCOORD1;
    float4 pos_in_light_view : TEXCOORD3;
};

PS_Input VS_Main(VS_Input vertex, uint id : SV_VertexID)
{
    PS_Input vsOut = (PS_Input) 0;
   
    vsOut.pos = mul(vertex.pos, worldMatrix);
    vsOut.pos = mul(vsOut.pos, viewMatrix);
    vsOut.pos = mul(vsOut.pos, projMatrix);

    vsOut.TexCoord = vertex.TexCoord;
    vsOut.WorldPos = mul(vertex.pos, worldMatrix);
    vsOut.Normal = mul(float4(vertex.Normal, 0.0f), InverseTransposeWorldMatrix);
    
    vsOut.pos_in_light_view = mul(float4(vsOut.WorldPos.rgb, 1), lightViewMatrix);
    vsOut.pos_in_light_view = mul(float4(vsOut.pos_in_light_view.rgb, 1), lightProjMatrix);
    return vsOut;
}

float4 PS_Main(PS_Input input) : SV_TARGET
{    
    const float bias = 5e-6f;
    
    float4 textureColor = ObjTexture.Sample(ObjSamplerState, input.TexCoord);
   
    float3 normal = normalize(input.Normal);
    
    float3 lightDir = normalize(-DirectionLight.xyz);
    float3 viewDir = normalize(ViewPos.xyz - input.WorldPos.xyz);
    float3 refVec = normalize(reflect(lightDir, normal));

    float3 ambient = KaSpecPowKsX.x * textureColor;
    float3 diffuse = max(0, dot(lightDir, normal)) * textureColor;
    float3 specular = pow(max(0, dot(-viewDir, refVec)), KaSpecPowKsX.y) * KsX;

    float2 projected_tex_coord;
    projected_tex_coord.x = input.pos_in_light_view.x / input.pos_in_light_view.w / 2.0f + 0.5f;
    projected_tex_coord.y = -input.pos_in_light_view.y / input.pos_in_light_view.w / 2.0f + 0.5f;
    
    float shadowFactor = 1.0f;
    
    if ((saturate(projected_tex_coord.x) == projected_tex_coord.x) &&
        (saturate(projected_tex_coord.y) == projected_tex_coord.y))
    {
        //Depth Comparison
        const float depth = ShadowMap.Sample(ShadowSamplerState, projected_tex_coord).r;
        const float light_depth = input.pos_in_light_view.z / input.pos_in_light_view.w - bias;

        float superDuperDepth = NewTexture.Sample(ObjSamplerState, projected_tex_coord).r;
        
        
        if (light_depth < depth)
        {
            shadowFactor = 1.0f;
        }
        else
        {
            shadowFactor = 1.0f * superDuperDepth;
        }
    
    }
    float4 finalColor = float4(ambient + (shadowFactor) * (diffuse + specular) * LightColor.rgb,1.0f) * textureColor;
    
    return finalColor;
}