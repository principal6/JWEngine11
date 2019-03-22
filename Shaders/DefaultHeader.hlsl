struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR0;
	float4 Specular	: COLOR1;
};

struct VS_OUTPUT
{
	float4 Position			: SV_POSITION;
	float2 TexCoord			: TEXCOORD;
	float3 Normal			: NORMAL0;
	float4 Diffuse			: COLOR0;
	float4 Specular			: COLOR1;
	float3 WorldPosition	: COLOR2;
};

struct SPointlight
{
	float3 LightColor;
	float Intensity;
	float3 Position;
	float Range;
	float3 Attenuation;
	float pad;
};