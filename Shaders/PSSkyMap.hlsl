#include "BaseHeader.hlsl"

TextureCube SkyMapTexture;
SamplerState CurrentSamplerState;

float4 main(SKY_MAP_OUTPUT input) : SV_Target
{
	return SkyMapTexture.Sample(CurrentSamplerState, input.TexCoord);
}