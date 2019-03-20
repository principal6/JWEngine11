#include "DefaultHeader.hlsl"

cbuffer cbPS
{
	bool HasTexture;
	float3 Color;
};

Texture2D CurrentTexture;
SamplerState CurrentSamplerState;

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 model_diffuse = float4(Color.r, Color.g, Color.b, 1);

	if (HasTexture == true)
	{
		model_diffuse = CurrentTexture.Sample(CurrentSamplerState, input.TexCoord);

		// clip if alpha is less than 0.1
		clip(model_diffuse.a - 0.1);
	}
	
	return model_diffuse;
}