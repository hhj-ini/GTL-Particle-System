#pragma pack_matrix(row_major)


cbuffer constants : register(b0)
{
    matrix VP;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
};

Texture2D SpriteTexture : register(t0);
SamplerState SpriteSampler : register(s0);

struct VS_INPUT
{
    float3 LocalPos : POSITION;
    float2 UV : TEXCOORD0;
    float3 InstPosition : INST_POSITION;
    float InstRelTime : INST_RELTIME;
    float2 InstSize : INST_SIZE;
    float InstRotation : INST_ROTATION;
    float InstSubImage : INST_SUBIMAGE;
    float4 InstColor : INST_COLOR;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    float s, c;
    sincos(input.InstRotation, s, c);
    float2 p = input.LocalPos.yz * input.InstSize;
    float3 Local = float3(0, p.x * c - p.y * s, p.x * s + p.y * c);
    float3 WorldPos = mul(float4(Local, 0), World).xyz + input.InstPosition;
    
    PS_INPUT Out;
    Out.position = mul(float4(WorldPos, 1), VP);
    Out.uv = input.UV;
    Out.color = input.InstColor;
    return Out;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 TexColor = SpriteTexture.Sample(SpriteSampler, input.uv);
    float4 color = TexColor * input.color;
    
    clip(color.a - 0.001f);
    
    return color;
}
    