float4x4 g_WorldViewProjection : WorldViewProjection;
float4x4 g_MeshWorldMatrix : MeshWorldMatrix;
Texture2D g_DiffuseMap : DiffuseMap;

SamplerState TextureSamplerAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};


SamplerState TextureSamplerPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState TextureSamplerLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};



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
float4 PSA(VS_OUTPUT input) : SV_TARGET
{
    return g_DiffuseMap.Sample(TextureSamplerAnisotropic, input.TextureUV);
}

float4 PSL(VS_OUTPUT input) : SV_TARGET
{
    return g_DiffuseMap.Sample(TextureSamplerLinear, input.TextureUV);
}

float4 PSP(VS_OUTPUT input) : SV_TARGET
{
    return g_DiffuseMap.Sample(TextureSamplerPoint, input.TextureUV);
}



technique11 DefaultTechnique
{
    pass P0
    {
        SetVertexShader( CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader( CompileShader(ps_5_0, PSA()));
    }
}

technique11 PointFiltering
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSP()));
    }
}

technique11 LinearFiltering
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PSL()));
    }
}

