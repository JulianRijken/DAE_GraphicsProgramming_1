float4x4 g_WorldViewProjection : WorldViewProjection;
float4x4 g_MeshWorldMatrix : MeshWorldMatrix;

// Texture 3D for array maybe
Texture2D g_DiffuseMap : DiffuseMap;
Texture2D g_NormalMap : NormalMap;
Texture2D g_SpecularMap : SpecularMap;
Texture2D g_GlossMap : GlossMap;

//float3 g_LightDirection;

// float g_PI;
// float g_LightIntensity;
// float g_Shininess;

SamplerState g_TextureSampler : Sampler; // Manually in code set this up

//{
//    Filter = ANISOTROPIC;
//    AddressU = Wrap;
//    AddressV = Wrap;
//};


struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 TextureUV : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
    float2 TextureUV : TEXCOORD;
};

// VERTEX SHADER (VS)
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    output.Position = mul(float4(input.Position, 1.f), mul(g_MeshWorldMatrix, g_WorldViewProjection));
    output.Color = input.Color;
    output.TextureUV = input.TextureUV;
    return output;
}

// PIXEL SHADER (PX)
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    
    float4 diffuseColor = g_DiffuseMap.Sample(g_TextureSampler, input.TextureUV);
    
    
    
    return diffuseColor;
}



technique11 DefaultTechnique
{
    pass P0
    {
        SetVertexShader( CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader(ps_5_0, PS()));
    }
}

