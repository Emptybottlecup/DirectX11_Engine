#include "../Header/GameBall.h"
#include "../Header/Pong.h"
#define M_PI 3.141592653589793238462643383279502884L

GameBall::GameBall(Game* GameObject, const DirectX::XMFLOAT2& position, Pong* pong) : BoxGameComponent(GameObject, pong), pPosition(position)
{
	std::vector<DirectX::XMFLOAT4> Points =
	{
		DirectX::XMFLOAT4(position.x - 0.025f,position.y + 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + 0.025f,position.y + 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x - 0.025f, position.y - 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(position.x + 0.025f, position.y - 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	pPoints = Points;

	boundingBox.Center = { pPosition.x, pPosition.y, 0 };
	boundingBox.Extents = { 0.025f, 0.025f,0 };

}

GameBall::GameBall(Game* GameObject, const DirectX::XMFLOAT2& position, Pong* pong, DirectX::XMFLOAT2 NewDirection, float Speed) : BoxGameComponent(GameObject, pong), first(false),  pPosition(position), direction(NewDirection), pSpeed(Speed)
{
	std::vector<DirectX::XMFLOAT4> Points =
	{
		DirectX::XMFLOAT4(0 - 0.025f,0 + 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0 + 0.025f,0 + 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0 - 0.025f,0 - 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0 + 0.025f,0 - 0.025f, 0.5f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)
	};
	pPoints = Points;

	boundingBox.Center = { pPosition.x, pPosition.y, 0 };
	boundingBox.Extents = { 0.025f, 0.025f,0 };
}

void GameBall::Update(float deltaTime)
{
	pPosition.x += direction.x * deltaTime * pSpeed;
	pPosition.y += direction.y * deltaTime * pSpeed;
	boundingBox.Center = { pPosition.x, pPosition.y, 0 };
	pPong->ChangeConstantBufferPong(DirectX::XMFLOAT4(pPosition.x , pPosition.y, 0, 0), DirectX::XMFLOAT4(1, 1, 1, 1));
}

void GameBall::ChangeDirection(float PaddleCenterY, float PaddleHeight,bool LeftRight)
{
	float impactPoint = pPosition.y - PaddleCenterY;

	float normalizedImpactPoint = impactPoint / (PaddleHeight / 2);

	float reflectionAngleRadians = normalizedImpactPoint * (50.0f * (M_PI / 180.0f));

	if (LeftRight)
	{
		direction.x = cos(reflectionAngleRadians);
	}
	else
	{
		direction.x = -cos(reflectionAngleRadians);
	}
	direction.y = sin(reflectionAngleRadians);

	DirectX::XMVECTOR vector = XMLoadFloat2(&direction);
	DirectX::XMVECTOR normalizedVector = DirectX::XMVector2Normalize(vector);
	XMStoreFloat2(&direction, normalizedVector);
	pPosition.x += pSpeed * direction.x * 0.05f;
}

void GameBall::Reload()
{
	if (first)
	{
		direction.y = 0;
		direction.x *= -1;
		pPosition.x = 0;
		pPosition.y = 0;
		boundingBox.Center = { pPosition.x, pPosition.y, 0 };
	}
	else
	{
		dead = true;
	}
}

void GameBall::ChangeUpDown(float deltatime)
{
	direction.y *= -1;
	pPosition.y += pSpeed * direction.y * 0.05f;
}

DirectX::XMFLOAT2 GameBall::GetPosition()
{
	return pPosition;
}

DirectX::XMFLOAT2 GameBall::GetDirection()
{
	return direction;
}


bool GameBall::GetNumber()
{
	return first;
}

float GameBall::GetSpeed()
{
	return pSpeed;
}

void GameBall::SetDead(bool Dead)
{
	dead = Dead;
}

bool GameBall::GetDead()
{
	return dead;
}

GameBall::~GameBall()
{

}
