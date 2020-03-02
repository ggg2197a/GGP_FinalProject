#pragma once

#include "DXCore.h"
#include <DirectXMath.h>

class Camera
{
public:

	void Move(DirectX::XMFLOAT3 movement);
	void SetPosition(DirectX::XMFLOAT3 position);
	void SetDirection(DirectX::XMFLOAT3 direction);
	void SetRotation(float xRot, float yRot);
	void Update(float);
	void UpdateProjectionMatrix(int width, int height);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetDirection();

	float GetRotationX();
	float GetRotationY();

	DirectX::XMFLOAT4X4 currentView;
	DirectX::XMFLOAT4X4 projectionMatrix;
	Camera(int width, int height);

private:

	
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 direction;
	float xRot;
	float yRot;
};

