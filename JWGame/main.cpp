#include "JWGame.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

using namespace JWEngine;

static JWGame myGame;

JW_FUNCTION_ON_WINDOWS_KEY_DOWN(OnWindowsKeyDown);
JW_FUNCTION_ON_WINDOWS_CHAR_KEY_PRESSED(OnWindowsCharKeyPressed);
JW_FUNCTION_ON_INPUT(OnInput);
JW_FUNCTION_ON_RENDER(OnRender);

bool g_ShouldDrawNormals{ false };
bool g_ShouldDrawWireframe{ false };

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	myGame.Create(SPositionInt(0, 30), SSizeInt(800, 600), "JWGame", "C:\\Users\\JesusKim\\Documents\\GitHub\\JWEngine11\\", "megt20all");
	//myGame.LoadCursorImage("cursor_default.png");

	myGame.GetCameraObject()
		.SetCameraType(ECameraType::FreeLook)
		.SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));

	myGame.AddOpaqueModel("Decoration_18.obj");
	myGame.GetOpaqueModel(0)
		.SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		.SetScale(XMFLOAT3(0.05f, 0.05f, 0.05f))
		.SetTranslation(XMFLOAT3(10.0f, 0.0f, 0.0f))
		.ShouldBeLit(true);

	myGame.AddOpaqueModel("Ezreal_Idle.X", true);
	myGame.GetOpaqueModel(1)
		.AddAdditionalAnimationFromFile("Ezreal_Punching.X")
		.AddAdditionalAnimationFromFile("Ezreal_Walk.X")
		.SetScale(XMFLOAT3(0.05f, 0.05f, 0.05f))
		.SetAnimation(0);
		//.ShouldBeLit(false);

	myGame.AddImage("Grayscale_Interval_Ten.png");
	myGame.GetImage(0)
		.SetPosition(XMFLOAT2(20, 30));

	myGame.AddLight(SLightData(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f, XMFLOAT3(0, -5, 0)));
	myGame.AddLight(SLightData(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(5, 5, 0), 0.6f));

	myGame.SetOnWindowsKeyDownFunction(OnWindowsKeyDown);
	myGame.SetOnWindowsCharKeyPressedFunction(OnWindowsCharKeyPressed);
	myGame.SetOnInputFunction(OnInput);
	myGame.SetOnRenderFunction(OnRender);

	myGame.Run();

	return 0;
}

JW_FUNCTION_ON_WINDOWS_KEY_DOWN(OnWindowsKeyDown)
{
	if (VK == VK_F1)
	{
		g_ShouldDrawWireframe = !g_ShouldDrawWireframe;
		if (g_ShouldDrawWireframe)
		{
			myGame.SetRasterizerState(ERasterizerState::WireFrame);
		}
		else
		{
			myGame.SetRasterizerState(ERasterizerState::SolidNoCull);
		}
	}

	if (VK == VK_F2)
	{
		g_ShouldDrawNormals = !g_ShouldDrawNormals;
	}
}

JW_FUNCTION_ON_WINDOWS_CHAR_KEY_PRESSED(OnWindowsCharKeyPressed)
{
	if (Character == '1')
	{
		myGame.GetOpaqueModel(1)
			.SetPrevAnimation();
	}

	if (Character == '2')
	{
		myGame.GetOpaqueModel(1)
			.SetNextAnimation();
	}
}

JW_FUNCTION_ON_INPUT(OnInput)
{
	if (DeviceState.Keys[DIK_ESCAPE])
	{
		myGame.Terminate();
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
	myGame.GetOpaqueModel(0)
		.ShouldDrawNormals(g_ShouldDrawNormals);

	myGame.GetOpaqueModel(1)
		.ShouldDrawNormals(g_ShouldDrawNormals)
		//.SetTPose();
		.Animate();

	myGame.DrawDesignerUI();
	myGame.DrawModels();
	myGame.DrawImages();

	//myGame.GetRawPixelSetterObject().GetRawPixelData().FillRect(SPositionInt(100, 150), SSizeInt(50, 80), SRawPixelColor(255, 0, 255, 255));
	//myGame.GetRawPixelSetterObject().Draw();

	myGame.DrawInstantText("FPS: " + ConvertIntToSTRING(myGame.GetFPS()), XMFLOAT2(10, 10), XMFLOAT3(0, 0.2f, 0.8f));
	myGame.DrawInstantText("Test instant text", XMFLOAT2(10, 30), XMFLOAT3(0, 0.5f, 0.7f));
}