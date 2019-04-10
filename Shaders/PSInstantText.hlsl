#include "BaseHeader.hlsl"

cbuffer cbColor
{
	float4 FontColor;
};

Texture2D CurrentTexture;
SamplerState CurrentSamplerState;

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 texel = CurrentTexture.Sample(CurrentSamplerState, input.TexCoord);
	return float4(input.Diffuse.r, input.Diffuse.g, input.Diffuse.b, texel.a);
}