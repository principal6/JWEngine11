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
	return float4(FontColor.r, FontColor.g, FontColor.b, texel.a);
}