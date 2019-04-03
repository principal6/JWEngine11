struct VS_INPUT_STATIC
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR0;
	float4 Specular	: COLOR1;
};

struct VS_INPUT_SKINNED
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR0;
	float4 Specular	: COLOR1;
	uint4  BoneID	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
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

struct SKY_MAP_OUTPUT
{
	float4 Position	: SV_POSITION;
	float3 TexCoord	: TEXCOORD;
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