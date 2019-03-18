#include "DefaultHeader.hlsl"

cbuffer cbPerObject
{
	float4x4 WVP;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.Position = mul(input.Position, WVP);
	output.TexCoord = input.TexCoord;

	return output;
}