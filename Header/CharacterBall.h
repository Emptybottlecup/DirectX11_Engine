#pragma once
#include "GameComponent.h"
#include <DirectXCollision.h>

class Camera;
class Mesh;

class CharacterBall : public GameComponent
{
public:
	CharacterBall(Game* GameInstance, int radius, const std::string& filename, DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 rotattion);

	void Initialize();

	void Update(float deltaTime);

	DirectX::XMFLOAT3* GetPosition();

	void SetCamera(Camera* camera);

	DirectX::BoundingSphere* GetCollision();

	DirectX::XMVECTOR GetQuaternion();

	void Draw();

	void DestroyResources();

	DirectX::XMMATRIX GetWorldMatrix();

	DirectX::XMMATRIX GetInvTrWorldViewMatrix();

	~CharacterBall();
private:
	bool LoadModel(const std::string& filename);

	void ProcessNode(aiNode* node, const aiScene* scene);

	Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);

	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;

	ID3D11InputLayout* pInputLayout;
	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;

	DirectX::XMFLOAT3 pPosition;
	DirectX::XMFLOAT3 pRotation;
	DirectX::XMFLOAT3 pScale;
	DirectX::XMVECTOR pQuaternion;

	DirectX::XMMATRIX InvTrWorldView;
	DirectX::XMMATRIX WorldMatrix;
	std::vector<Mesh*> pMeshes;
	const std::string pFilename;

	DirectX::XMFLOAT3 Ambient;
	float Shininess;
	DirectX::XMFLOAT3 Diffuse;
	DirectX::XMFLOAT3 Specular;


	ID3D11ShaderResourceView* pTextureRV;

	int pRadius;
	int pLatitudeSegments;
	int pLongitudeSegments;
	int pIndexCount = 0;

	Camera* pCamera = nullptr;
	DirectX::BoundingSphere pCollision;
};

