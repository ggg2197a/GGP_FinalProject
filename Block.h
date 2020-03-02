#pragma once
#include "Entity.h"
class Block :
	public Entity
{

public:
	Block(Mesh* meshPtr, ID3D11DeviceContext* context, Material* mat);
	bool settled;
private:

};

