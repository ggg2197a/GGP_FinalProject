#include "Game.h"
#include "Vertex.h"
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include <string>
#include <iostream>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields

	vertexShader = 0;
	pixelShader = 0;

	isMouseDown = false;
	camera = new Camera(width, height);
	

	prevMousePos = { 0,0 };

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Release any (and all!) DirectX objects
	// we've made in the Game class

	//Release Brick ptr's
	brickAlbedo->Release();
	brickMetal->Release();
	brickNormal->Release();
	brickRoughness->Release();

	//Release Crab ptr's
	crabAlbedo->Release();
	crabMetal->Release();
	crabNormal->Release();
	crabRoughness->Release();

	//Release Block ptr's
	rblockAlbedo->Release();
	gblockAlbedo->Release();
	bblockAlbedo->Release();
	oblockAlbedo->Release();
	pblockAlbedo->Release();
	blockMetal->Release();
	blockNormal->Release();
	blockRoughness->Release();

	//Release Sky ptrs
	skyDepthState->Release();
	skyRastState->Release();
	skySRV->Release();

	//Release Sampler ptrs
	samplerOptions->Release(); 
	
	SamplerStatePtr->Release();

	//Release refraction ptrs
	refractSampler->Release();
	refractionRTV->Release();
	refractionSRV->Release();

	// Delete our simple shader objects, which
	// will clean up their own internal DirectX stuff
	delete vertexShader;
	delete pixelShader;
	delete rVertexShader;
	delete rPixelShader;
	delete quadVS;
	delete quadPS;
	delete camera;

	delete tetromino;

	delete rBlockMaterial;
	delete brickMaterial;
	delete crabMaterial;

	delete skyVS;
	delete skyPS;


	for (int i = 0; i < meshArr.size(); i++) {
		delete meshArr[i];
	}
	for (int i = 0; i < entityArr.size(); i++) {
		delete entityArr[i];
	}
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	camera->UpdateProjectionMatrix(width, height);
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMatrices();
	CreateBasicGeometry();

	//Create Light objects
	light = { XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f), XMFLOAT4(0.9f, 0.95f, 0.95f, 1), XMFLOAT3(0, 0, 0.85f) };
	pixelShader->SetData(
		"light",
		&light,
		sizeof(DirectionalLight));

	light2 = { XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f), XMFLOAT4(0.75f, 0.7f, 0.65f, 1), XMFLOAT3(0, -0.95, 0) };
	pixelShader->SetData(
		"light2",
		&light2,
		sizeof(DirectionalLight));

	CreateDDSTextureFromFile(device, L"Assets/Textures/BeachCubeMap.dds", 0, &skySRV);

	// Create a sampler state
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;//D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampDesc, &samplerOptions);

	// Sky resources
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rd, &skyRastState);

	D3D11_DEPTH_STENCIL_DESC ds = {};
	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&ds, &skyDepthState);

	// Refraction setup ------------------------
	ID3D11Texture2D* refractionRenderTexture;

	// Set up render texture
	D3D11_TEXTURE2D_DESC rtDesc = {};
	rtDesc.Width = width;
	rtDesc.Height = height;
	rtDesc.MipLevels = 1;
	rtDesc.ArraySize = 1;
	rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtDesc.Usage = D3D11_USAGE_DEFAULT;
	rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	rtDesc.CPUAccessFlags = 0;
	rtDesc.MiscFlags = 0;
	rtDesc.SampleDesc.Count = 1;
	rtDesc.SampleDesc.Quality = 0;
	device->CreateTexture2D(&rtDesc, 0, &refractionRenderTexture);

	// Set up render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = rtDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	device->CreateRenderTargetView(refractionRenderTexture, &rtvDesc, &refractionRTV);

	// Set up shader resource view for same texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = rtDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(refractionRenderTexture, &srvDesc, &refractionSRV);

	refractionRenderTexture->Release();

	D3D11_SAMPLER_DESC rSamp = {};
	rSamp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	rSamp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	rSamp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	rSamp.Filter = D3D11_FILTER_ANISOTROPIC;
	rSamp.MaxAnisotropy = 16;
	rSamp.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&rSamp, &refractSampler);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
	//Lighting shaders
	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	// Refraction shaders
	quadVS = new SimpleVertexShader(device, context);
	quadVS->LoadShaderFile(L"FullscreenQuadVS.cso");

	quadPS = new SimplePixelShader(device, context);
	quadPS->LoadShaderFile(L"FullscreenQuadPS.cso");

	rVertexShader = new SimpleVertexShader(device, context);
	rVertexShader->LoadShaderFile(L"RefractVS.cso");

	rPixelShader = new SimplePixelShader(device, context);
	rPixelShader->LoadShaderFile(L"RefractPS.cso");

	// Skybox shaders
	skyVS = new SimpleVertexShader(device, context);
	skyVS->LoadShaderFile(L"VSSky.cso");

	skyPS = new SimplePixelShader(device, context);
	skyPS->LoadShaderFile(L"PSSky.cso");
}



