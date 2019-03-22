#include "JWGame.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

using namespace JWEngine;

static JWGame myGame;

JW_FUNCTION_ON_INPUT(OnInput);
JW_FUNCTION_ON_RENDER(OnRender);

bool g_ShouldDrawNormals{ false };
float g_RotationAngle{};

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	myGame.Create(SPositionInt(0, 30), SSizeInt(800, 600), "JWGame", "C:\\Users\\JesusKim\\Documents\\GitHub\\JWEngine11\\", "megt20all");
	//myGame.LoadCursorImage("cursor_default.png");

	myGame.GetCameraObject()
		.SetCameraType(ECameraType::FreeLook)
		.SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));

	myGame.AddOpaqueModel("sphere.obj");
	myGame.GetOpaqueModel(0)
		.SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		.SetScale(XMFLOAT3(0.1f, 0.1f, 0.1f));
	
	myGame.AddImage("Grayscale_Interval_Ten.png");
	myGame.GetImage(0)
		.SetPosition(XMFLOAT2(20, 30));

	myGame.AddLight(SLightData(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.3f, XMFLOAT3(0, -5, 0)));
	myGame.AddLight(SLightData(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(5, 5, 0), 0.8f));

	myGame.SetOnInputFunction(OnInput);
	myGame.SetOnRenderFunction(OnRender);

	myGame.Run();

	return 0;
}

JW_FUNCTION_ON_INPUT(OnInput)
{
	if (DeviceState.Keys[DIK_N])
	{
		g_ShouldDrawNormals = !g_ShouldDrawNormals;
	}

	if (DeviceState.Keys[DIK_W])
	{
		myGame.GetCameraObject().MoveCamera(ECameraMoveDirection::Forward, 1.0f);
	}

	if (DeviceState.Keys[DIK_S])
	{
		myGame.GetCameraObject().MoveCamera(ECameraMoveDirection::Backward, 1.0f);
	}

	if (DeviceState.Keys[DIK_A])
	{
		myGame.GetCameraObject().MoveCamera(ECameraMoveDirection::Left, 1.0f);
	}

	if (DeviceState.Keys[DIK_D])
	{
		myGame.GetCameraObject().MoveCamera(ECameraMoveDirection::Right, 1.0f);
	}

	if ((DeviceState.CurrentMouse.lX != DeviceState.PreviousMouse.lX) || (DeviceState.CurrentMouse.lY != DeviceState.PreviousMouse.lY))
	{
		if (DeviceState.CurrentMouse.rgbButtons[1])
		{
			myGame.GetCameraObject().RotateCamera(static_cast<float>(DeviceState.CurrentMouse.lY), static_cast<float>(DeviceState.CurrentMouse.lX), 0);
		}
	}
	
	if ((DeviceState.CurrentMouse.lZ))
	{
		myGame.GetCameraObject().ZoomCamera(static_cast<float>(DeviceState.CurrentMouse.lZ));
	}
}

JW_FUNCTION_ON_RENDER(OnRender)
{
	g_RotationAngle += 0.0005f;
	if (g_RotationAngle >= XM_PI * 2)
	{
		g_RotationAngle = 0;
	}

	myGame.GetOpaqueModel(0)
		.SetRotation(XMFLOAT4(0, 1, 0, 0), g_RotationAngle)
		.ShouldDrawNormals(g_ShouldDrawNormals);

	myGame.DrawDesignerUI();
	myGame.DrawModels();
	myGame.DrawImages();

	myGame.DrawInstantText("FPS: " + ConvertIntToSTRING(myGame.GetFPS()), XMFLOAT2(10, 10), XMFLOAT3(0, 0.2f, 0.8f));
	myGame.DrawInstantText("Test instant text", XMFLOAT2(10, 30), XMFLOAT3(0, 0.5f, 0.7f));
}