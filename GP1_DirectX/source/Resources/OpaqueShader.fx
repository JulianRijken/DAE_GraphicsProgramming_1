
float4x4 g_WorldViewProjection : WorldViewProjection;
float4x4 g_MeshWorldMatrix : MeshWorldMatrix;
float3 g_CameraOrigin : CameraOrigin;
float3 g_LightDirection : Light_Direction;
bool g_UseNormalMap : UseNormalMap;

Texture2D g_DiffuseMap : DiffuseMap;
Texture2D g_NormalMap : NormalMap;
Texture2D g_SpecularMap : SpecularMap;
Texture2D g_GlossMap : GlossMap;

float g_DiffuseStrengthKd;
float g_SampledPhongExponent;
float g_SpecularKs;

float3 g_AmbientColor;

struct Light
{
    float Intensity;
    float3 Origin;
    float3 Direction;
    float4 Color;
    //uint Type;
};

cbuffer LightBuffer : register(b1)
{
    Light lights[4]; // Limit to max 10 lights
};


SamplerState g_TextureSampler : Sampler;

RasterizerState g_RasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

BlendState g_BlendState
{
// Always use default
};

DepthStencilState g_DepthStencilState
{
// Always use default
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
    float3 PositionWorld : POSITION_WORLD;
};

// VERTEX SHADER (VS)
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    
    // Transform output to world space (Using mesh matrix)
    output.Position = mul(float4(input.Position, 1.f), g_MeshWorldMatrix);
    output.PositionWorld = output.Position.rgb;

    // We still normalize here to avoid errors in the input
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


// PIXEL SHADER (PX)
float4 PS(VS_OUTPUT input) : SV_TARGET
{

    float4 ambientColor = float4(g_AmbientColor, 1.0f);    
    float4 finalColor = ambientColor;


    for (int i = 0; i < 4; ++i) // Use the same array size as in the LightBuffer
    {

        //!!! TODO: lightDirection and lightRadiance only work for directional!!! this is not finished!!!

        float3 lightDirection = float3(0,0,0);
        //if (i == 0)
        //{
			lightDirection = lights[i].Direction; // Directional
        //}
		//else
        //{
			//lightDirection = normalize(input.PositionWorld - lights[i].Origin); // Point
        //}



        float4 lightRadiance = float4(0,0,0,1);
       // if (i == 0)
        //{
			lightRadiance = lights[i].Color * lights[i].Intensity; // Directional
        //}
		//else
        //{
           // float distance = length(lights[i].Origin - input.PositionWorld);
		    //lightRadiance = lights[i].Color * (lights[i].Intensity * distance * distance); // Point
        //}



        float4 sampleDiffuseColor = float4(1, 1, 1, 1); // Optional add material color (Not the same as vertex color)
        float sampledSpecular = g_SpecularKs;
        float sampledPhongExponent = g_SampledPhongExponent;
        float diffuseStrengthKd = g_DiffuseStrengthKd;
        
        // HINT: Does not check if texture is null
        sampledPhongExponent *= g_GlossMap.Sample(g_TextureSampler, input.TextureUV).r;
        sampledSpecular *= g_SpecularMap.Sample(g_TextureSampler, input.TextureUV).r;
        sampleDiffuseColor *= g_DiffuseMap.Sample(g_TextureSampler, input.TextureUV);
    

        float3 sampledNormal = input.Normal;
        if (g_UseNormalMap)  // Transforms normal based on normal map
        {
            float3 sampledNormalColor = g_NormalMap.Sample(g_TextureSampler, input.TextureUV).xyz;
            float3x3 tbnMatrix = float3x3(input.Tangent, cross(input.Normal, input.Tangent), input.Normal);
            sampledNormal = mul(sampledNormalColor * 2.0f - 1.0f, tbnMatrix);
        }
        
        sampledNormal = normalize(sampledNormal);
        
        // Get lambert diffuse
        float4 lambertDiffuse = sampleDiffuseColor * diffuseStrengthKd / 3.16f;
        
        // Get Cosine Law
        float observedArea = max(0.0f, dot(sampledNormal, -lightDirection));
        
        // Get Specular Intensity
        float3 reflectedRay = reflect(lightDirection, sampledNormal);
        float cosAlpha = max(0.0f, dot(reflectedRay, input.VertexToCamera));
        float specularIntensity = sampledSpecular * pow(cosAlpha, sampledPhongExponent);
        float4 specularColor = specularIntensity * float4(1, 1, 1, 1);
        

        finalColor += saturate(lightRadiance * (specularColor + lambertDiffuse) * observedArea);
    }
    
    return saturate(finalColor);
}



technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(g_RasterizerState);
        SetBlendState(g_BlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetDepthStencilState(g_DepthStencilState, 0);

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}

