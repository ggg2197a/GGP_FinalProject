
struct DirectionalLight {

	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

cbuffer perFrame : register(b0)
{
	float3 CameraPosition;
}

cbuffer lightBuffer : register(b1) {
	DirectionalLight light;
	DirectionalLight light2;
};
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION; // The world position of this PIXEL
};

// Texture-related variables
Texture2D AlbedoTexture			: register(t0);
Texture2D NormalTexture			: register(t1);
Texture2D RoughnessTexture		: register(t2);
Texture2D MetalTexture			: register(t3);
SamplerState BasicSampler		: register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//Re-normalize the normals
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	//Use normal mapping
	// Grab the normal from the map
	float3 normalFromMap = NormalTexture.Sample(BasicSampler, input.uv).rgb * 2.0f - 1.0f;

	// Gather the required vectors for converting the normal
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	// Create the 3x3 matrix to convert from TANGENT-SPACE normals to WORLD-SPACE normals
	float3x3 TBN = float3x3(T, B, N);

	// Adjust the normal from the map and simply use the results
	float3 normalMap = normalize(mul(normalFromMap, TBN));
	input.normal = lerp(input.normal, normalMap, 1);
	//End normal mapping

	//Sample roughness map
	float roughness = RoughnessTexture.Sample(BasicSampler, input.uv).r;
	roughness = lerp(0.2f, roughness, 1);

	//Sample metal map
	float metal = MetalTexture.Sample(BasicSampler, input.uv).r;
	metal = lerp(0.0f, metal, 1);

	//Sample texture
	float4 surfaceColor = AlbedoTexture.Sample(BasicSampler, input.uv);

	//Specular color
	float3 specColor = lerp(float3(0.04, 0.04, 0.04), surfaceColor.rgb, metal);

	float3 totalColor = float3(0, 0, 0);

	//DirectionalLight1 calculations
	float3 toLight = normalize(-light.Direction);
	float3 toCam = normalize(CameraPosition - input.worldPos);

	//diffuse light calculated - OOOOOOOOOOOOOOO
	float diff = saturate(dot(input.normal, toLight));
	
	//specular light calculations - Microfacet BDRF from the PBR Demo
	//other vector required
	float3 h = normalize(toCam + 1);

	//SpecDistribution (uses input.normal, h, roughness)
	float NdotH = saturate(dot(input.normal, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, 0.0000001f);

	//((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH * (a2 - 1) + 1;

	//SpecDis value
	float SpecDis = a2 / (3.14159265359f * denomToSquare * denomToSquare); //D
	
	//Fresnel (uses toCam, h, specColor)
	float VdotH = saturate(dot(toCam, h));

	//Fresnel value
	float3 Fres = specColor + (1 - specColor) * pow(1 - VdotH, 5);

	//Geometric Shadowing (using input.normal, toCam, h, roughness) * 
	//GeoShad (using input.normal, toLight, h, roughness)
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(input.normal, toCam));
	float geo1 = NdotV / (NdotV * (1 - k) + k);

	float NdotL = saturate(dot(input.normal, toLight));
	float geo2 = NdotL / (NdotL * (1 - k) + k);

	//GeoShad final value
	float geoShad = geo1 * geo2;

	//Final formula for MicrofacetBDRF - OOOOOOOOOOOOOOOOOOOOOOOOOO
	float3 spec = (SpecDis * Fres * geoShad) / (4 * max(dot(input.normal, toCam), dot(input.normal, toLight)));

	//calculate diffuse with energy cons
	float3 balance = diff * ((1 - saturate(spec)) * (1 - metal));

	//FIRST TOTALCOLOR ADDITION
	totalColor += (balance * surfaceColor + spec) * 1.05f * light.DiffuseColor + light.AmbientColor;


	//DirectionalLight2 calculations
	toLight = normalize(-light2.Direction);

	//diffuse light calculated - OOOOOOOOOOOOOOO
	diff = saturate(dot(input.normal, toLight));

	//h value already calculated

	//SpecDistribution already calculated

	//Fresnel already calculated

	//Geometric Shadowing (using input.normal, toCam, h, roughness) * 
	//GeoShad (using input.normal, toLight, h, roughness)
	//float k = pow(roughness + 1, 2) / 8.0f;
	//float NdotV = saturate(dot(input.normal, toCam));
	//float geo1 = NdotV / (NdotV * (1 - k) + k);

	//Only change is the light - only change that needs to be accounted for
	NdotL = saturate(dot(input.normal, toLight));
	geo2 = NdotL / (NdotL * (1 - k) + k);

	//GeoShad final value
	geoShad = geo1 * geo2;

	//Final formula for MicrofacetBDRF - OOOOOOOOOOOOOOOOOOOOOOOOOO
	spec = (SpecDis * Fres * geoShad) / (4 * max(dot(input.normal, toCam), dot(input.normal, toLight)));

	//calculate diffuse with energy cons
	balance = diff * ((1 - saturate(spec)) * (1 - metal));

	totalColor += (balance * surfaceColor + spec) * 1.05f * light2.DiffuseColor + light2.AmbientColor;

	return float4(totalColor, 1.0f);
}