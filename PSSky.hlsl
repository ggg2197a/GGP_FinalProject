

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 sampleDirection : TEXCOORD;
};

// Textures and samplers
TextureCube skyTexture		: register(t0);
SamplerState samplerOptions : register(s0);



// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	// Sample the cube map in the specified direction
	return skyTexture.Sample(samplerOptions, input.sampleDirection);
}