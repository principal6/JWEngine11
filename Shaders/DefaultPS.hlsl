#include "DefaultHeader.hlsl"

Texture2D CurrentTexture;
SamplerState CurrentSamplerState;

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 texel = CurrentTexture.Sample(CurrentSamplerState, input.TexCoord);

	// clip if alpha is less than 0.1
	clip(texel.a - 0.1);

	return texel;
}