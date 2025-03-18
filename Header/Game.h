#pragma once
#include "DisplayWin32.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <chrono>
#include <wrl.h>
#include <stdio.h>
#include <vector>
#include "GameComponent.h"
#include <fstream>
#include <sstream>
#include "InputDevice.h"
#include <set>
#include "Camera.h"
#include <WICTextureLoader.h>


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "DirectXTK.lib")

class GameComponent;
class GameStick;
class Camera;
class CharacterBall;
class Model;

class Game
{
public:

	Game();

	void Initialize(HINSTANCE hInstance, HWND hwnd, InputDevice* InputDevice, CharacterBall* CharacterBall);

	void PushGameComponents(GameComponent* newGameComponent);

	void PushCollisions(Model* newModel);

	void ChangeConstantBuffer(DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projMatrix, DirectX::XMMATRIX InverseTransposeWorldMatrix, DirectX::XMVECTOR Ka,
		DirectX::XMVECTOR Kd, DirectX::XMVECTOR KsX, float time);

	virtual void Run();

	ID3D11Device* GetDevice();

	ID3D11DeviceContext* GetDeviceContext();

	IDXGISwapChain* GetSwapChain();

	HWND& GetWindowHandle();

	Camera* GetCamera();

	InputDevice* GetInputDevice();

	std::vector<GameComponent*> GetGameComponents();

	void Draw();

	void Update(float deltaTime);

	void PrepareFrame(std::chrono::time_point<std::chrono::steady_clock>& PrevTime);

	bool GetRenderShadowMap();

	void RenderShadowMap();
	void DeleteResources();

	~Game();

	

protected:
	HINSTANCE phInstance;
	HWND pWindow;
	unsigned int width;
	unsigned int height;
	InputDevice* pInputDevice;
	Camera* pCamera;
	CharacterBall* pCharacterBall;
	std::vector<GameComponent*> pGameComponents;
	std::set<Model*> pModelsCollisions;

	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	IDXGISwapChain* pSwapChain;
	ID3D11RenderTargetView* pBackBufferTarget;
	ID3D11Buffer* pConstantBuffer;
	ID3D11RasterizerState* pRasterizerState;
	D3D11_VIEWPORT viewport;
	ID3D11Texture2D* pDepthTexture;
	ID3D11DepthStencilView* pDepthStencilView;
	ID3D11DepthStencilState* pDSState;
	ID3D11SamplerState* pSamplerLinear;
	float deltaTime = 0;

	D3D11_VIEWPORT viewport_depth_directional_light{};
	ID3D11ShaderResourceView* resource_view_depth_directional_light = nullptr;
	ID3D11DepthStencilView* depth_stencil_view_depth_directional_light = nullptr;
	ID3D11Texture2D* background_texture_depth;
	ID3D11SamplerState* shadowSample;
	ID3D11VertexShader* pVertexShadowShader;
	ID3D11PixelShader* pPixelShadowShader;
	bool renderShadowMap = false;

	DirectX::XMMATRIX lightViewMatrix;
	DirectX::XMMATRIX lightProjMatrix;
	DirectX::XMVECTOR lightDirection;
	DirectX::XMVECTOR lightpos;

	ID3D11ShaderResourceView* pNewTexture;
};