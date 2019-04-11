Texture2D<uint4> CurrentTexture : register(t0);

uint4 main(float4 Position : SV_POSITION) : SV_TARGET
{
	uint4 final_color = CurrentTexture.Load(int3(Position.xy, 0));
	return final_color;
}