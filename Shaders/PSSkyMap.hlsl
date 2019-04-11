#include "BaseHeader.hlsl"

TextureCube	SkyMapTexture	: register(t0);

float4 main(SKY_MAP_OUTPUT input) : SV_Target
{
	return SkyMapTexture.Sample(CurrentSampler, input.TexCoord);
}