#include "BaseHeader.hlsl"

cbuffer cbFlags : register(b0)
{
	bool HasTexture;
	bool UseLighting;
	float2 pad;
};

cbuffer cbLights : register(b1)
{
	float4 AmbientColor;
	float4 DirectionalColor;
	float4 DirectionalDirection;
};

cbuffer cbCamera : register(b2)
{
	float4 CameraPosition;
};

cbuffer cbPointlight
{
	SPointlight Pointlight;
};

Texture2D	CurrentTexture	: register(t0);

float4 main(VS_OUTPUT_MODEL input) : SV_TARGET
{
	float4 final_color = input.Diffuse;

	if (HasTexture == true)
	{
		final_color = CurrentTexture.Sample(CurrentSampler, input.TexCoord);

		// clip if alpha is less than 0.1
		clip(final_color.a - 0.1);
	}

	if (UseLighting == true)
	{
		// #0 Model base color (diffuse color)
		float3 base_color = final_color.rgb;

		// #1 Ambient light
		float3 ambient_result = AmbientColor.rgb * AmbientColor.a;
		ambient_result = base_color * ambient_result;

		// #2 Directional light
		float directional_light_amount = saturate(dot(input.Normal, DirectionalDirection.xyz));
		float3 directional_result = DirectionalColor.rgb * DirectionalColor.a * directional_light_amount;
		directional_result = base_color * directional_result;

		// #3 Specular light //(relative to Directional Light)
		float3 EyeDirection = normalize(CameraPosition.xyz - input.WorldPosition);
		float3 HalfWay = normalize(DirectionalDirection.xyz + EyeDirection);
		float specular_light_amount = saturate(dot(input.Normal, HalfWay));
		float3 specular_result = pow(specular_light_amount, 30.0f);
		//specular_result = input.Specular.rgb * DirectionalColor.rgb * specular_result;
		specular_result = input.Specular.rgb * specular_result;

		// ## Get final color
		final_color.rgb = saturate(ambient_result + directional_result + specular_result);
	}

	// Gamma correction
	final_color *= final_color;

	return final_color;
}