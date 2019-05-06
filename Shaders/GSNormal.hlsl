#include "BaseHeader.hlsl"

[maxvertexcount(2)]
void main(point VS_OUTPUT_MODEL input[1], inout LineStream<VS_OUTPUT_MODEL> output)
{
	VS_OUTPUT_MODEL element;

	// Line start
	element = input[0];
	element.Diffuse = float4(1, 1, 0, 1);
	output.Append(element);

	// Line end
	element.Position += input[0].WVPNormal;
	element.Diffuse = float4(1, 1, 0, 1);
	output.Append(element);

	output.RestartStrip();
}