// --------------------------------------------------------
// Initializes the matrices necessary to represent our geometry's 
// transformations and our 3D camera
// --------------------------------------------------------
void Game::CreateMatrices()
{
	// Set up world matrix
	// - In an actual game, each object will need one of these and they should
	//    update when/if the object moves (every frame)
	// - You'll notice a "transpose" happening below, which is redundant for
	//    an identity matrix.  This is just to show that HLSL expects a different
	//    matrix (column major vs row major) than the DirectX Math library
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W)); // Transpose for HLSL!

	// Create the View matrix
	// - In an actual game, recreate this matrix every time the camera 
	//    moves (potentially every frame)
	// - We're using the LOOK TO function, which takes the position of the
	//    camera and the direction vector along which to look (as well as "up")
	// - Another option is the LOOK AT function, to look towards a specific
	//    point in 3D space
	XMVECTOR pos = XMVectorSet(0, 0, -5, 0);
	XMVECTOR dir = XMVectorSet(0, 0, 1, 0);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX V = XMMatrixLookToLH(
		pos,     // The position of the "camera"
		dir,     // Direction the camera is looking
		up);     // "Up" direction in 3D space (prevents roll)
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(V)); // Transpose for HLSL!

	// Create the Projection matrix
	// - This should match the window's aspect ratio, and also update anytime
	//    the window resizes (which is already happening in OnResize() below)
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,		// Field of View Angle
		(float)width / height,		// Aspect ratio
		0.1f,						// Near clip plane distance
		100.0f);					// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}


// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	//XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	//XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	//XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in memory
	//    over to a DirectX-controlled data structure (the vertex buffer)
	/*Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +2.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
		{ XMFLOAT3(+1.5f, -1.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
		{ XMFLOAT3(-1.5f, -1.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
	};

	// Set up the indices, which tell us which vertices to use and in which order
	// - This is somewhat redundant for just 3 vertices (it's a simple example)
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };

	Vertex* vertPtr = vertices;
	unsigned int* indexPtr = indices;

	meshArr.push_back(new Mesh(vertPtr, 3, indexPtr, 3, device));

	Vertex vertices2[] =
	{
		{ XMFLOAT3(+2.0f, +1.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
		{ XMFLOAT3(+2.5f, 0.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
		{ XMFLOAT3(1.0f, 0.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
	};
	Vertex* vertPtr2 = vertices2;
	meshArr.push_back(new Mesh(vertPtr2, 3, indexPtr, 3, device));

	unsigned int indices2[] = { 0, 1, 2, 0, 2, 3 };

	Vertex vertices3[] =
	{
		{ XMFLOAT3(-1.0f, 0.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
		{ XMFLOAT3(-3.0f, -1.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
		{ XMFLOAT3(-3.0f, +1.5f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
		{ XMFLOAT3(-1.0f, 1.0f, +0.0f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
	}; 
	Vertex* vertPtr3 = vertices3;
	meshArr.push_back(new Mesh(vertPtr3, 4, indices2, 6, device));*/

	//Load in textures
	std::string cubeUrl = "Assets/Models/cube.obj";
	std::string crabUrl = "Assets/Models/crab.obj";
	
	CreateWICTextureFromFile(device, context, L"Assets/Textures/brick.png", 0, &brickAlbedo);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/brick_normals.png", 0, &brickNormal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/brick_metal.png", 0, &brickMetal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/brick_roughness.png", 0, &brickRoughness);

	CreateWICTextureFromFile(device, context, L"Assets/Textures/mrcrabs.jpg", 0, &crabAlbedo);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/mrcrabs_normals.png", 0, &crabNormal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/mrcrabs_metal.png", 0, &crabMetal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/mrcrabs_roughness.png", 0, &crabRoughness);

	CreateWICTextureFromFile(device, context, L"Assets/Textures/rblock.png", 0, &rblockAlbedo);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/oblock.png", 0, &oblockAlbedo);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/gblock.png", 0, &gblockAlbedo);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/bblock.png", 0, &bblockAlbedo);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/pblock.png", 0, &pblockAlbedo);

	CreateWICTextureFromFile(device, context, L"Assets/Textures/block_normals.png", 0, &blockNormal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/block_metal.png", 0, &blockMetal);
	CreateWICTextureFromFile(device, context, L"Assets/Textures/block_roughness.png", 0, &blockRoughness);
	
	//Sampler for lighting
	D3D11_SAMPLER_DESC SamplerDesc = {};
	SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&SamplerDesc, &SamplerStatePtr);

	meshArr.push_back(new Mesh(&cubeUrl[0], device));
	//Material(SimpleVertexShader * vertShaderPtr, SimplePixelShader * pixelShaderPtr, DirectX::XMFLOAT4 color, float shininess, DirectX::XMFLOAT2 uvScale, ID3D11ShaderResourceView * albedo, ID3D11ShaderResourceView * normals, ID3D11ShaderResourceView * roughness, ID3D11ShaderResourceView * metal, ID3D11SamplerState * samplerState);
	
	//Materials for the bricks, crab, and gems
	brickMaterial = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 1024.0f, XMFLOAT2(2, 2), brickAlbedo, brickNormal, brickRoughness, brickMetal, SamplerStatePtr);
	crabMaterial = new Material(vertexShader, pixelShader, XMFLOAT4(1, 1, 1, 1), 1024.0f, XMFLOAT2(2, 2), crabAlbedo, crabNormal, crabRoughness, crabMetal, SamplerStatePtr);
	rBlockMaterial = new Material(rVertexShader, rPixelShader, XMFLOAT4(1, 1, 1, 1), 1024.0f, XMFLOAT2(2, 2), rblockAlbedo, blockNormal, blockRoughness, blockMetal, SamplerStatePtr);

	//Create objects in arrays
	for (int i = 0; i < 12; i++) //this is the bottom i assume
	{
		entityArr.push_back(new Entity(meshArr[0], context, brickMaterial, XMFLOAT3(i - 5.5f, -10, 0)));
	}

	for (int i = 0; i < 21; i++)
	{
		entityArr.push_back(new Entity(meshArr[0], context, brickMaterial, XMFLOAT3(-5.5, i - 9.0f, 0))); //one side

		entityArr.push_back(new Entity(meshArr[0], context, brickMaterial, XMFLOAT3(5.5, i - 9.0f, 0))); //the other side
	}

	meshArr.push_back(new Mesh(&crabUrl[0], device));

	crab = new Player(meshArr[1], context, crabMaterial);

	crab->SetPosition(XMFLOAT3(0, -9, 0));
	crab->SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));

	tetromino = new Tetromino(meshArr[0], context, rBlockMaterial);

	std::vector<Block*> ents = tetromino->GetContent();

	for (int i = 0; i < 4; i++) 
	{
		//entityArr.push_back(ents[i]);
		//blockArr.push_back(ents[i]);
	}

	entityArr.push_back(crab);

	
	/*entityArr.push_back(new Entity(meshArr[0], context, new Material(vertexShader, pixelShader)));
	entityArr.push_back(new Entity(meshArr[2], context, new Material(vertexShader, pixelShader)));
	entityArr[1]->Move(XMFLOAT3(1.0f, -1.0f, 0));
	entityArr.push_back(new Entity(meshArr[2], context, new Material(vertexShader, pixelShader)));*/
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update our projection matrix since the window size changed
	/*XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * 3.1415926535f,	// Field of View Angle
		(float)width / height,	// Aspect ratio
		0.1f,				  	// Near clip plane distance
		100.0f);			  	// Far clip plane distance
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!*/
	camera->UpdateProjectionMatrix(width, height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	camera->Update(deltaTime);

	crab->Update(deltaTime,blockArr);

	tetromino->Update(totalTime*3,blockArr, crab);

	if (tetromino->GetNewBlocksReady()) 
	{
		std::vector<Block*> ents = tetromino->GetContent();

		for (int i = 0; i < 4; i++)
		{
			entityArr.push_back(ents[i]);
			blockArr.push_back(ents[i]);
		}

		CheckForLines();
	}

	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();
}

