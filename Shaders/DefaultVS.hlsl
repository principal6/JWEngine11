#include "DefaultHeader.hlsl"

cbuffer cbPerObject
{
	float4x4 WVP;
	float4x4 World;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.Position = mul(input.Position, WVP);
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(mul(input.Normal, (float3x3)World));
	output.Diffuse = input.Diffuse;
	output.Specular = input.Specular;
	output.WorldPosition = mul(input.Position, World).xyz;

	return output;
}