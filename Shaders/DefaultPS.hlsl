#include "DefaultHeader.hlsl"

cbuffer cbPS
{
	bool HasTexture;
	float3 pad;
};

Texture2D CurrentTexture;
SamplerState CurrentSamplerState;

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 model_diffuse = input.Diffuse;

	if (HasTexture == true)
	{
		model_diffuse = CurrentTexture.Sample(CurrentSamplerState, input.TexCoord);

		// clip if alpha is less than 0.1
		clip(model_diffuse.a - 0.1);
	}
	
	return model_diffuse;
}