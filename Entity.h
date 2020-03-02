#pragma once
#include "DXCore.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"


class Entity
{
public:
	
	DirectX::XMFLOAT4X4 worldMatrix;
	
	Mesh* mesh;
	Material* material;

	bool visible = true;

	void Move(DirectX::XMFLOAT3 movement);
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetScale(DirectX::XMFLOAT3 scale);
	void SetRotation(float xRot, float yRot, float zRot);
	void PrepareMaterial(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix);

	void PrepareRefractMaterial(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix, ID3D11ShaderResourceView* refractionSRV, ID3D11SamplerState* samplerOptions, ID3D11SamplerState* refractSampler, Camera* camera);

	void Draw(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix);

	void DrawRefract(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix, ID3D11ShaderResourceView* refractionSRV, ID3D11SamplerState* samplerOptions, ID3D11SamplerState* refractSampler, Camera* camera);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetScale();

	float GetRotationX();
	float GetRotationY();
	float GetRotationZ();

	Entity(Mesh* meshPtr, ID3D11DeviceContext* context, Material*);
	Entity(Mesh* meshPtr, ID3D11DeviceContext* context, Material*, DirectX::XMFLOAT3);
	~Entity();
protected:
	void AssembleWorldMatrix();

	ID3D11DeviceContext* context;

	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 scale;

	float xRot;
	float yRot;
	float zRot;
};

