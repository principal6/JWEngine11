#include "BaseHeader.hlsl"

cbuffer cbSpace : register(b0)
{
	float4x4 WVP;
	float4x4 World;
};

VS_OUTPUT main(VS_INPUT_STATIC input)
{
	VS_OUTPUT output;

	output.Position = mul(input.Position, WVP);
	output.WorldPosition = mul(input.Position, World).xyz;
	output.Normal = normalize(mul(input.Normal, (float3x3)World));
	
	output.TexCoord = input.TexCoord;
	output.Diffuse = input.Diffuse;
	output.Specular = input.Specular;
	
	return output;
}