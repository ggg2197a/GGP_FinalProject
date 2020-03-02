#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "DXCore.h"
#include "SimpleShader.h"

class Material
{
public:
	Material(SimpleVertexShader* vertShaderPtr, SimplePixelShader* pixelShaderPtr, DirectX::XMFLOAT4 color, float shininess, DirectX::XMFLOAT2 uvScale, ID3D11ShaderResourceView* albedo, ID3D11ShaderResourceView* normals, ID3D11ShaderResourceView* roughness, ID3D11ShaderResourceView* metal, ID3D11SamplerState* samplerState);
	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();

	ID3D11ShaderResourceView* GetAlbedoShaderResourceView();
	ID3D11ShaderResourceView* GetNormalsShaderResourceView();
	ID3D11ShaderResourceView* GetRoughnessShaderResourceView();
	ID3D11ShaderResourceView* GetMetalShaderResourceView();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT4 GetColor();
	float GetShininess();
	ID3D11SamplerState* GetSamplerState();

private:

	SimpleVertexShader* vertShader;
	SimplePixelShader* pixelShader;
	
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT4 color;
	float shininess;

	ID3D11ShaderResourceView* albedoSRV;
	ID3D11ShaderResourceView* normalSRV;
	ID3D11ShaderResourceView* roughnessSRV;
	ID3D11ShaderResourceView* metalSRV;
	ID3D11SamplerState* samplerState;
};

