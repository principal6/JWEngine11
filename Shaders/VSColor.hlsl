#include "ColorHeader.hlsl"

cbuffer cbSpace : register(b0)
{
	float4x4 WVP;
	float4x4 World;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;

	output.Position = mul(input.Position, WVP);
	output.Color = input.Color;

	return output;
}