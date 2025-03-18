#include "../Header/Model.h"
#include "../Header/CharacterBall.h"
#include <iostream>

struct VertexPos
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexCoord;
	DirectX::XMFLOAT3 Normal;
};

Model::Model(Game* GameInstance, const std::string& filename) : GameComponent(GameInstance), pFilename(filename)
{
	pPosition = { 0.0f, 0.0f, 0.0f };
	pScale = { 1.0f, 1.0f, 1.0f };
	pRotation = { 0.0f, 0.0f, 0.0f };
	pQuaternion = DirectX::XMQuaternionIdentity();

	pCollision.Center = pPosition;
	pCollision.Radius = 1.0f;
	pDistanceForCharacter = DirectX::XMVectorZero();
}

Model::Model(Game* GameInstance, const std::string& filenamel, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation)
	: GameComponent(GameInstance), pFilename(filenamel), pPosition(position), pScale(scale), pRotation(rotation)
{
	pQuaternion = DirectX::XMQuaternionRotationRollPitchYaw(pRotation.x, pRotation.y, pRotation.z);;

	pCollision.Center = pPosition;
	pCollision.Radius = 2.0f;
	pCharacter = nullptr;
}

void Model::Initialize()
{
	HRESULT d3dResult;

	ID3DBlob* vsBuffer = 0;
	bool compileResult = CompileD3DShader("Shaders/3D_Shaders.hlsl", "VS_Main", "vs_5_0", &vsBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"Failed to compile Vertex 3D_Shaders", 0, 0);
		return;
	}
	d3dResult = pGame->GetDevice()->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), 0, &pVertexShader);
	if (FAILED(d3dResult))
	{
		if (vsBuffer)
			vsBuffer->Release();
		return;
	}

	ID3DBlob* psBuffer = 0;
	compileResult = CompileD3DShader("Shaders/3D_Shaders.hlsl", "PS_Main", "ps_5_0", &psBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"Failed to compile Pixel 3D_Shaders", 0, 0);
		return;
	}
	d3dResult = pGame->GetDevice()->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &pPixelShader);
	psBuffer->Release();
	if (FAILED(d3dResult))
	{
		return;
	}

	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	unsigned int totalLayoutElements = ARRAYSIZE(solidColorLayout);
	d3dResult = pGame->GetDevice()->CreateInputLayout(solidColorLayout, totalLayoutElements, vsBuffer->GetBufferPointer(),
		vsBuffer->GetBufferSize(), &pInputLayout);
	vsBuffer->Release();
	if (FAILED(d3dResult))
	{
		return;
	}

	LoadModel(pFilename);

	for (auto mesh : pMeshes)
	{
		mesh->Initialize();
	}
}

bool Model::LoadModel(const std::string& filename)
{
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

	if (pScene == NULL)
	{
		return false;
	}

	ProcessNode(pScene->mRootNode, pScene);

	return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		pMeshes.push_back(ProcessMesh(mesh, scene));
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<VertexPos> vertices;
	std::vector<DWORD> indices;

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		VertexPos vertex;

		vertex.Pos.x = mesh->mVertices[i].x;
		vertex.Pos.y = mesh->mVertices[i].y;
		vertex.Pos.z = mesh->mVertices[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.TexCoord.x = mesh->mTextureCoords[0][i].x;
			vertex.TexCoord.y = mesh->mTextureCoords[0][i].y;
		}
		if (mesh->mNormals)
		{
			vertex.Normal.x = mesh->mNormals[i].x;
			vertex.Normal.y = mesh->mNormals[i].y;
			vertex.Normal.z = mesh->mNormals[i].z;
		}

		vertices.push_back(vertex);
	}

	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	std::wstring texturePath;
	aiColor3D ambientColor(0.0f, 0.0f, 0.0f);
	aiColor3D diffuseColor(0.0f, 0.0f, 0.0f);
	aiColor3D specularColor(0.0f, 0.0f, 0.0f);
	float shininess = 0.0f;
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString path;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			texturePath = std::wstring(path.C_Str(), path.C_Str() + path.length);
		}
		material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
		material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
		material->Get(AI_MATKEY_SHININESS, shininess);
	}
	return new Mesh(pGame,this, vertices, indices, texturePath,ambientColor,diffuseColor,specularColor,shininess);
}

