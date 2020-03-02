#include "Block.h"

Block::Block(Mesh* meshPtr, ID3D11DeviceContext* context, Material* mat) : Entity(meshPtr,context,mat)
{
	settled = false;
}
