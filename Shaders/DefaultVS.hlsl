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
	output.Normal = mul(input.Normal, World);

	return output;
}