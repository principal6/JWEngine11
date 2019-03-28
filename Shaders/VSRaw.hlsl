static const float2 QuadArray[4] =
{
	float2(-1.0, +1.0),
	float2(+1.0, +1.0),
	float2(-1.0, -1.0),
	float2(+1.0, -1.0),
};

float4 main(uint VertexID : SV_VertexID) : SV_POSITION
{
	return float4(QuadArray[VertexID].xy, 0.0, 1.0);
}