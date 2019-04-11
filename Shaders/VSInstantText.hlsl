#include "BaseHeader.hlsl"

cbuffer cbSpace : register(b0)
{
	float4x4 WVP;
	float4x4 World;
};

VS_OUTPUT_TEXT main(VS_INPUT_TEXT input)
{
	VS_OUTPUT_TEXT output;

	output.Position	= mul(input.Position, WVP);
	output.TexCoord	= input.TexCoord;
	output.Color	= input.Color;

	return output;
}