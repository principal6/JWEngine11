#include "JWGame.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

using namespace JWEngine;

static JWGame myGame;
JWLogger myLogger;

JW_FUNCTION_ON_WINDOWS_KEY_DOWN(OnWindowsKeyDown);
JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(OnWindowsCharKeyInput);
JW_FUNCTION_ON_INPUT(OnInput);
JW_FUNCTION_ON_RENDER(OnRender);

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// TODO:
	// Instancing!

	myLogger.InitializeTime();

	myGame.Create(SPositionInt(0, 30), SSizeInt(800, 600), "JWGame", "megt20all", &myLogger);
	//myGame.LoadCursorImage("cursor_default.png");

	myGame.Camera()
		.SetCameraType(ECameraType::FreeLook)
		.SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));

	// Create shared resource for ECS
	myGame.ECS().CreateSharedModelFromFile(ESharedModelType::StaticModel, "Decoration_18.obj"); // Shared Model #0
	myGame.ECS().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_light.obj"); // Shared Model #1
	myGame.ECS().CreateSharedModelFromFile(ESharedModelType::RiggedModel, "Ezreal_Idle.X") // Shared Model #2
		.AddAnimationToModelFromFile(2, "Ezreal_Punching.X")
		.AddAnimationToModelFromFile(2, "Ezreal_Walk.X");
		//.BakeAnimationTextureToFile(2, SSizeInt(KColorCountPerTexel * KMaxBoneCount, 400), "baked_animation.dds");

	myGame.ECS().CreateSharedModelSphere(100.0f, 16, 7); // Shared Model #3
	myGame.ECS().CreateSharedModelSquare(10.0f, XMFLOAT2(10.0f, 10.0f)); // Shared Model #4
	myGame.ECS().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_camera.obj"); // Shared Model #5

	myGame.ECS().CreateSharedImage2D(SPositionInt(160, 10), SSizeInt(100, 40)); // Shared Image2D #0

	myGame.ECS().CreateSharedLineModel() // Shared LineModel #0
		->Make3DGrid(50.0f, 50.0f, 1.0f);

	myGame.ECS().CreateSharedLineModel() // Shared LineModel #1 (Ray-cast representation)
		->AddLine3D(XMFLOAT3(0, 0, 0), XMFLOAT3(2, 2, 2), XMFLOAT4(1, 0, 0, 1))
		->AddEnd();

	myGame.ECS().CreateSharedResource(ESharedResourceType::TextureCubeMap, "skymap.dds"); // Shared Resource #0
	myGame.ECS().CreateSharedResource(ESharedResourceType::Texture2D, "grass.png"); //Shared Resource #1
	myGame.ECS().CreateSharedResource(ESharedResourceType::Texture2D, "Grayscale_Interval_Ten.png"); //Shared Resource #2
	myGame.ECS().CreateSharedResourceFromSharedModel(2); //Shared Resource #3

	myGame.ECS().CreateAnimationTextureFromFile("baked_animation.dds"); //AnimationTexture #0

	auto grid = myGame.ECS().CreateEntity("grid");
	grid->CreateComponentRender()
		->SetLineModel(myGame.ECS().GetSharedLineModel(0));

	auto jars = myGame.ECS().CreateEntity("jars");
	jars->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	jars->CreateComponentRender()
		->SetModel(myGame.ECS().GetSharedModel(0))
		->SetRenderFlag(JWFlagRenderOption_UseLighting);

	auto ambient_light = myGame.ECS().CreateEntity("ambient_light");
	ambient_light->CreateComponentLight()
		->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);
	ambient_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0.0f, -5.0f, 0.0f));
	ambient_light->CreateComponentRender()
		->SetModel(myGame.ECS().GetSharedModel(1));

	auto directional_light = myGame.ECS().CreateEntity("directional_light");
	directional_light->CreateComponentLight()
		->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(5.0f, 5.0f, 0.0f), 0.6f);
	directional_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(8.0f, 8.0f, 0.0f));
	directional_light->CreateComponentRender()
		->SetModel(myGame.ECS().GetSharedModel(1));

	auto main_sprite = myGame.ECS().CreateEntity("main_sprite");
	main_sprite->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(-10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	main_sprite->CreateComponentRender()
		->SetModel(myGame.ECS().GetSharedModel(2))
		->SetTexture(myGame.ECS().GetSharedResource(3))
		->SetRenderFlag(JWFlagRenderOption_UseTexture | JWFlagRenderOption_UseLighting | JWFlagRenderOption_UseAnimationInterpolation)
		->SetAnimationTexture(myGame.ECS().GetAnimationTexture(0))
		->SetAnimation(3);
	
	auto sky_sphere = myGame.ECS().CreateEntity("Sky");
	sky_sphere->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	sky_sphere->CreateComponentRender()
		->SetModel(myGame.ECS().GetSharedModel(3))
		->SetTexture(myGame.ECS().GetSharedResource(0))
		->SetVertexShader(EVertexShader::VSSkyMap)
		->SetPixelShader(EPixelShader::PSSkyMap);

	auto floor_plane = myGame.ECS().CreateEntity("Floor");
	floor_plane->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	floor_plane->CreateComponentRender()
		->SetModel(myGame.ECS().GetSharedModel(4))
		->SetTexture(myGame.ECS().GetSharedResource(1));

	auto image_gamma = myGame.ECS().CreateEntity("IMG_Gamma");
	image_gamma->CreateComponentRender()
		->SetImage2D(myGame.ECS().GetSharedImage2D(0))
		->SetTexture(myGame.ECS().GetSharedResource(2));

	auto cam = myGame.ECS().CreateEntity("Camera");
	cam->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0, 2, 0));
	cam->CreateComponentRender()
		->SetModel(myGame.ECS().GetSharedModel(5));

	auto ray = myGame.ECS().CreateEntity("Ray");
	ray->CreateComponentRender()
		->SetLineModel(myGame.ECS().GetSharedLineModel(1));

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
		myGame.ECS().GetEntityByName("jars")->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_DrawNormals);
		myGame.ECS().GetEntityByName("main_sprite")->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_DrawNormals);
	}

	if (VK == VK_F3)
	{
		myGame.ECS().GetEntityByName("jars")->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_UseLighting);
		myGame.ECS().GetEntityByName("main_sprite")->GetComponentRender()->ToggleRenderFlag(JWFlagRenderOption_UseLighting);
	}
}

JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(OnWindowsCharKeyInput)
{
	if (Character == '1')
	{
		myGame.ECS().GetEntityByName("main_sprite")->GetComponentRender()->PrevAnimation();
	}

	if (Character == '2')
	{
		myGame.ECS().GetEntityByName("main_sprite")->GetComponentRender()->NextAnimation();
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
	// Skybox
	myGame.ECS().GetEntityByName("Sky")->GetComponentTransform()->SetPosition(XMFLOAT3(
		myGame.Camera().GetPositionFloat4().x, myGame.Camera().GetPositionFloat4().y, myGame.Camera().GetPositionFloat4().z));

	XMFLOAT3 ray_dest{};
	auto cam_pos_vec = myGame.Camera().GetPosition();
	XMMATRIX mat_rot = XMMatrixRotationY(XM_PIDIV4);
	cam_pos_vec = XMVector3Transform(cam_pos_vec, mat_rot);
	XMStoreFloat3(&ray_dest, cam_pos_vec);
	
	// Ray
	myGame.ECS().GetEntityByName("Ray")->GetComponentRender()->PtrLine
		->SetLine3D(0, XMFLOAT3(0, 0, 0), ray_dest, XMFLOAT4(1, 0, 0, 1))
		->UpdateLines();

	// ECS
	myGame.ECS().ExecuteSystems();

	// Sprite info
	const auto& anim_state = myGame.ECS().GetEntityByName("main_sprite")->GetComponentRender()->AnimationState;
	
	// Text
	static WSTRING s_temp{};
	static WSTRING s_fps{};
	static WSTRING s_anim_id{};
	static WSTRING s_ray{};

	s_fps = L"FPS: " + ConvertIntToWSTRING(myGame.GetFPS(), s_temp);
	s_anim_id = L"Animation ID: " + ConvertIntToWSTRING(anim_state.CurrAnimationID, s_temp);
	s_ray = L"Ray Destination: ( ";
	s_ray += ConvertFloatToWSTRING(ray_dest.x, s_temp) + L", ";
	s_ray += ConvertFloatToWSTRING(ray_dest.y, s_temp) + L", ";
	s_ray += ConvertFloatToWSTRING(ray_dest.z, s_temp) + L" )";

	myGame.InstantText().BeginRendering();

	myGame.InstantText().RenderText(s_fps, XMFLOAT2(10, 10), XMFLOAT4(0, 0.5f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_anim_id, XMFLOAT2(10, 30), XMFLOAT4(0, 0.5f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_ray, XMFLOAT2(10, 50), XMFLOAT4(0, 0.5f, 0.7f, 1.0f));

	myGame.InstantText().EndRendering();
}