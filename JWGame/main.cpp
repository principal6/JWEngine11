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

	// Create shared resource for ECS
	// Texture source: http://www.custommapmakers.org/skyboxes.php
	myGame.ECS().CreateSharedResource(ESharedResourceType::TextureCubeMap, "skymap.dds"); // SharedResource #0
	myGame.ECS().CreateSharedResource(ESharedResourceType::Texture2D, "grass.png"); //SharedResource #1
	myGame.ECS().CreateSharedResource(ESharedResourceType::Texture2D, "Grayscale_Interval_Ten.png"); //SharedResource #2
	myGame.ECS().CreateAnimationTexture(SSizeInt(KColorCountPerTexel * KMaxBoneCount, 400)); //AnimationTexture #0

	auto& jars = myGame.ECS().CreateEntity();
	jars.CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	jars.CreateComponentRender()
		->LoadModel(ERenderType::Model_StaticModel, "Decoration_18.obj")
		->SetRenderFlag(JWFlagRenderOption_UseLighting);

	auto& main_sprite = myGame.ECS().CreateEntity();
	main_sprite.CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(-10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	main_sprite.CreateComponentRender()
		->LoadModel(ERenderType::Model_RiggedModel, "Ezreal_Idle.X")
		->SetRenderFlag(JWFlagRenderOption_UseTexture | JWFlagRenderOption_UseLighting | JWFlagRenderOption_UseAnimationInterpolation)
		->AddAnimation("Ezreal_Punching.X")
		->AddAnimation("Ezreal_Walk.X")
		->BakeAnimationsIntoTexture(myGame.ECS().GetAnimationTexture(0))
		->SaveBakedAnimationAsTIF("baked_animation.tif")
		->SetAnimation(0);
	
	auto& ambient_light = myGame.ECS().CreateEntity();
	ambient_light.CreateComponentLight()
		->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);
	ambient_light.CreateComponentTransform()
		->SetPosition(XMFLOAT3(0.0f, -5.0f, 0.0f));
	ambient_light.CreateComponentRender()
		->LoadModel(ERenderType::Model_StaticModel, "lightbulb.obj");

	auto& directional_light = myGame.ECS().CreateEntity();
	directional_light.CreateComponentLight()
		->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(5.0f, 5.0f, 0.0f), 0.6f);
	directional_light.CreateComponentTransform()
		->SetPosition(XMFLOAT3(8.0f, 8.0f, 0.0f));
	directional_light.CreateComponentRender()
		->LoadModel(ERenderType::Model_StaticModel, "lightbulb.obj");
	
	auto& sky_sphere = myGame.ECS().CreateEntity();
	sky_sphere.CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	sky_sphere.CreateComponentRender()
		->MakeSphere(100.0f, 16, 7)
		->SetVertexShader(EVertexShader::VSSkyMap)
		->SetPixelShader(EPixelShader::PSSkyMap)
		->SetTexture(myGame.ECS().GetSharedResource(0));

	auto& floor_plane = myGame.ECS().CreateEntity();
	floor_plane.CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	floor_plane.CreateComponentRender()
		->MakeSquare(10.0f, XMFLOAT2(10.0f, 10.0f))
		->SetTexture(myGame.ECS().GetSharedResource(1));

	auto& image_gamma = myGame.ECS().CreateEntity();
	image_gamma.CreateComponentRender()
		->MakeImage2D(SPositionInt(160, 10), SSizeInt(100, 40))
		->SetTexture(myGame.ECS().GetSharedResource(2));

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
	myGame.ECS().GetEntity(4)->GetComponentTransform()->SetPosition(XMFLOAT3(
		myGame.Camera().GetPositionFloat4().x, myGame.Camera().GetPositionFloat4().y, myGame.Camera().GetPositionFloat4().z));

	myGame.DrawDesignerUI();

	myGame.UpdateEntities();

	myGame.DrawInstantText("FPS: " + ConvertIntToSTRING(myGame.GetFPS()), XMFLOAT2(10, 10), XMFLOAT3(0, 0.2f, 0.8f));
	myGame.DrawInstantText("Test instant text", XMFLOAT2(10, 30), XMFLOAT3(0, 0.5f, 0.7f));
}