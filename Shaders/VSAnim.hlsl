#include "BaseHeader.hlsl"

#define MAX_BONE_COUNT 60

cbuffer cb
{
	matrix WVP;
	matrix World;

	matrix Bone[MAX_BONE_COUNT];
};

VS_OUTPUT main(VS_INPUT_SKINNED input)
{
	VS_OUTPUT output;

	float4 position_result = input.Position;
	float4 normal_result = float4(input.Normal.xyz, 0);

	// #01 Apply bone transform to position
	float4x4 bone_transform = Bone[input.BoneID.x] * input.Weight.x;
	bone_transform += Bone[input.BoneID.y] * input.Weight.y;
	bone_transform += Bone[input.BoneID.z] * input.Weight.z;
	bone_transform += Bone[input.BoneID.w] * input.Weight.w;

	position_result = mul(input.Position, bone_transform);
	//@warning: position_result.w must be '1' before multiplying WVP
	position_result.w = 1.0;

	// #02 Apply bone transform to normal
	normal_result = mul(normal_result, bone_transform);

	output.Position = mul(position_result, WVP);
	output.WorldPosition = mul(position_result, World).xyz;
	output.Normal = normalize(mul(normal_result, World).xyz);

	output.TexCoord = input.TexCoord;
	output.Diffuse = input.Diffuse;
	output.Specular = input.Specular;
	

	return output;
}