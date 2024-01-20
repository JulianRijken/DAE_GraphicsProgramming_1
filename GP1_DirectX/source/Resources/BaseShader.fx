
float4x4 g_WorldViewProjection : WorldViewProjection;
float4x4 g_MeshWorldMatrix : MeshWorldMatrix;
float3 g_CameraOrigin : CameraOrigin;
float3 g_LightDirection : Light_Direction;

Texture2D g_DiffuseMap : DiffuseMap;
Texture2D g_NormalMap : NormalMap;
Texture2D g_SpecularMap : SpecularMap;
Texture2D g_GlossMap : GlossMap;

SamplerState g_TextureSampler : Sampler;

RasterizerState g_RasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 TextureUV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    uint MaterialIndex : MATERIAL_INDEX;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
    float2 TextureUV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    uint MaterialIndex : MATERIAL_INDEX;
    float3 VertexToCamera : VERTEX_TO_CAMERA;
};

// VERTEX SHADER (VS)
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    // Transform output to world space (Using mesh matrix)
    output.Position = mul(float4(input.Position, 1.f), g_MeshWorldMatrix);
    output.Normal = mul(input.Normal, (float3x3) g_MeshWorldMatrix);
    output.Tangent = mul(input.Tangent, (float3x3) g_MeshWorldMatrix);
    
    // Get view direction based on world postion
    output.VertexToCamera = normalize(g_CameraOrigin - output.Position.xyz);
    
    // Get positon in view space
    output.Position = mul(output.Position, g_WorldViewProjection);
    
    output.Color = input.Color;
    output.TextureUV = input.TextureUV;
    output.MaterialIndex = input.MaterialIndex;
    
    return output;
}


// PIXEL SHADER (PX)
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    
    float4 sampleDiffuseColor = float4(1, 1, 1, 1); // Optional add material color (Not the same as vertex color)
    float4 ambientColor = float4(0.03f,0.03f,0.03f,1.0f);
    float sampledSpecular = 1.0f;
    float sampledPhongExponent = 25.0f;
    float diffuseStrengthKd = 7.0f;
    
    sampledPhongExponent *= g_GlossMap.Sample(g_TextureSampler, input.TextureUV).r;
    sampledSpecular *= g_SpecularMap.Sample(g_TextureSampler, input.TextureUV).r;
    sampleDiffuseColor *= g_DiffuseMap.Sample(g_TextureSampler, input.TextureUV);
   

    float3 sampledNormalColor = g_NormalMap.Sample(g_TextureSampler, input.TextureUV).xyz;
    float3x3 tbnMatrix = float3x3(input.Tangent, cross(input.Normal, input.Tangent), input.Normal);
    float3 sampledNormal = mul(sampledNormalColor * 2.0f - 1.0f, tbnMatrix);
    
    // Get lambert diffuse
    float4 lambertDiffuse = sampleDiffuseColor * diffuseStrengthKd / 3.16f;
    
    // Get Cosine Law
    float observedArea = max(0.0f, dot(sampledNormal, -g_LightDirection));
    
    // Get Specular Intensity
    float3 reflectedRay = reflect(g_LightDirection, sampledNormal);
    float cosAlpha = max(0.0f, dot(reflectedRay, input.VertexToCamera));
    float specularIntensity = sampledSpecular * pow(cosAlpha, sampledPhongExponent);
    float4 specularColor = specularIntensity * float4(1, 1, 1, 1);
    
    return saturate((specularColor + lambertDiffuse) * observedArea + ambientColor);
}



technique11 DefaultTechnique
{
    pass P0
    {
        SetVertexShader( CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader(ps_5_0, PS()));
        SetRasterizerState(g_RasterizerState);
    }
}

