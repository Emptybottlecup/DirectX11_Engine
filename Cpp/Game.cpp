#include "../Header/Game.h"
#include <iostream>
#include "../Header/GameStick.h"
#include "../Header/CharacterBall.h"
#include "../Header/Model.h"

bool CompileD3DShader(const char* filePath, const char* entry, const char* shaderModel, ID3DBlob** buffer)
{
	ID3DBlob* errorBuffer = 0;
	HRESULT result;

	int bufferSize = MultiByteToWideChar(CP_ACP, 0, filePath, -1, NULL, 0);
	WCHAR* wideFilePath = new WCHAR[bufferSize];
	MultiByteToWideChar(CP_ACP, 0, filePath, -1, wideFilePath, bufferSize);

	result = D3DCompileFromFile(wideFilePath, 0, 0, entry, shaderModel, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR, 0, buffer, &errorBuffer);

	if (FAILED(result))
	{
		if (errorBuffer != 0)
		{
			OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
			errorBuffer->Release();
		}
		return false;
	}

	if (errorBuffer != 0)
		errorBuffer->Release();

	return true;
}

struct ConstantBuffer
{
	DirectX::XMMATRIX worldMatrix;
	DirectX::XMMATRIX viewMatrix;
	DirectX::XMMATRIX projMatrix;

	DirectX::XMVECTOR LightDirection;
	DirectX::XMVECTOR LightColor;
	DirectX::XMVECTOR ViewerPosition;
	DirectX::XMVECTOR KaSpecPowKsX;

	DirectX::XMVECTOR Ka;
	DirectX::XMVECTOR Kd;
	DirectX::XMVECTOR KsX;

	DirectX::XMMATRIX InverseTransposeWorldMatrix;

	float time;

	DirectX::XMMATRIX lightViewMatrix;
	DirectX::XMMATRIX lightProjMatrix;
};

Game::Game()
{

}

ID3D11Device* Game::GetDevice()
{
	return pDevice;
}

ID3D11DeviceContext* Game::GetDeviceContext()
{
	return pDeviceContext;
}

IDXGISwapChain* Game::GetSwapChain()
{
	return pSwapChain;
}

HWND& Game::GetWindowHandle()
{
	return pWindow;
}

Camera* Game::GetCamera()
{
	return pCamera;
}

InputDevice* Game::GetInputDevice()
{
	return pInputDevice;
}

std::vector<GameComponent*> Game::GetGameComponents()
{
	return pGameComponents;
}

void Game::PushGameComponents(GameComponent* newGameComponent)
{
	pGameComponents.push_back(newGameComponent);
}

void Game::PushCollisions(Model* newModel)
{
	pModelsCollisions.insert(newModel);
}

bool Game::GetRenderShadowMap()
{
	return renderShadowMap;
}

void Game::ChangeConstantBuffer(DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projMatrix, DirectX::XMMATRIX InverseTransposeWorldMatrix, DirectX::XMVECTOR Ka,
	DirectX::XMVECTOR Kd, DirectX::XMVECTOR KsX, float time)
{
	DirectX::XMVECTOR lightDireForRenderScene = DirectX::XMVector4Normalize(lightDirection);
	lightDireForRenderScene = DirectX::XMVector4Transform(lightDireForRenderScene, pCamera->GetViewMatrix());
	lightDireForRenderScene = DirectX::XMVector4Normalize(lightDireForRenderScene);
	DirectX::XMVECTOR lightColor = DirectX::XMVectorSet(2.0f, 2.0f, 2.0f, 0.0f);
	DirectX::XMVECTOR KaSpecPowKsX = DirectX::XMVectorSet(0.4f, 50.0f, 0.25f, 0.0f);
	ConstantBuffer constBuf = { worldMatrix, viewMatrix, projMatrix,lightDireForRenderScene, lightColor, pCamera->GetPositionVector(), KaSpecPowKsX, Ka, Kd, KsX ,InverseTransposeWorldMatrix, time, lightViewMatrix, lightProjMatrix };

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &constBuf, sizeof(ConstantBuffer));
	pDeviceContext->Unmap(pConstantBuffer, 0);
}

