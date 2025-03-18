#include "../Header/Pong.h"
#include "../Header/GameStick.h"
#include <chrono>
#include "iostream"


struct ConstantBufferPong
{
	DirectX::XMFLOAT4 offset;
	DirectX::XMFLOAT4 size;
};


Pong::Pong()
{

}

void Pong::InitializeGame()
{
	DirectX::XMFLOAT2 firstPaddlePosition = { -0.99f, 0.0f };
	DirectX::XMFLOAT2 secondPaddlePosition = { 0.99f, 0.0f };
	DirectX::XMFLOAT2 ballPosition = { 0.0f, 0.0f };

	pPaddle1 = new GameStick(this, firstPaddlePosition, this);
	pPaddle2 = new GameStick(this, secondPaddlePosition, this);
	pBall = new GameBall(this, ballPosition, this);
	pBallsVector.push_back(pBall);

	UpWall.Center = { 0.0f, 1.0f, 0.0f };
	UpWall.Extents = { 1.0f, 0.01f, 0.0f };
	DownWall.Center = { 0.0f, -1.0f, 0.0f };
	DownWall.Extents = { 1.0f, 0.01f, 0.0f };
	LeftWall.Center = { -1.0f, 0.0f, 0.0f };
	LeftWall.Extents = { 0.01f, 1.0f, 0.0f };
	RightWall.Center = { 1.0f, 0.0f, 0.0f };
	RightWall.Extents = { 0.01f, 1.0f, 0.0f };
	PushGameComponents(pPaddle1);
	PushGameComponents(pPaddle2);
	PushGameComponents(pBall);

	ConstantBufferPong constantBufferPoints = { DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) };
	D3D11_BUFFER_DESC constantBufDesc;
	constantBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBufDesc.MiscFlags = 0;
	constantBufDesc.StructureByteStride = 0;
	constantBufDesc.ByteWidth = sizeof(ConstantBufferPong);
	D3D11_SUBRESOURCE_DATA constantData;
	constantData.pSysMem = &constantBufferPoints;
	constantData.SysMemPitch = 0;
	constantData.SysMemSlicePitch = 0;

	HRESULT result = pDevice->CreateBuffer(&constantBufDesc, &constantData, &pConstantBufferPong);

	if (FAILED(result))
	{
		OutputDebugString(L"Failed to create constant buffer!\n");
		return;
	}

	pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBufferPong);
}

