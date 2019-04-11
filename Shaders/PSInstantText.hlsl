#include "BaseHeader.hlsl"

Texture2D	FontTexture	: register(t0);

float4 main(VS_OUTPUT_TEXT input) : SV_TARGET
{
	float4 texel = FontTexture.Sample(CurrentSampler, input.TexCoord);
	return float4(input.Color.r, input.Color.g, input.Color.b, texel.a);
}