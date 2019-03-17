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
	
	myGame.Create(SPositionInt(0, 30), SSizeInt(800, 600), "JWGame", "C:\\Users\\JesusKim\\Documents\\GitHub\\JWEngine11\\");

	myGame.GetCameraObject().SetPosition(XMFLOAT4(0.0f, 4.0f, -6.0f, 0.0f));

	myGame.AddTransparentModel("Asset\\", "TestBox.obj");
	myGame.GetTransparentModel(0)
		.SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		.SetScale(XMFLOAT3(0.08f, 0.08f, 0.08f));

	myGame.AddTransparentModel("Asset\\", "TestBox.obj");
	myGame.GetTransparentModel(1)
		.SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleTransRot)
		.SetScale(XMFLOAT3(0.08f, 0.08f, 0.08f))
		.SetTranslation(XMFLOAT3(4, 0, 0));

	myGame.AddImage2D("Asset\\", "test.png");
	myGame.GetImage2D(0)
		.SetPosition(XMFLOAT2(50, 50))
		.SetSize(XMFLOAT2(200, 200));

	myGame.SetRenderFunction(Render);

	//myGame.SetRasterizerState(ERasterizerState::WireFrame);
	//myGame.SetRasterizerState(ERasterizerState::SolidBackCullCW);
	myGame.SetRasterizerState(ERasterizerState::SolidNoCull);

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

	myGame.GetTransparentModel(0).SetRotation(XMFLOAT4(0, 1, 0, 0), rotation_angle);
	myGame.GetTransparentModel(1).SetRotation(XMFLOAT4(0, 1, 0, 0), -rotation_angle);

	myGame.DrawAll();
}