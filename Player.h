#pragma once
#include "Entity.h"
#include "Block.h"
class Player :
	public Entity
{

public:

	Player(Mesh* meshPtr, ID3D11DeviceContext* context, Material* mat);
	void Update(float, std::vector<Block*>);

	void Move(DirectX::XMFLOAT3 movement, std::vector<Block*> blocks);

private:
	float yVelocity = 0.0f;
	float speed = 4.0f;
	bool grounded = true;

	void TestGrounded(std::vector<Block*> blocks);
	void Jump();

};

