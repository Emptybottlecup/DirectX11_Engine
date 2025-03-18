#pragma once
#include "BoxGameComponent.h"

class GameBall : public BoxGameComponent
{
public:
	GameBall(Game* GameObject, const DirectX::XMFLOAT2& position, Pong* pong);

	GameBall(Game* GameObject, const DirectX::XMFLOAT2& position, Pong* pong, DirectX::XMFLOAT2 NewDirection, float Speed);

	void Update(float deltaTime);

	void ChangeDirection(float PaddleCenterY, float PaddleHeight, bool LeftRight);

	void Reload() override;
	void ChangeUpDown(float deltatime);

	DirectX::XMFLOAT2 GetPosition();

	DirectX::XMFLOAT2 GetDirection();

	bool GetNumber();

	bool GetDead();

	void SetDead(bool Dead);

	float GetSpeed();

	~GameBall();
private:
	DirectX::XMFLOAT2 direction = { -1.0f, 0.0f };
	DirectX::XMFLOAT2 pPosition;
	float pSpeed = 0.5f;

	bool first = true;
	bool dead = false;
};
