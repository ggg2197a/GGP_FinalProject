#include "Player.h"

using namespace DirectX;

Player::Player(Mesh* meshPtr, ID3D11DeviceContext* context, Material* mat) : Entity(meshPtr, context, mat)
{

}

void Player::Update(float deltaTime, std::vector<Block*> blocks)
{
	//Update positions with player input
	if (GetAsyncKeyState('A') & 0x8000) 
	{
		Move(XMFLOAT3(deltaTime * -speed,0,0),blocks);
	}
	if (GetAsyncKeyState('D') & 0x8000) 
	{
		Move(XMFLOAT3(deltaTime * speed, 0, 0),blocks);
	}

	TestGrounded(blocks);

	if (GetAsyncKeyState('W') & 0x8000 && grounded)
	{
		Jump();
	}

	Move(XMFLOAT3(0,deltaTime*yVelocity,0),blocks);

	if (!grounded) 
	{
		yVelocity -= 9.8f*deltaTime;
	}
	else 
	{
		yVelocity = 0;
	}
}

void Player::Move(DirectX::XMFLOAT3 movement, std::vector<Block*> blocks)
{
	grounded = false;
	XMFLOAT3 hypPos = XMFLOAT3(position);

	position.x += movement.x;
	position.x = max(-4.5, position.x);
	position.x = min(4.5, position.x);

	for (int i = 0; i < blocks.size(); i++)
	{
		Entity* block = blocks[i];
		if (abs(position.y - block->GetPosition().y) < 1 && abs(position.x - block->GetPosition().x) < 1 && block->visible)
		{
			if (movement.x > 0)
			{
				position.x = block->GetPosition().x - 1;
			}
			else if(movement.x < 0)
			{
				position.x = block->GetPosition().x + 1;
			}
		}
	}

	position.y += movement.y;

	position.y = max(-9, position.y);

	for (int i = 0; i < blocks.size(); i++)
	{
		Entity* block = blocks[i];
		if (abs(position.y - block->GetPosition().y) < 1 && abs(position.x - block->GetPosition().x) < 1 && block->visible)
		{
			if (movement.y > 0)
			{
				position.y = block->GetPosition().y - 1;
				yVelocity = 0;
			}
			else if (movement.y < 0)
			{
				position.y = block->GetPosition().y + 1;
				yVelocity = 0;
				grounded = true;
			}
		}
	}

	if (position.y == -9)
	{
		grounded = true;
	}
}

void Player::TestGrounded(std::vector<Block*> blocks)
{
	if (position.y == -9) 
	{
		grounded = true;
		return;
	}
}

void Player::Jump()
{
	yVelocity = 9;
	grounded = false;
}