void Model::SetParent(CharacterBall* character)
{
	if (pCharacter == nullptr)
	{
		pCharacter = character;
		DirectX::XMVECTOR inverseCharacterQuaternion = DirectX::XMQuaternionInverse(pCharacter->GetQuaternion());
		initialRelativeRotation = DirectX::XMQuaternionMultiply(pQuaternion, inverseCharacterQuaternion);
		DirectX::XMVECTOR inverseQuaternion = DirectX::XMQuaternionInverse(pCharacter->GetQuaternion());
		pDistanceForCharacter = DirectX::XMVector3Rotate(DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&pPosition), DirectX::XMLoadFloat3(pCharacter->GetPosition())), inverseQuaternion);
	}

}

void Model::Update(float deltaTime)
{
	for (int i = 0; i < pMeshes.size(); i++)
	{
		pMeshes[i]->Update(deltaTime);
	}

	if (pCharacter != nullptr)
	{
		DirectX::XMVECTOR targetPosition = DirectX::XMVectorAdd(DirectX::XMVector3Rotate(pDistanceForCharacter, pCharacter->GetQuaternion()), DirectX::XMLoadFloat3(pCharacter->GetPosition()));
		DirectX::XMStoreFloat3(&pPosition, targetPosition);
		DirectX::XMVECTOR currentRotation = DirectX::XMQuaternionMultiply(initialRelativeRotation, pCharacter->GetQuaternion());
		pQuaternion = currentRotation;
	}
}

DirectX::BoundingSphere Model::GetCollision()
{
	return pCollision;
}

void Model::Draw()
{
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(pQuaternion);
	pGame->GetDeviceContext()->IASetInputLayout(pInputLayout);
	pGame->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	if (!pGame->GetRenderShadowMap())
	{
		pGame->GetDeviceContext()->VSSetShader(pVertexShader, 0, 0);
		pGame->GetDeviceContext()->PSSetShader(pPixelShader, 0, 0);
	}
	WorldMatrix = DirectX::XMMatrixScaling(pScale.x, pScale.y, pScale.z) *
		rotationMatrix * DirectX::XMMatrixTranslation(pPosition.x, pPosition.y, pPosition.z);
	InvTrWorldView = DirectX::XMMatrixScaling(pScale.x, pScale.y, pScale.z) *
		rotationMatrix;
	InvTrWorldView = DirectX::XMMatrixInverse(0, InvTrWorldView);
	InvTrWorldView = DirectX::XMMatrixTranspose(InvTrWorldView);
	InvTrWorldView = InvTrWorldView * pGame->GetCamera()->GetViewMatrix();
	for (int i = 0; i < pMeshes.size(); i++)
	{
		pMeshes[i]->Draw();
	}
}

void Model::DestroyResources()
{
	for (int i = 0; i < pMeshes.size(); i++)
	{
		delete pMeshes[i];
	}

	if (pInputLayout) pInputLayout->Release();
	if (pTextureRV) pTextureRV->Release();
	if (pVertexShader) pVertexShader->Release();
	if (pPixelShader) pPixelShader->Release();

	pVertexShader = 0;
	pPixelShader = 0;
	pInputLayout = 0;
	pTextureRV = 0;
}

DirectX::XMMATRIX Model::GetWorldMatrix()
{
	return WorldMatrix;
}

DirectX::XMMATRIX Model::GetInvTrWorldViewMatrix()
{
	return InvTrWorldView;
}

Model::~Model()
{
	DestroyResources();
}








