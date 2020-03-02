#include "Material.h"

Material::Material(SimpleVertexShader* vertShaderPtr, SimplePixelShader* pixelShaderPtr, DirectX::XMFLOAT4 color, float shininess, DirectX::XMFLOAT2 uvScale, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normals, ID3D11ShaderResourceView* roughness, ID3D11ShaderResourceView* metal, ID3D11SamplerState* samplerState)
{
	this->vertShader = vertShaderPtr;
	this->pixelShader = pixelShaderPtr;
	this->color = color;
	this->shininess = shininess;
	this->albedoSRV = albedo;
	this->normalSRV = normals;
	this->roughnessSRV = roughness;
	this->metalSRV = metal;
	this->samplerState = samplerState;
	this->uvScale = uvScale;
}

SimpleVertexShader* Material::GetVertexShader()
{
	return vertShader;
}

SimplePixelShader* Material::GetPixelShader()
{
	return pixelShader;
}

ID3D11ShaderResourceView* Material::GetAlbedoShaderResourceView()
{
	return albedoSRV;
}

ID3D11ShaderResourceView* Material::GetNormalsShaderResourceView()
{
	return normalSRV;
}

ID3D11ShaderResourceView* Material::GetRoughnessShaderResourceView()
{
	return roughnessSRV;
}

ID3D11ShaderResourceView* Material::GetMetalShaderResourceView()
{
	return metalSRV;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT4 Material::GetColor()
{
	return color;
}

float Material::GetShininess()
{
	return shininess;
}

ID3D11SamplerState* Material::GetSamplerState()
{
	return samplerState;
}
