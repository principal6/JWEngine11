#include "BaseHeader.hlsl"

#define FLAG_OFF 0
#define FLAG_ON 1
#define FLAG_ID_USE_LIGHTING 0
#define FLAG_ID_USE_DIFFUSE_TEXTURE 1
#define FLAG_ID_USE_NORMAL_TEXTURE 2

cbuffer cbFlags : register(b0)
{
	// FLAG (32 bit)
	// ID 00 HEX 0x0001 UseLighting
	// ID 01 HEX 0x0002 UseDiffuseTexture
	// ID 02 HEX 0x0004 UseNormalTexture
	// ID 03 HEX 0x0008 ...
	// ID 04 HEX 0x0010 ...
	// ID 05 HEX 0x0020 ...
	uint FlagPS;
	float3 pad;
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

Texture2D	TextureDiffuse	: register(t0);
Texture2D	TextureNormal	: register(t1);

uint InterpretFlag(uint Flag, uint FlagID)
{
	uint result = (FlagPS << (31 - FlagID)) >> 31;
	return result;
}

float4 main(VS_OUTPUT_MODEL input) : SV_TARGET
{
	float4 final_color = input.Diffuse;
	float3 final_normal = input.Normal;

	if (InterpretFlag(FlagPS, FLAG_ID_USE_NORMAL_TEXTURE) == FLAG_ON)
	{
		float3 normal_map = TextureNormal.Sample(CurrentSampler, input.TexCoord).xyz;

		// Map normal from [0.0, 1.0] to [-1.0, 1.0]
		normal_map = (normal_map * 2.0) - 1.0;

		final_normal = input.Tangent * normal_map.x + input.Bitangent * normal_map.y + input.Normal * normal_map.z;
		final_normal = normalize(final_normal);
	}

	if (InterpretFlag(FlagPS, FLAG_ID_USE_DIFFUSE_TEXTURE) == FLAG_ON)
	{
		final_color = TextureDiffuse.Sample(CurrentSampler, input.TexCoord);

		// clip if alpha is less than 0.1
		clip(final_color.a - 0.1);
	}

	if (InterpretFlag(FlagPS, FLAG_ID_USE_LIGHTING) == FLAG_ON)
	{
		// #0 Model base color (diffuse color)
		float3 base_color = final_color.rgb;

		// #1 Ambient light
		float3 ambient_result = AmbientColor.rgb * AmbientColor.a;
		ambient_result = base_color * ambient_result;

		// #2 Directional light
		float directional_light_amount = saturate(dot(final_normal, -DirectionalDirection.xyz));
		float3 directional_result = DirectionalColor.rgb * DirectionalColor.a * directional_light_amount;
		directional_result = base_color * directional_result;

		// #3 Specular light //(relative to Directional Light)
		float3 EyeDirection = normalize(CameraPosition.xyz - input.WorldPosition);
		float3 HalfWay = normalize(-DirectionalDirection.xyz + EyeDirection);
		float specular_light_amount = saturate(dot(final_normal, HalfWay));
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