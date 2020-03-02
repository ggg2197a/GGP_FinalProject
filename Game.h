#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Lights.h"
#include <vector>
#include "Player.h"
#include "Tetromino.h"
#include "Block.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void DrawFullscreenQuad(ID3D11ShaderResourceView* texture);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);

	//Brick resources
	ID3D11ShaderResourceView* brickAlbedo;
	ID3D11ShaderResourceView* brickNormal;
	ID3D11ShaderResourceView* brickRoughness;
	ID3D11ShaderResourceView* brickMetal;

	//Crab resources
	ID3D11ShaderResourceView* crabAlbedo;
	ID3D11ShaderResourceView* crabNormal;
	ID3D11ShaderResourceView* crabRoughness;
	ID3D11ShaderResourceView* crabMetal;

	//Block Resources
	ID3D11ShaderResourceView* rblockAlbedo;
	ID3D11ShaderResourceView* gblockAlbedo;
	ID3D11ShaderResourceView* bblockAlbedo;
	ID3D11ShaderResourceView* oblockAlbedo;
	ID3D11ShaderResourceView* pblockAlbedo;
	ID3D11ShaderResourceView* blockNormal;
	ID3D11ShaderResourceView* blockRoughness;
	ID3D11ShaderResourceView* blockMetal;


	ID3D11SamplerState* SamplerStatePtr;
private:
	std::vector<Mesh*> meshArr;
	std::vector<Entity*> entityArr;
	std::vector<Block*> blockArr;
	Camera* camera;
	Player* crab;
	Tetromino* tetromino;

	Material* brickMaterial;
	Material* crabMaterial;
	Material* rBlockMaterial;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateMatrices();
	void CreateBasicGeometry();
	void DrawRefraction();
	void CheckForLines();

	ID3D11ShaderResourceView* skySRV;
	ID3D11RasterizerState* skyRastState;
	ID3D11DepthStencilState* skyDepthState;

	ID3D11SamplerState* samplerOptions;

	// Wrappers for DirectX shaders to provide simplified functionality
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	SimpleVertexShader* rVertexShader;
	SimplePixelShader* rPixelShader;
	SimpleVertexShader* quadVS;
	SimplePixelShader* quadPS;

	ID3D11SamplerState* refractSampler;
	ID3D11RenderTargetView* refractionRTV;
	ID3D11ShaderResourceView* refractionSRV;
	SimpleVertexShader* skyVS;
	SimplePixelShader* skyPS;

	// The matrices to go from model space to screen space
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	DirectionalLight light;
	DirectionalLight light2;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;

	bool isMouseDown;

};

