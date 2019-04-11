#include "BaseHeader.hlsl"

Texture2D	FontTexture	: register(t0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 texel = FontTexture.Sample(CurrentSampler, input.TexCoord);
	return float4(input.Diffuse.r, input.Diffuse.g, input.Diffuse.b, texel.a);
}