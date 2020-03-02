#include "Entity.h"
#include "Camera.h"

using namespace DirectX;



Entity::Entity(Mesh* meshPtr, ID3D11DeviceContext* context, Material* materialPtr)
{
	worldMatrix = XMFLOAT4X4();
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	material = materialPtr;

	xRot = 0.0f;
	yRot = 0.0f;
	zRot = 0.0f;

	this->mesh = meshPtr;
	this->context = context;
}

Entity::Entity(Mesh* meshPtr, ID3D11DeviceContext* context, Material* materialPtr, XMFLOAT3 position)
{
	worldMatrix = XMFLOAT4X4();
	this->position = position;
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	material = materialPtr;

	xRot = 0.0f;
	yRot = 0.0f;
	zRot = 0.0f;

	this->mesh = meshPtr;
	this->context = context;
}

Entity::~Entity()
{
	
}

void Entity::AssembleWorldMatrix()
{
	XMMATRIX matTrans = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX matRot = XMMatrixRotationX(xRot) *XMMatrixRotationY(yRot)* XMMatrixRotationZ(zRot);
	XMMATRIX matScale = XMMatrixScaling(scale.x, scale.y, scale.z);

	XMMATRIX world = matScale * matRot * matTrans;

	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
}

void Entity::Move(XMFLOAT3 movement)
{
	position.x += movement.x;
	position.y += movement.y;
	position.z += movement.z;
}

void Entity::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
}

void Entity::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
}

void Entity::SetRotation(float xRot, float yRot, float zRot)
{
	this->xRot = xRot;
	this->yRot = yRot;
	this->zRot = zRot;
}

void Entity::PrepareMaterial(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix)
{
	AssembleWorldMatrix();

	material->GetVertexShader()->SetMatrix4x4("world", worldMatrix);
	material->GetVertexShader()->SetMatrix4x4("view", viewMatrix);
	material->GetVertexShader()->SetMatrix4x4("projection", projMatrix);
	material->GetVertexShader()->SetFloat2("uvScale", material->GetUVScale());

	material->GetVertexShader()->SetShader();
	material->GetVertexShader()->CopyAllBufferData();

	material->GetPixelShader()->SetMatrix4x4("world", worldMatrix);
	material->GetPixelShader()->SetMatrix4x4("view", viewMatrix);
	material->GetPixelShader()->SetMatrix4x4("projection", projMatrix);

	material->GetPixelShader()->SetShaderResourceView("AlbedoTexture", material->GetAlbedoShaderResourceView());
	material->GetPixelShader()->SetShaderResourceView("NormalTexture", material->GetNormalsShaderResourceView());
	material->GetPixelShader()->SetShaderResourceView("RoughnessTexture", material->GetRoughnessShaderResourceView());
	material->GetPixelShader()->SetShaderResourceView("MetalTexture", material->GetMetalShaderResourceView());
	material->GetPixelShader()->SetSamplerState("BasicSampler", material->GetSamplerState());

	material->GetPixelShader()->SetShader();
	material->GetPixelShader()->CopyAllBufferData();
}

void Entity::PrepareRefractMaterial(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix, ID3D11ShaderResourceView* refractionSRV, ID3D11SamplerState* samplerOptions, ID3D11SamplerState* refractSampler,Camera* camera)
{
	AssembleWorldMatrix();

	// Setup vertex shader
	material->GetVertexShader()->SetMatrix4x4("world", worldMatrix);
	material->GetVertexShader()->SetMatrix4x4("view",viewMatrix);
	material->GetVertexShader()->SetMatrix4x4("projection", projMatrix);
	material->GetVertexShader()->CopyAllBufferData();
	material->GetVertexShader()->SetShader();

	// Setup pixel shader
	material->GetPixelShader()->SetShaderResourceView("ScenePixels", refractionSRV);	// Pixels of the screen
	material->GetPixelShader()->SetShaderResourceView("NormalMap", material->GetNormalsShaderResourceView());	// Normal map for the object itself
	material->GetPixelShader()->SetSamplerState("BasicSampler", samplerOptions);			// Sampler for the normal map
	material->GetPixelShader()->SetSamplerState("RefractSampler", refractSampler);	// Uses CLAMP on the edges
	material->GetPixelShader()->SetFloat3("CameraPosition", camera->GetPosition());
	material->GetPixelShader()->SetMatrix4x4("view", camera->currentView);				// View matrix, so we can put normals into view space
	material->GetPixelShader()->CopyAllBufferData();
	material->GetPixelShader()->SetShader();
}

void Entity::Draw(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* meshVertexBuffer = mesh->GetVertexBuffer();

	context->IASetVertexBuffers(0, 1, &meshVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	PrepareMaterial(viewMatrix, projMatrix);

	context->DrawIndexed(
		mesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}

void Entity::DrawRefract(DirectX::XMFLOAT4X4 viewMatrix, DirectX::XMFLOAT4X4 projMatrix, ID3D11ShaderResourceView* refractionSRV, ID3D11SamplerState* samplerOptions, ID3D11SamplerState* refractSampler, Camera* camera)
{
	if (!visible)
		return;

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* meshVertexBuffer = mesh->GetVertexBuffer();

	context->IASetVertexBuffers(0, 1, &meshVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	PrepareRefractMaterial(viewMatrix, projMatrix, refractionSRV, samplerOptions, refractSampler, camera);

	context->DrawIndexed(
		mesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}

DirectX::XMFLOAT3 Entity::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Entity::GetScale()
{
	return scale;
}

float Entity::GetRotationX()
{
	return xRot;
}

float Entity::GetRotationY()
{
	return yRot;
}

float Entity::GetRotationZ()
{
	return zRot;
}
