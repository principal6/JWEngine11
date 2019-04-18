struct VS_INPUT_TEXT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float4 Color	: COLOR;
};

struct VS_INPUT_MODEL
{
	// Model data
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR0;
	float4 Specular	: COLOR1;

	// Rigging data
	uint4  BoneID	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;

	// Instance data
	float4 InstanceWorld0	: INST_WORLD0;
	float4 InstanceWorld1	: INST_WORLD1;
	float4 InstanceWorld2	: INST_WORLD2;
	float4 InstanceWorld3	: INST_WORLD3;
};

struct VS_OUTPUT_TEXT
{
	float4 Position	: SV_POSITION;
	float2 TexCoord	: TEXCOORD;
	float4 Color	: COLOR;
};

struct VS_OUTPUT_MODEL
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

SamplerState	CurrentSampler	: register(s0);