void Pong::Run()
{
	MSG msg = {};

	bool isExitRequested = false;

	std::chrono::time_point<std::chrono::steady_clock> PrevTime = std::chrono::steady_clock::now();

	for (auto object : pGameComponents)
	{
		object->Initialize();
	}

	InputDevice::KeyboardInputEventArgs args;

	while (!isExitRequested) {
		// Handle the windows messages.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

			args.VKey = msg.wParam;
			pInputDevice->OnKeyDown(args);

			if (msg.message == WM_KEYDOWN) {

				args.Flags = msg.message;
				pInputDevice->OnKeyDown(args);
			}
			else if (msg.message == WM_KEYUP) {

				args.Flags = msg.message;
				pInputDevice->OnKeyDown(args);

			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}


		if (msg.message == WM_QUIT) {
			isExitRequested = true;
		}

		float clearColor[] = { 0.0, 0.0, 0.0, 1 };
		pDeviceContext->ClearRenderTargetView(pBackBufferTarget, clearColor);

		auto curTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		pDeviceContext->OMSetRenderTargets(1, &pBackBufferTarget, nullptr);

		if (pInputDevice->IsKeyDown(Keys::W))
		{
			pPaddle1->ChangeState(Direction::UP);
		}
		else if (pInputDevice->IsKeyDown(Keys::S))
		{
			pPaddle1->ChangeState(Direction::DOWN);
		}
		else
		{
			pPaddle1->ChangeState(Direction::STOP);
		}


		if (pInputDevice->IsKeyDown(Keys::T))
		{
			pPaddle2->ChangeState(Direction::UP);
		}
		else if (pInputDevice->IsKeyDown(Keys::G))
		{
			pPaddle2->ChangeState(Direction::DOWN);
		}
		else
		{
			pPaddle2->ChangeState(Direction::STOP);
		}

		
		GameBall* NewBall1 = nullptr; 
		GameBall* NewBall2 = nullptr;

		std::vector<GameBall*> newBallsVector;

		for (auto ball : pBallsVector)
		{
			if (ball->GetCollision().Intersects(pPaddle1->GetCollision()))
			{
					ball->ChangeDirection(pPaddle1->GetPosition().y, 0.2f, true);

					DirectX::XMFLOAT2 newDirection1 = ball ->GetDirection();
					newDirection1.y *= -0.5f;
					GameBall* NewBall1 = new GameBall(this, ball->GetPosition(), this, newDirection1, ball->GetSpeed()/2);

					DirectX::XMFLOAT2 newDirection2 = ball->GetDirection();
					newDirection2.y *= 0.5f;
					GameBall* NewBall2 = new GameBall(this, ball->GetPosition(), this, newDirection2, ball->GetSpeed() /2.5);

					NewBall1->Initialize();
					NewBall2->Initialize();

					newBallsVector.push_back(NewBall1);
					newBallsVector.push_back(NewBall2);
			}
			else if (ball->GetCollision().Intersects(pPaddle2->GetCollision()))
			{
				
					ball->ChangeDirection(pPaddle2->GetPosition().y, 0.2f, false);

					DirectX::XMFLOAT2 newDirection = ball->GetDirection();
					newDirection.y += 0.5f;
					GameBall* NewBall1 = new GameBall(this, ball->GetPosition(), this, newDirection, ball->GetSpeed() / 2);

					DirectX::XMFLOAT2 newDirection2 = ball->GetDirection();
					newDirection2.y += -0.5f;
					GameBall* NewBall2 = new GameBall(this, ball->GetPosition(), this, newDirection2, ball->GetSpeed() / 2.5);

					NewBall1->Initialize();
					NewBall2->Initialize();

					newBallsVector.push_back(NewBall1);
					newBallsVector.push_back(NewBall2);
			}
			else if (ball->GetCollision().Intersects(UpWall) || ball->GetCollision().Intersects(DownWall))
			{
				ball->ChangeUpDown(deltaTime);
			}
			else if (ball->GetCollision().Intersects(LeftWall))
			{
				for (auto object : pGameComponents)
				{
					object->Reload();
				}
				std::cout << "Player_1: " << Player_1 << " Player_2: " << ++Player_2 << std::endl;
				break;
			}
			else if (ball->GetCollision().Intersects(RightWall))
			{
				for (auto object : pGameComponents)
				{
					object->Reload();
				}
				std::cout << "Player_1: " << ++Player_1 << " Player_2: " << Player_2 << std::endl;
				break;
			}
		}

		for(auto ball : newBallsVector)
		{
			pBallsVector.push_back(ball);
			pGameComponents.push_back(ball);
		}

		for (auto it = pBallsVector.begin(); it != pBallsVector.end(); )
		{
			if ((*it)->GetDead()) {

				auto deleteBall = static_cast<GameComponent*>(*it);

				pGameComponents.erase(std::remove(pGameComponents.begin(), pGameComponents.end(), deleteBall), pGameComponents.end());

				delete* it;
				it = pBallsVector.erase(it);
			}
			else 
			{
				++it;
			}
		}

		for (auto object : pGameComponents)
		{
			object->Update(deltaTime);
			object->Draw();
		}
		pSwapChain->Present(1, 0);
	}

}

void Pong::ChangeConstantBufferPong(DirectX::XMFLOAT4 offset, DirectX::XMFLOAT4 size)
{
	ConstantBufferPong constBuf = {offset, size};

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pDeviceContext->Map(pConstantBufferPong, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &constBuf, sizeof(ConstantBufferPong));
	pDeviceContext->Unmap(pConstantBufferPong, 0);
}

void Pong::DeleteResources()
{
	if (pConstantBuffer) pConstantBufferPong->Release();
	pConstantBufferPong = 0;

	if (pConstantBuffer) pConstantBufferPong->Release();
	pConstantBuffer = 0;

	if (pBackBufferTarget) pBackBufferTarget->Release();
	pBackBufferTarget = 0;

	if (pSwapChain) pSwapChain->Release();
	pSwapChain = 0;

	if (pDeviceContext) pDeviceContext->Release();
	pDeviceContext = 0;

	if (pDevice) pDevice->Release();
	pDevice = 0;

	if (pRasterizerState) pRasterizerState->Release();
	pRasterizerState = 0;

	if (pDepthTexture) pDepthTexture->Release();
	pDepthTexture = 0;

	if (pDepthStencilView) pDepthStencilView->Release();
	pDepthStencilView = 0;

	if (pConstantBuffer) pConstantBuffer->Release();
	pConstantBuffer = 0;

	for (auto object : pGameComponents)
	{
		delete object;
	}
}

Pong::~Pong()
{
	DeleteResources();
}
