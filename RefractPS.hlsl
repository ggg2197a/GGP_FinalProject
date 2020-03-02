cbuffer externalData : register(b0)
{
	matrix view;
	float3 CameraPosition;
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

Texture2D ScenePixels		: register(t0);
Texture2D NormalMap			: register(t1);
SamplerState BasicSampler	: register(s0);
SamplerState RefractSampler	: register(s1);

float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	float3 normalFromTexture = NormalMap.Sample(BasicSampler, input.uv).xyz * 2 - 1;

	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	input.normal = normalize(mul(normalFromTexture, TBN));

	float indexOfRefr = 0.9f; 
	float refrAdjust = 0.1f;  

	float3 dirToPixel = normalize(input.worldPos - CameraPosition);
	float3 refrDir = refract(dirToPixel, input.normal, indexOfRefr);

	float2 refrUV = mul(float4(refrDir, 0.0f), view).xy * refrAdjust;
	refrUV.x *= -1.0f; 

	float4 output = ScenePixels.Sample(RefractSampler, input.screenUV + refrUV);

	output.x += 1.0;
	output.x *= 0.5;
	output.y += 0;
	output.y *= 0.5;
	output.z += 0;
	output.z *= 0.5;
	output.w += 0;
	output.w *= 0.5;

	return output;
}