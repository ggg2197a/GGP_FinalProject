#pragma once
#include "DXCore.h"
#include "Entity.h"
#include "Block.h"
#include "Player.h"
#include "winuser.h"
#include <DirectXMath.h>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

class Tetromino
{

public:

	Tetromino(Mesh*, ID3D11DeviceContext*, Material*);
	~Tetromino();

	std::vector<Block*> GetContent();

	bool GetNewBlocksReady();

	void Reform();

	void Update(float, std::vector<Block*>, Player*);

private:

	bool newBlocksReady = false;

	bool scriptedMode = true;
	int currentScriptedBlock = 0;
	const int scriptedBlocks[16][2] = { //level used from the 2018 tetris world championship finals
		{11, 9},
		{8, 7},
		{4, 8},
		{14, 1},
		{4, 4},
		{5, 5},
		{2, 9},
		{12, 6},
		{2, 8},
		{6, 3},
		{0, 6},
		{4, 2},
		{0, 8},
		{2, 5},
		{7, 6},
		{2, 0},
	};

	Mesh* mesh;
	Material* mat;
	ID3D11DeviceContext* context;

	float nextMove = .5f;

	std::vector<Block*> content;

	void SetPosition(XMFLOAT3);
	void SlideDown();
	bool Landed(std::vector<Block*> blocks);
	bool HitPlayer(Player*);

	const int types[19][8]= {
		//square
		{0,0, 1,0, 0,1, 1,1}, //0

		//long
		{0,0, 1,0, 2,0, -1,0}, //1
		{0,0, 0,1, 0,2, 0,-1}, //2

		//s
		{0,0, 1,0, 0,-1, -1,-1}, //3
		{0,0, 0,-1, -1,0, -1,1}, //4

		//z
		{0,0, -1,0, 0,-1, 1,-1}, //5
		{0,0, 0,-1, 1,0, 1,1}, //6

		//L
		{0,0, 1,0, 0,1, 0,2}, //7
		{0,0, 0,1, -1,0, -2,0}, //8
		{0,0, -1,0, 0,-1, 0,-2}, //9 - upside down
		{0,0, 0,-1, 1,0, 2,0}, //10

		//J
		{0,0, -1,0, 0,1, 0,2}, //11 - upright
		{0,0, 0,-1, -1,0, -2,0}, //12
		{0,0, 1,0, 0,-1, 0,-2}, //13
		{0,0, 0,1, 1,0, 2,0}, //14

		//T
		{0,0, -1,0, 0,1, 0,-1}, //15
		{0,0, 1,0, -1,0, 0,-1}, //16
		{0,0, 0,-1, 1,0, -1,0}, //17
		{0,0, 0,1, 0,-1, -1,0}, //18

	};
};