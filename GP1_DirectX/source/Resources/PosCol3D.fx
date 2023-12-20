float4x4 g_WorldViewProjection : WorldViewProjection;
float4x4 g_MeshWorldMatrix : MeshWorldMatrix;
float3 g_CameraOrigin : CameraOrigin;


// Texture 3D for array maybe
Texture2DArray g_DiffuseMapArray : DiffuseMapArray;
Texture2DArray g_NormalMapArray : NormalMapArray;
Texture2DArray g_SpecularMapArray : SpecularMapArray;
Texture2DArray g_GlossMapArray : GlossMapArray;

SamplerState g_TextureSampler : Sampler;


float3 g_LightDirection : Light_Direction;


RasterizerState g_RasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};


// float g_PI;
// float g_LightIntensity;
// float g_Shininess;


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
    output.Normal = normalize(mul(input.Normal, (float3x3) g_MeshWorldMatrix));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) g_MeshWorldMatrix));
    
    // Get view direction based on world postion
    output.VertexToCamera = normalize(g_CameraOrigin - output.Position.xyz);
    
    // Get positon in view space
    output.Position = mul(output.Position, g_WorldViewProjection);
    
    
    output.Color = input.Color;
    output.TextureUV = input.TextureUV;
    output.MaterialIndex = input.MaterialIndex;
    

    return output;
}

float nrand(int random)
{
    return frac(sin(dot(random, float2(12.9898f, 78.233f))) * 43758.5453f);
}

// PIXEL SHADER (PX)
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    // Show material index
    //return float4(nrand(input.MaterialIndex), nrand(input.MaterialIndex + 1), nrand(input.MaterialIndex + 2), 1);
    
    return g_DiffuseMapArray.Sample(g_TextureSampler, float3(input.TextureUV, input.MaterialIndex));
    
    
    float4 sampleDiffuseColor = float4(1, 1, 1, 1); // Optional add material color (Not the same as vertex color)
    float4 ambientColor = float4(0.05f,0.05f,0.05f,1.0f);
    float sampledSpecular = 0.5f;
    float sampledPhongExponent = 20.0f;
    float diffuseStrengthKd = 3.0f;
    
    int materialIndex = input.MaterialIndex;
    
    sampleDiffuseColor *= g_DiffuseMapArray.Sample(g_TextureSampler, float3(input.TextureUV, 0));
    sampledPhongExponent *= g_GlossMapArray.Sample(g_TextureSampler, float3(input.TextureUV, materialIndex)).r;
    sampledSpecular *= g_SpecularMapArray.Sample(g_TextureSampler, float3(input.TextureUV, materialIndex)).r;
   
    // Transforms normal based on normal map
    float3 sampledNormalColor = g_NormalMapArray.Sample(g_TextureSampler, float3(input.TextureUV, materialIndex)).xyz;
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

