#include "JWGame.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

using namespace JWEngine;

static JWGame myGame;

JW_FUNCTION_ON_WINDOWS_KEY_DOWN(OnWindowsKeyDown);
JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(OnWindowsCharKeyInput);
JW_FUNCTION_ON_INPUT(OnInput);
JW_FUNCTION_ON_RENDER(OnRender);

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	myGame.Create(SPositionInt(0, 30), SSizeInt(800, 600), "JWGame", "C:\\Users\\JesusKim\\Documents\\GitHub\\JWEngine11\\", "megt20all");
	//myGame.LoadCursorImage("cursor_default.png");

	myGame.Camera()
		.SetCameraType(ECameraType::FreeLook)
		.SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));

	auto& prop = myGame.ECS().CreateEntity();
	prop.CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	prop.CreateComponentRender()
		->LoadModel(EComponentRenderType::Model_StaticModel, "Decoration_18.obj")
		->SetRenderFlag(JWFlagRenderOption_UseLighting);
		//->Load(EComponentRenderType::Model_StaticModel, "TestBox.obj")
		//->SetRenderFlag(JWFlagRenderOption_UseTexture | JWFlagRenderOption_UseLighting | JWFlagRenderOption_UseTransparency);

	auto& main_sprite = myGame.ECS().CreateEntity();
	main_sprite.CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(-10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	main_sprite.CreateComponentRender()
		->LoadModel(EComponentRenderType::Model_RiggedModel, "Ezreal_Idle.X")
		->SetRenderFlag(JWFlagRenderOption_UseTexture | JWFlagRenderOption_UseLighting | JWFlagRenderOption_UseAnimationInterpolation)
		->AddAnimation("Ezreal_Punching.X")
		->AddAnimation("Ezreal_Walk.X")
		->SetAnimation(0);
	
	auto& ambient_light = myGame.ECS().CreateEntity();
	ambient_light.CreateComponentLight()
		->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);
	ambient_light.CreateComponentTransform()
		->SetPosition(XMFLOAT3(0.0f, -5.0f, 0.0f));
	ambient_light.CreateComponentRender()
		->LoadModel(EComponentRenderType::Model_StaticModel, "lightbulb.obj");

	auto& directional_light = myGame.ECS().CreateEntity();
	directional_light.CreateComponentLight()
		->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(5.0f, 5.0f, 0.0f), 0.6f);
	directional_light.CreateComponentTransform()
		->SetPosition(XMFLOAT3(8.0f, 8.0f, 0.0f));
	directional_light.CreateComponentRender()
		->LoadModel(EComponentRenderType::Model_StaticModel, "lightbulb.obj");

	auto& primitive_cube = myGame.ECS().CreateEntity();
	primitive_cube.CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	primitive_cube.CreateComponentRender()
		->MakeCylinder(2.0f, 1.0f, 2);
	
	myGame.AddImage("Grayscale_Interval_Ten.png");
	myGame.GetImage(0).SetPosition(XMFLOAT2(20, 30));

	myGame.SetFunctionOnWindowsKeyDown(OnWindowsKeyDown);
	myGame.SetFunctionOnWindowsCharInput(OnWindowsCharKeyInput);
	myGame.SetFunctionOnInput(OnInput);
	myGame.SetFunctionOnRender(OnRender);

	myGame.Run();

	return 0;
}

JW_FUNCTION_ON_WINDOWS_KEY_DOWN(OnWindowsKeyDown)
{
	if (VK == VK_F1)
	{
		myGame.ToggleWireFrame();
	}

	if (VK == VK_F2)
	{
		myGame.ECS().GetEntity(0)->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_DrawNormals);
		myGame.ECS().GetEntity(1)->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_DrawNormals);
	}

	if (VK == VK_F3)
	{
		myGame.ECS().GetEntity(0)->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_UseLighting);
		myGame.ECS().GetEntity(1)->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_UseLighting);
	}
}

JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(OnWindowsCharKeyInput)
{
	if (Character == '1')
	{
		myGame.ECS().GetEntity(1)->GetComponentRender()->NextAnimation();
	}

	if (Character == '2')
	{
		myGame.ECS().GetEntity(1)->GetComponentRender()->PrevAnimation();
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
		myGame.Camera().MoveCamera(ECameraMoveDirection::Forward, 1.0f);
	}

	if (DeviceState.Keys[DIK_S])
	{
		myGame.Camera().MoveCamera(ECameraMoveDirection::Backward, 1.0f);
	}

	if (DeviceState.Keys[DIK_A])
	{
		myGame.Camera().MoveCamera(ECameraMoveDirection::Left, 1.0f);
	}

	if (DeviceState.Keys[DIK_D])
	{
		myGame.Camera().MoveCamera(ECameraMoveDirection::Right, 1.0f);
	}

	if ((DeviceState.CurrentMouse.lX != DeviceState.PreviousMouse.lX) || (DeviceState.CurrentMouse.lY != DeviceState.PreviousMouse.lY))
	{
		if (DeviceState.CurrentMouse.rgbButtons[1])
		{
			myGame.Camera().RotateCamera(static_cast<float>(DeviceState.CurrentMouse.lY), static_cast<float>(DeviceState.CurrentMouse.lX), 0);
		}
	}
	
	if ((DeviceState.CurrentMouse.lZ))
	{
		myGame.Camera().ZoomCamera(static_cast<float>(DeviceState.CurrentMouse.lZ));
	}
}

JW_FUNCTION_ON_RENDER(OnRender)
{
	myGame.DrawDesignerUI();

	myGame.UpdateEntities();

	myGame.DrawImages();

	myGame.DrawInstantText("FPS: " + ConvertIntToSTRING(myGame.GetFPS()), XMFLOAT2(10, 10), XMFLOAT3(0, 0.2f, 0.8f));
	myGame.DrawInstantText("Test instant text", XMFLOAT2(10, 30), XMFLOAT3(0, 0.5f, 0.7f));
}