void Game::Initialize(HINSTANCE hInstance, HWND hwnd, InputDevice* InputDevice, CharacterBall* CharacterBall = nullptr)
{
	phInstance = hInstance;
	pWindow = hwnd;
	pInputDevice = InputDevice;
	pCamera = new Camera(&hwnd, InputDevice);

	RECT dimensions;
	GetClientRect(hwnd, &dimensions);
	width = dimensions.right - dimensions.left;
	height = dimensions.bottom - dimensions.top;

	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };
	unsigned int totalFeatureLevels = ARRAYSIZE(featureLevel);
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = width;
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 144;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = pWindow;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = 0;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;


	HRESULT result;

	result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&pSwapChain,
		&pDevice,
		nullptr,
		&pDeviceContext);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create D3D11 Device!\n");
		return;
	}

	ID3D11Texture2D* backBufferTexture;
	result = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to get back buffer texture!\n");
		return;
	}
	result = pDevice->CreateRenderTargetView(backBufferTexture, 0, &pBackBufferTarget);
	if (backBufferTexture) backBufferTexture->Release();

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	result = pDevice->CreateDepthStencilState(&dsDesc, &pDSState);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create depth state!\n");
		return;
	}

	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
	depthTexDesc.Width = width;
	depthTexDesc.Height = height;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;
	result = pDevice->CreateTexture2D(&depthTexDesc, 0, &pDepthTexture);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create depth texture!\n");
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = depthTexDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	result = pDevice->CreateDepthStencilView(pDepthTexture, &descDSV, &pDepthStencilView);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create depth stencil view!\n");
		return;
	}
	;
 
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	ConstantBuffer constantBufferPoints = { DirectX::XMMatrixIdentity(),DirectX::XMMatrixIdentity() , DirectX::XMMatrixIdentity(), DirectX::XMVectorZero(),
	DirectX::XMVectorZero(), DirectX::XMVectorZero()};
	D3D11_BUFFER_DESC constantBufDesc;
	constantBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufDesc.MiscFlags = 0;
	constantBufDesc.StructureByteStride = 0;
	constantBufDesc.ByteWidth = sizeof(ConstantBuffer);
	D3D11_SUBRESOURCE_DATA constantData;
	constantData.pSysMem = &constantBufferPoints;
	constantData.SysMemPitch = 0;
	constantData.SysMemSlicePitch = 0;
	result = pDevice->CreateBuffer(&constantBufDesc, &constantData, &pConstantBuffer);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create constant buffer!\n");
		return;
	}


	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create rasterizer state!\n");
		return;
	}

	viewport_depth_directional_light = viewport;
	viewport_depth_directional_light.Width = static_cast<float>(16384);
	viewport_depth_directional_light.Height = static_cast<float>(16384);

	D3D11_TEXTURE2D_DESC depth_texture_descriptor{};
	depth_texture_descriptor.Width = 16384;
	depth_texture_descriptor.Height = 16384;
	depth_texture_descriptor.MipLevels = 1;
	depth_texture_descriptor.ArraySize = 1;
	depth_texture_descriptor.Format = DXGI_FORMAT_R32_TYPELESS;
	depth_texture_descriptor.SampleDesc.Count = 1;
	depth_texture_descriptor.SampleDesc.Quality = 0;
	depth_texture_descriptor.Usage = D3D11_USAGE_DEFAULT;
	depth_texture_descriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	pDevice->CreateTexture2D(&depth_texture_descriptor, nullptr, &background_texture_depth);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	pDevice->CreateDepthStencilView(background_texture_depth, &dsvDesc, &depth_stencil_view_depth_directional_light);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	pDevice->CreateShaderResourceView(background_texture_depth, &srvDesc, &resource_view_depth_directional_light);

	D3D11_SAMPLER_DESC sampler_descriptor;
	sampler_descriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_descriptor.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_descriptor.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_descriptor.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_descriptor.MipLODBias = 0.0f;
	sampler_descriptor.MaxAnisotropy = 1;
	sampler_descriptor.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampler_descriptor.BorderColor[0] = 0;
	sampler_descriptor.BorderColor[1] = 0;
	sampler_descriptor.BorderColor[2] = 0;
	sampler_descriptor.BorderColor[3] = 0;
	sampler_descriptor.MinLOD = 0;
	sampler_descriptor.MaxLOD = D3D11_FLOAT32_MAX;
	pDevice->CreateSamplerState(&sampler_descriptor, &pSamplerLinear);

	sampler_descriptor.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_descriptor.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_descriptor.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	pDevice->CreateSamplerState(&sampler_descriptor, &shadowSample);

	ID3DBlob* vsBuffer = 0;
	bool compileResult = CompileD3DShader("Shadows.hlsl", "VS_Shadow", "vs_5_0", &vsBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"Failed to compile Shadows", 0, 0);
		return;
	}
	result = pDevice->CreateVertexShader(vsBuffer->GetBufferPointer(), vsBuffer->GetBufferSize(), 0, &pVertexShadowShader);
	if (FAILED(result))
	{
		if (vsBuffer)
			vsBuffer->Release();
		return;
	}

	ID3DBlob* psBuffer = 0;
	compileResult = CompileD3DShader("Shadows.hlsl", "PS_Shadow", "ps_5_0", &psBuffer);
	if (compileResult == false)
	{
		MessageBox(0, L"Failed to compile Shadows", 0, 0);
		return;
	}
	result = pDevice->CreatePixelShader(psBuffer->GetBufferPointer(), psBuffer->GetBufferSize(), 0, &pPixelShadowShader);
	psBuffer->Release();
	if (FAILED(result))
	{
		return;
	}

	if (CharacterBall != nullptr)
	{
		pCharacterBall = CharacterBall;
		pCamera->SetCharacter(pCharacterBall);
		pCharacterBall->SetCamera(pCamera);
		pGameComponents.push_back(pCharacterBall);
	}

	for (auto object : pGameComponents)
	{
		object->Initialize();
	}

	result = DirectX::CreateWICTextureFromFile(pDevice, L"Models/PlanetsTextures/1147.jpg", nullptr, &pNewTexture);
	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create texture\n");
		return;
	}
}