// Draws the specified texture to the screen
void Game::DrawFullscreenQuad(ID3D11ShaderResourceView* texture)
{
	// First, turn off our buffers, as we'll be generating the vertex
	// data on the fly in a special vertex shader using the index of each vert
	context->IASetVertexBuffers(0, 0, 0, 0, 0);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Set up the fullscreen quad shaders
	quadVS->SetShader();

	quadPS->SetShaderResourceView("Pixels", texture);
	quadPS->SetSamplerState("Sampler", samplerOptions);
	quadPS->SetShader();

	// Draw
	context->Draw(3, 0);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearRenderTargetView(refractionRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH ,
		1.0f,
		0);

	context->OMSetRenderTargets(1, &refractionRTV, depthStencilView);

	for (int i = 0; i < entityArr.size(); i++) {

		if (entityArr[i]->material == rBlockMaterial)
			continue;

		entityArr[i]->material->GetPixelShader()->SetFloat3("CameraPosition", camera->GetPosition());
		entityArr[i]->Draw(camera->currentView, camera->projectionMatrix);
	}

	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	DrawFullscreenQuad(refractionSRV);
	
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	DrawRefraction();

	ID3D11ShaderResourceView* nullSRV[16] = {};
	context->PSSetShaderResources(0, 16, nullSRV);

	// === Sky box drawing ======================
	// Draw the sky AFTER everything else to prevent overdraw
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Set up sky states
	context->RSSetState(skyRastState);
	context->OMSetDepthStencilState(skyDepthState, 0);

	// Grab the data from the box mesh
	ID3D11Buffer* skyVB = meshArr[0]->GetVertexBuffer();
	ID3D11Buffer* skyIB = meshArr[0]->GetIndexBuffer();

	// Set buffers in the input assembler
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	// Set up the new sky shaders
	skyVS->SetMatrix4x4("view", camera->currentView);
	skyVS->SetMatrix4x4("projection", camera->projectionMatrix);

	skyVS->CopyAllBufferData();
	skyVS->SetShader();


	skyPS->SetShader();
	skyPS->SetShaderResourceView("skyTexture", skySRV);
	skyPS->SetSamplerState("samplerOptions", samplerOptions);

	// Finally do the actual drawing
	context->DrawIndexed(meshArr[0]->GetIndexCount(), 0, 0);

	// Reset states for next frame
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);

	// Due to the usage of a more sophisticated swap chain effect,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
}

void Game::DrawRefraction() 
{
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
	for (int i = 0; i < entityArr.size(); i++) {
		if (entityArr[i]->material != rBlockMaterial)
			continue;

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		ID3D11Buffer* vb = entityArr[i]->mesh->GetVertexBuffer();
		ID3D11Buffer* ib = entityArr[i]->mesh->GetIndexBuffer();
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		entityArr[i]->material->GetPixelShader()->SetFloat3("CameraPosition", camera->GetPosition());
		entityArr[i]->DrawRefract(camera->currentView, camera->projectionMatrix,refractionSRV,samplerOptions,refractSampler,camera);
	}
}

void Game::CheckForLines()
{
	for (int y = -9; y < 9; y++) {
		std::vector<int> lineArr;
		for (int i = 0; i < blockArr.size(); i++)
		{
			if (blockArr[i]->GetPosition().y == y && blockArr[i]->settled && blockArr[i]->visible)
			{
				lineArr.push_back(i);
			}
			else 
			{
				std::cout << (blockArr[i]->GetPosition().y)<<"\n";
			}
		}
		
		if (lineArr.size()>=10) 
		{
			for (int i = 0; i < lineArr.size(); i++)
			{
				for (int j = 0; j < entityArr.size();j++) 
				{
					if (blockArr[lineArr[i]] == entityArr[j]) {
						entityArr[j]->visible = false;
						break;
					}
				}
			}

			for (int i = 0; i < blockArr.size(); i++)
			{
				if (blockArr[i]->GetPosition().y > y)
				{
					blockArr[i]->Move(XMFLOAT3(0,-1,0));
				}
			}


			y--;
		}
	}
}

#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...
	if (buttonState & 0x0002) {
		camera->SetRotation(camera->GetRotationX() + (y - prevMousePos.y) / 100.0f, camera->GetRotationY() + (x - prevMousePos.x) / 100.0f);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion