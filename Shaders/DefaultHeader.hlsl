struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal	: NORMAL;
	float4 Diffuse	: COLOR;
};