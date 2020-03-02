#pragma once

#include <d3d11.h>
#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex* vertices, int numVerts, unsigned int* indices, int numIndices, ID3D11Device* device);
	Mesh(char* filename, ID3D11Device* device);
	
	~Mesh();
	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();

	int GetIndexCount();

private:
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	void CreateBuffer(Vertex* vertices, int numVerts, unsigned int* indices, int numIndices, ID3D11Device* device);
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	int indexVerts;
};

