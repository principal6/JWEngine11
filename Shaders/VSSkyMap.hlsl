#include "BaseHeader.hlsl"

cbuffer cbWorld
{
	float4x4 WVP;
	float4x4 World;
};

SKY_MAP_OUTPUT main(VS_INPUT_MODEL input)
{
	SKY_MAP_OUTPUT output;

	// Use w intead of z (for z to be always 1.0)
	output.Position = mul(input.Position, WVP).xyww;

	// The sphere is centered(0.0, 0.0, 0.0), so the TexCoord is just the vertex position.
	output.TexCoord = input.Position.xyz;

	return output;
}