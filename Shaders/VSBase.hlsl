#include "BaseHeader.hlsl"

#define MAX_BONE_COUNT 50
#define EVS_NO_ANIMATION 0
#define EVS_CPU_ANIMATION 1
#define EVS_GPU_ANIMATION 3
#define EVS_INSTANCED 4

cbuffer cbSpace : register(b0)
{
	matrix	WVP;
	matrix	World;
};

cbuffer cbFlags : register(b1)
{
	uint	Flag;
};

cbuffer cbCPUAnimationData : register(b2)
{
	matrix	Bone[MAX_BONE_COUNT];
};

cbuffer cbGPUAnimationData : register(b3)
{
	uint	AnimationID;
	uint	CurrFrame;
	uint	NextFrame;
	float	DeltaTime;
};

Texture2D<float4> animation_texture : register(t0);

float4x4 LoadMatrixFromTexture(uint BoneID)
{
	// Get current animation's info
	float4 anim_info = animation_texture.Load(uint3(AnimationID, 0, 0));
	uint start_y = anim_info.y;

	// Get current frame's bone matrix
	float4 mat_1 = animation_texture[uint2(BoneID * 4 + 0, start_y + CurrFrame)];
	float4 mat_2 = animation_texture[uint2(BoneID * 4 + 1, start_y + CurrFrame)];
	float4 mat_3 = animation_texture[uint2(BoneID * 4 + 2, start_y + CurrFrame)];
	float4 mat_4 = animation_texture[uint2(BoneID * 4 + 3, start_y + CurrFrame)];
	float4x4 mata = float4x4(mat_1, mat_2, mat_3, mat_4);

	// Get next frame's bone matrix
	mat_1 = animation_texture[uint2(BoneID * 4 + 0, start_y + NextFrame)];
	mat_2 = animation_texture[uint2(BoneID * 4 + 1, start_y + NextFrame)];
	mat_3 = animation_texture[uint2(BoneID * 4 + 2, start_y + NextFrame)];
	mat_4 = animation_texture[uint2(BoneID * 4 + 3, start_y + NextFrame)];
	float4x4 matb = float4x4(mat_1, mat_2, mat_3, mat_4);

	// Apply linear interpolation
	mata = mata + DeltaTime * (matb - mata);

	return mata;
}

VS_OUTPUT_MODEL main(VS_INPUT_MODEL input)
{
	VS_OUTPUT_MODEL output;

	uint temp_flag = Flag;

	float4 position_result = input.Position;

	// Check if the model is instanced
	if (temp_flag == EVS_INSTANCED)
	{
		float4x4 instance_world = float4x4(input.InstanceWorld0, input.InstanceWorld1, input.InstanceWorld2, input.InstanceWorld3);

		position_result = mul(position_result, instance_world);

		temp_flag -= EVS_INSTANCED;
	}

	float4 normal_result = float4(input.Normal.xyz, 0);

	// Check if the model is rigged
	if (temp_flag > EVS_NO_ANIMATION)
	{
		float4x4 bone_transform = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

		if (temp_flag == EVS_GPU_ANIMATION)
		{
			// GPU animmation
			bone_transform = LoadMatrixFromTexture(input.BoneID.x) * input.Weight.x;
			bone_transform += LoadMatrixFromTexture(input.BoneID.y) * input.Weight.y;
			bone_transform += LoadMatrixFromTexture(input.BoneID.z) * input.Weight.z;
			bone_transform += LoadMatrixFromTexture(input.BoneID.w) * input.Weight.w;
		}
		else if (temp_flag == EVS_CPU_ANIMATION)
		{
			// CPU animation
			bone_transform = Bone[input.BoneID.x] * input.Weight.x;
			bone_transform += Bone[input.BoneID.y] * input.Weight.y;
			bone_transform += Bone[input.BoneID.z] * input.Weight.z;
			bone_transform += Bone[input.BoneID.w] * input.Weight.w;
		}

		// Rigging #01 Apply bone transform to position
		position_result = mul(input.Position, bone_transform);
		// @important: position_result.w must be '1' before multiplying WVP
		position_result.w = 1.0;

		// Rigging #02 Apply bone transform to normal
		normal_result = mul(normal_result, bone_transform);
	}

	output.Position = mul(position_result, WVP);
	output.WorldPosition = mul(position_result, World).xyz;
	output.Normal = normalize(mul(normal_result, World).xyz);
	output.Tangent = normalize(mul(float4(input.Tangent.xyz, 0), World).xyz);
	output.Bitangent = normalize(mul(float4(input.Bitangent.xyz, 0), World).xyz);

	output.TexCoord = input.TexCoord;
	output.Diffuse = input.Diffuse;
	output.Specular = input.Specular;

	return output;
}