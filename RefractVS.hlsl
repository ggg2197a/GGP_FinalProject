cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

struct VertexShaderInput 
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	noperspective float screenUV		: TEXCOORD1;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	noperspective float2 screenUV		: TEXCOORD1;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	matrix worldViewProj = mul(mul(world, view), projection);
	output.position = mul(float4(input.position, 1.0f), worldViewProj);

	output.worldPos = mul(float4(input.position, 1.0f), world).xyz;

	output.normal = mul(input.normal, (float3x3)world);
	output.normal = normalize(output.normal);

	output.tangent = normalize(mul(input.tangent, (float3x3)world));

	output.uv = input.uv;

	output.screenUV = (output.position.xy / output.position.w);
	output.screenUV.x = output.screenUV.x * 0.5f + 0.5f;
	output.screenUV.y = -output.screenUV.y * 0.5f + 0.5f;

	return output;
}