void Game::Update(float deltaTime)
{
	if (pCamera != nullptr)
	{
		pCamera->ProcessTransformPosition(deltaTime);
	}

	std::vector<Model*> collidedModels;
	for (auto model : pModelsCollisions)
	{
		if (model->GetCollision().Intersects(*pCharacterBall->GetCollision()))
		{
			model->SetParent(pCharacterBall);
			collidedModels.push_back(model);
		}
	}
	for (auto model : collidedModels)
	{
		pModelsCollisions.erase(model);
	}

	for (auto object : pGameComponents)
	{
		object->Update(deltaTime);;
	}
}

void Game::RenderShadowMap()
{

	pDeviceContext->ClearState();

	lightpos = DirectX::XMVectorSet(0.0f, 20.0f, 0, 1);

	DirectX::XMVECTOR rightDirection = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);

	DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	lightViewMatrix = DirectX::XMMatrixLookAtRH(lightpos, DirectX::XMVectorAdd(lightpos, rightDirection), upDirection);

	lightProjMatrix = DirectX::XMMatrixOrthographicRH(110,110, 0.1f, 1000.0f);

	pDeviceContext->VSSetShader(pVertexShadowShader, 0, 0);
	pDeviceContext->PSSetShader(pPixelShadowShader, 0, 0);
	pDeviceContext->RSSetState(pRasterizerState);
	pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);

	float clearColor[] = { 0.0, 0.0, 0.0, 1 };
	pDeviceContext->RSSetViewports(1, &viewport_depth_directional_light);
	pDeviceContext->ClearDepthStencilView(depth_stencil_view_depth_directional_light, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	pDeviceContext->OMSetRenderTargets(0, nullptr, depth_stencil_view_depth_directional_light);

	renderShadowMap = true;
	Draw();
	renderShadowMap = false;
}

void Game::PrepareFrame(std::chrono::time_point<std::chrono::steady_clock>& PrevTime)
{
	pDeviceContext->ClearState();

	pDeviceContext->PSSetSamplers(0, 1, &pSamplerLinear);
	pDeviceContext->PSSetSamplers(1, 1, &shadowSample);
	pDeviceContext->RSSetState(pRasterizerState);
	pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
	float clearColor[] = { 0.0, 0.0, 0.0, 1 };
	pDeviceContext->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	pDeviceContext->ClearRenderTargetView(pBackBufferTarget, clearColor);
	pDeviceContext->OMSetRenderTargets(1, &pBackBufferTarget, pDepthStencilView);
	pDeviceContext->RSSetViewports(1, &viewport);
	auto curTime = std::chrono::steady_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
	PrevTime = curTime;
}

void Game::Draw()
{
	for (auto object : pGameComponents)
	{
		pDeviceContext->PSSetShaderResources(1, 1, &resource_view_depth_directional_light);
		pDeviceContext->PSSetShaderResources(2, 1, &pNewTexture);
		object->Draw();
	}
}

void Game::Run()
{
	MSG msg = {};

	bool isExitRequested = false;

	std::chrono::time_point<std::chrono::steady_clock> PrevTime = std::chrono::steady_clock::now();

	lightDirection = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);

	while (!isExitRequested) {

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			isExitRequested = true;
		}

		
		Update(deltaTime);
		RenderShadowMap();
		PrepareFrame(PrevTime);
		Draw();
		pSwapChain->Present(1, 0);
	}
}


void Game::DeleteResources()
{
	if (pRasterizerState) pRasterizerState->Release();
	if (pDepthTexture) pDepthTexture->Release();
	if (pDepthStencilView) pDepthStencilView->Release();
	if (pBackBufferTarget) pBackBufferTarget->Release();
	if (pConstantBuffer) pConstantBuffer->Release();
	if (pSwapChain) pSwapChain->Release();
	if (pDeviceContext) pDeviceContext->Release();
	if (pDevice) pDevice->Release();

	pRasterizerState = 0;
	pDepthTexture = 0;
	pDepthStencilView = 0;
	pDeviceContext = 0;
	pDevice = 0;
	pSwapChain = 0;
	pBackBufferTarget = 0;
	pConstantBuffer = 0;
}

Game::~Game()
{
	DeleteResources();
}
