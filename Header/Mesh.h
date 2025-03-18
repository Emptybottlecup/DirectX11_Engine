#pragma once
#include "GameComponent.h"

struct VertexPos;

class Mesh : public GameComponent
{
public:
	Mesh(Game* GameInstance, Model* model, std::vector<VertexPos>& vertices, std::vector<DWORD>& indices, std::wstring texturePath, aiColor3D ambient, aiColor3D diffuse, aiColor3D specular, float shininess);

	Mesh(Game* GameInstance, CharacterBall* Character, std::vector<VertexPos>& vertices, std::vector<DWORD>& indices, std::wstring texturePath, aiColor3D ambient, aiColor3D diffuse, aiColor3D specular, float shininess);


	void Initialize();

	void Update(float deltaTime);

	void Draw();

	void DestroyResources();

	~Mesh();
private:

	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	ID3D11ShaderResourceView* pTextureRV;

	std::wstring pTexturePath;

	std::vector <VertexPos> pVertices;
	std::vector <DWORD> pIndices;
	
	Model* pParentModel;

	CharacterBall* pGameCharacter;

	aiColor3D pAmbient;
	aiColor3D pDiffuse;
	aiColor3D pSpecular;
	float pShininess;
};