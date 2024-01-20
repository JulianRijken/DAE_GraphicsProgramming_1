
float4x4 g_WorldViewProjection : WorldViewProjection;
float4x4 g_MeshWorldMatrix : MeshWorldMatrix;
float3 g_CameraOrigin : CameraOrigin;
float3 g_LightDirection : Light_Direction;

Texture2D g_DiffuseMap : DiffuseMap;

SamplerState g_TextureSampler : Sampler;

RasterizerState g_RasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false;
};

BlendState g_BlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState g_DepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;

    ////others are redundant because
    //// StencilEnable is FALSE
    ////(for demo purposes only)
    //StencilReadMask = 0x0F;
    //StencilWriteMask = 0x0F;

    //FrontFaceStencilFunc = always;
    //BackFaceStencilFunc = always;

    //FrontFaceStencilDepthFail = keep;
    //BackFaceStencilDepthFail = keep;

    //FrontFaceStencilPass = keep;
    //BackFaceStencilPass = keep;

    //FrontFaceStencilFail = keep;
    //BackFaceStencilFail = keep;
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
    float4 sampleDiffuseColor = g_DiffuseMap.Sample(g_TextureSampler, input.TextureUV);
    
    return sampleDiffuseColor;
}



technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(g_RasterizerState);
        SetBlendState(g_BlendState, float4(0.0f,0.0f,0.0f,0.0f), 0xFFFFFFFF);
        SetDepthStencilState(g_DepthStencilState,0);

        SetVertexShader( CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader(ps_5_0, PS()));
    }
}

