#include "Tetromino.h"
#include <time.h>

Tetromino::Tetromino(Mesh* mesh, ID3D11DeviceContext* context, Material* mat)
{

	this->mesh = mesh;
	this->context = context;
	this->mat = mat;

	srand(time(NULL));

	content.push_back(NULL);
	content.push_back(NULL);
	content.push_back(NULL);
	content.push_back(NULL);

	Reform();

	SetPosition(XMFLOAT3(9.5,10,0));
}

Tetromino::~Tetromino()
{
	//delete content[0];
	//delete content[1];
	//delete content[2];
	//delete content[3];
}

std::vector<Block*> Tetromino::GetContent()
{
	return content;
}

bool Tetromino::GetNewBlocksReady()
{
	if (newBlocksReady) {
		newBlocksReady = false;
		return true;
	}
	return false;
}

void Tetromino::Reform()
{
	int type;
	if (scriptedMode) {
		type = scriptedBlocks[currentScriptedBlock][0];
	}
	else {
		type = (rand() % 19);
	}

	content[0] = new Block(mesh, context, mat);
	content[1] = new Block(mesh, context, mat);
	content[2] = new Block(mesh, context, mat);
	content[3] = new Block(mesh, context, mat);

	content[0]->SetPosition(XMFLOAT3(types[type][0], types[type][1], 0));
	content[1]->SetPosition(XMFLOAT3(types[type][2], types[type][3], 0));
	content[2]->SetPosition(XMFLOAT3(types[type][4], types[type][5], 0));
	content[3]->SetPosition(XMFLOAT3(types[type][6], types[type][7], 0));

	if (scriptedMode) {
		SetPosition(XMFLOAT3(scriptedBlocks[currentScriptedBlock][1] - 4.5f, 10, 0));
	}
	else {
		SetPosition(XMFLOAT3(round((double)rand() * 10 / (RAND_MAX)) - 4.5f, 10, 0));
	}
	
	newBlocksReady = true;
}

void Tetromino::Update(float totalTime, std::vector<Block*> blocks,Player* crab)
{

	if (totalTime > nextMove) {
		if (Landed(blocks)) 
		{
			for (int i = 0; i < 4; i++)
			{
				content[i]->settled = true;
			}
			if (scriptedMode) {
				currentScriptedBlock++;
				if (currentScriptedBlock > 15)
					scriptedMode = false;
			}
			Reform();
			return;
		}

		if (HitPlayer(crab)) 
		{
			for (int i = 0; i < blocks.size(); i++) {
				blocks[i]->visible = false;
				crab->SetPosition(XMFLOAT3(0, -9, 0));
				//crab->
				currentScriptedBlock = 0;
				scriptedMode = true;
			}
		}

		SlideDown();
		nextMove+= .6f;
	}

}

void Tetromino::SetPosition(XMFLOAT3 oPosition)
{
	XMFLOAT3 blockPos = content[0]->GetPosition();

	XMFLOAT3 position = XMFLOAT3(oPosition);

	position.x -= blockPos.x;
	position.y -= blockPos.y;
	position.z -= blockPos.z;

	float moveBy = 0;

	for (int i = 0; i < 4; i++)
	{
		blockPos = content[i]->GetPosition();

		blockPos.x += position.x;
		blockPos.y += position.y;
		blockPos.z += position.z;

		content[i]->SetPosition(blockPos);

		if (blockPos.x < -4.5)
		{
			moveBy = max(abs(blockPos.x - 4.5), moveBy);
		}

		if (blockPos.x > 4.5)
		{
			moveBy = min(-abs(blockPos.x - 4.5), moveBy);
		}
	}

	if (moveBy != 0) 
	{

		oPosition.x += moveBy;

		SetPosition(oPosition);
	}
}

void Tetromino::SlideDown()
{
	for (int i = 0; i < 4; i++)
	{
		XMFLOAT3 blockPos = content[i]->GetPosition();

		blockPos.y--;

		content[i]->SetPosition(blockPos);
	}
}

bool Tetromino::Landed(std::vector<Block*> blocks)
{
	for (int i = 0; i < 4; i++) 
	{
		XMFLOAT3 blockPos = content[i]->GetPosition();
		if (blockPos.y == -9) 
		{
			return true;
		}

		for (int f = 0; f < blocks.size(); f++)
		{
			Block* block = blocks[f];

			if (block->settled && block->GetPosition().y == blockPos.y-1 && blockPos.x == block->GetPosition().x && block->visible)
			{
				return true;
			}

		}
	}
	return false;
}

bool Tetromino::HitPlayer(Player* crab)
{
	for (int i = 0; i < 4; i++)
	{
		XMFLOAT3 blockPos = content[i]->GetPosition();

		if (abs(crab->GetPosition().y - (blockPos.y-1)) < 1 && abs(crab->GetPosition().x - blockPos.x) < 1) {
			return true;
		}
	}
	return false;
}
