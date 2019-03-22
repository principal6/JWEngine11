#include "DefaultHeader.hlsl"

cbuffer cbDefault
{
	bool HasTexture;
	bool UseLighting;
		float2 pad;
	float4 Ambient;
};

cbuffer cbPointlight
{
	SPointlight Pointlight;
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

	if (UseLighting == true)
	{
		model_diffuse.rgb = model_diffuse.rgb * Ambient.rgb * Ambient.a;
	}
	
	return model_diffuse;
}