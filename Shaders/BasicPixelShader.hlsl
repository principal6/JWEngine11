#include "ShaderHeader.hlsl"

float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float4 texel = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	// clip if alpha is less than 0.1
	clip(texel.a - 0.1);

	return ObjTexture.Sample(ObjSamplerState, input.TexCoord);
}