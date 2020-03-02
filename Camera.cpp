#include "Camera.h"

using namespace DirectX;

Camera::Camera(int width, int height)
{
	//Set position - (0,0,-30) is the default
	position = XMFLOAT3(0, 0, -30);
	direction = XMFLOAT3(0, 0, 0);

	xRot = 0;
	yRot = 0;
}

void Camera::Move(DirectX::XMFLOAT3 movement)
{
	position.x += movement.x;
	position.y += movement.y;
	position.z += movement.z;
}

void Camera::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
}

void Camera::SetDirection(DirectX::XMFLOAT3 direction)
{
	this->direction = direction;
}

void Camera::SetRotation(float xRot, float yRot)
{
	this->xRot = xRot;
	this->yRot = yRot;
}

void Camera::Update(float deltaTime)
{

	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(xRot, yRot, 0);

	XMVECTOR dir = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), quat);

	 XMStoreFloat4x4(&currentView, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&position), dir, XMVectorSet(0, 1, 0, 0))));

}

void Camera::UpdateProjectionMatrix(int width, int height)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

DirectX::XMFLOAT3 Camera::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Camera::GetDirection()
{
	return direction;
}

float Camera::GetRotationX()
{
	return xRot;
}

float Camera::GetRotationY()
{
	return yRot;
}


