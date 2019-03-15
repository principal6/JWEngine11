#include "JWGame.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

using namespace JWEngine;

static JWGame myGame;

void Render();

float rotation_angle{};

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	myGame.Create(800, 600, "JWGame");

	myGame.GetCameraObject().SetPosition(XMFLOAT4(0.0f, 4.0f, -8.0f, 0.0f));

	myGame.GetModelObject().LoadModelObj("..\\Asset\\TestBox.obj");

	myGame.SetRenderFunction(Render);

	//myGame.SetRasterizerState(ERasterizerState::WireFrame);
	myGame.SetRasterizerState(ERasterizerState::SolidNoCull);
	//myGame.SetRasterizerState(ERasterizerState::SolidBackCullCW);

	myGame.Run();

	return 0;
}

void Render()
{
	rotation_angle += 0.0002f;
	if (rotation_angle >= XM_PI * 2)
	{
		rotation_angle = 0;
	}

	myGame.SetBlendState(EBlendState::Transprent);

	myGame.GetModelObject()
		.SetScale(XMFLOAT3(0.08f, 0.08f, 0.08f))
		.SetRotation(XMFLOAT4(0, 1, 0, 0), rotation_angle)
		.Draw();

	myGame.GetModelObject()
		.SetScale(XMFLOAT3(0.08f, 0.08f, 0.08f))
		.SetTranslation(XMFLOAT3(4, 0, 0))
		.SetRotation(XMFLOAT4(0, 1, 0, 0), -rotation_angle)
		.Draw();
}