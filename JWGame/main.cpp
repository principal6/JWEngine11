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
	// # JWPrimitiveMaker	@ Dynamic Quad
	// # Render		@ Frustum culling + Culling freeze
	// # Physics	@ Collision
	// # Physics	@ Light/Camera representations must be pickable but not subject to physics, so these must be NonPhysical type
	// # Render		@ Sprite instancing

	myLogger.InitializeTime();

	myGame.Create(SPositionInt(0, 30), SSizeInt(800, 600), "JWGame", "megt20all", &myLogger);
	//myGame.LoadCursorImage("cursor_default.png");
	
	// ECS Shared resources
	auto& ecs = myGame.ECS();
	{
		// SharedModel
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "Decoration_18.obj"); // Shared Model #0
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_light.obj"); // Shared Model #1
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::RiggedModel, "Ezreal_Idle.X") // Shared Model #2
			->AddAnimationFromFile("Ezreal_Punching.X")
			->AddAnimationFromFile("Ezreal_Walk.X")
			->BakeAnimationTexture(SSizeInt(KColorCountPerTexel * KMaxBoneCount, 400), "baked_animation.dds");
		ecs.SystemRender().CreateSharedModelPrimitive(
			ecs.SystemRender().PrimitiveMaker().MakeSphere(100.0f, 16, 7)); // Shared Model #3 (Sphere for Sky)
		ecs.SystemRender().CreateSharedModelPrimitive(
			ecs.SystemRender().PrimitiveMaker().MakeSquare(10.0f, XMFLOAT2(10.0f, 10.0f))); // Shared Model #4
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_camera.obj"); // Shared Model #5
		ecs.SystemRender().CreateSharedModelDynamicPrimitive(
			ecs.SystemRender().PrimitiveMaker().MakeTriangle(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(-1, -1, 0))
		); // Shared Model #6
		ecs.SystemRender().CreateSharedModelDynamicPrimitive(
			ecs.SystemRender().PrimitiveMaker().MakeQuad(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0))
		); // Shared Model #7
	}
	{
		// SharedImage2D
		ecs.SystemRender().CreateSharedImage2D(SPositionInt(160, 10), SSizeInt(100, 40)); // Shared Image2D #0
	}
	{
		// SharedLineModel
		ecs.SystemRender().CreateSharedLineModel() // Shared LineModel #0
			->Make3DGrid(50.0f, 50.0f, 1.0f);
		ecs.SystemRender().CreateSharedLineModel() // Shared LineModel #1 (Picking ray casting representation)
			->AddLine3D(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT4(1, 0, 1, 1))
			->AddEnd();
	}
	{
		// SharedTexture
		ecs.SystemRender().CreateSharedTexture(ESharedTextureType::TextureCubeMap, "skymap.dds"); // Shared Resource #0
		ecs.SystemRender().CreateSharedTexture(ESharedTextureType::Texture2D, "grass.png"); //Shared Resource #1
		ecs.SystemRender().CreateSharedTexture(ESharedTextureType::Texture2D, "Grayscale_Interval_Ten.png"); //Shared Resource #2
		ecs.SystemRender().CreateSharedTextureFromSharedModel(2); //Shared Resource #3
	}
	{
		// Animations texture
		ecs.SystemRender().CreateAnimationTextureFromFile("baked_animation.dds"); //AnimationTexture #0
	}

	auto camera_0 = ecs.CreateEntity("camera_0");
	camera_0->CreateComponentTransform()
		->SetPosition(XMFLOAT3(3, 12, 3))
		->RotatePitchYawRoll(XM_PIDIV2, 0, 0, true);
	camera_0->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(5));
	camera_0->CreateComponentCamera()
		->CreatePerspectiveCamera(ECameraType::FreeLook, myGame.GetWindowWidth(), myGame.GetWindowHeight());

	ecs.SystemCamera().SetCurrentCamera(0);

	auto grid = ecs.CreateEntity(EEntityType::Grid);
	grid->CreateComponentRender()
		->SetLineModel(ecs.SystemRender().GetSharedLineModel(0));

	auto picked_tri = ecs.CreateEntity(EEntityType::PickedTriangle);
	picked_tri->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(6))
		->SetRenderFlag(JWFlagRenderOption_AlwaysSolidNoCull);

	auto ray = ecs.CreateEntity(EEntityType::PickingRay);
	ray->CreateComponentRender()
		->SetLineModel(ecs.SystemRender().GetSharedLineModel(1));

	auto main_sprite = ecs.CreateEntity(EEntityType::MainSprite);
	main_sprite->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(-10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	main_sprite->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(2))
		->SetTexture(ecs.SystemRender().GetSharedTexture(3))
		->SetRenderFlag(JWFlagRenderOption_UseTexture | JWFlagRenderOption_UseLighting | JWFlagRenderOption_UseAnimationInterpolation)
		->SetAnimationTexture(ecs.SystemRender().GetAnimationTexture(0))
		->SetAnimation(3);

	auto sky_sphere = ecs.CreateEntity(EEntityType::Sky);
	sky_sphere->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	sky_sphere->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(3))
		->SetTexture(ecs.SystemRender().GetSharedTexture(0))
		->SetVertexShader(EVertexShader::VSSkyMap)
		->SetPixelShader(EPixelShader::PSSkyMap);

	{
		auto jars = ecs.CreateEntity("jars");
		jars->CreateComponentTransform()
			->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
			->SetPosition(XMFLOAT3(10.0f, 0.0f, 0.0f))
			->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
		jars->CreateComponentRender()
			->SetModel(ecs.SystemRender().GetSharedModel(0))
			->SetRenderFlag(JWFlagRenderOption_UseLighting);
		auto jars_physics = jars->CreateComponentPhysics();
		ecs.SystemPhysics().CreateBoundingSphere(jars_physics, 1.1f, XMFLOAT3(0, 0.7f, 0.3f));
	}

	auto ambient_light = ecs.CreateEntity("ambient_light");
	ambient_light->CreateComponentLight()
		->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);
	ambient_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0.0f, 5.0f, 0.0f));
	ambient_light->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(1));

	auto directional_light = ecs.CreateEntity("directional_light");
	directional_light->CreateComponentLight()
		->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), 0.6f);
	directional_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(3.0f, 3.0f, 3.0f));
	directional_light->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(1));

	{
		auto floor_plane = ecs.CreateEntity("Floor");
		floor_plane->CreateComponentTransform()
			->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
			->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
		floor_plane->CreateComponentRender()
			->SetModel(ecs.SystemRender().GetSharedModel(4))
			->SetTexture(ecs.SystemRender().GetSharedTexture(1));
		auto floor_plane_physics = floor_plane->CreateComponentPhysics();
		ecs.SystemPhysics().CreateBoundingSphere(floor_plane_physics, 2.0f);
	}

	auto image_gamma = ecs.CreateEntity("IMG_Gamma");
	image_gamma->CreateComponentRender()
		->SetImage2D(ecs.SystemRender().GetSharedImage2D(0))
		->SetTexture(ecs.SystemRender().GetSharedTexture(2));

	auto cam = ecs.CreateEntity("Camera");
	cam->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0, 2, 0));
	cam->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(5));
	cam->CreateComponentCamera()
		->CreatePerspectiveCamera(ECameraType::FreeLook, myGame.GetWindowWidth(), myGame.GetWindowHeight());

	myGame.SetFunctionOnWindowsKeyDown(OnWindowsKeyDown);
	myGame.SetFunctionOnWindowsCharInput(OnWindowsCharKeyInput);
	myGame.SetFunctionOnInput(OnInput);
	myGame.SetFunctionOnRender(OnRender);

	myGame.Run();
	
	return 0;
}

JW_FUNCTION_ON_WINDOWS_KEY_DOWN(OnWindowsKeyDown)
{
	auto& ecs = myGame.ECS();

	if (VK == VK_F1)
	{
		ecs.SystemRender().ToggleWireFrame();
	}

	if (VK == VK_F2)
	{
		ecs.SystemRender().ToggleNormalDrawing();
	}

	if (VK == VK_F3)
	{
		ecs.SystemRender().ToggleLighting();
	}

	if (VK == VK_F4)
	{
		ecs.SystemRender().ToggleBoundingVolumeDrawing();
	}

	if (VK == VK_F5)
	{
		ecs.SystemRender().ToggleCameraDrawing();
	}
}

JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(OnWindowsCharKeyInput)
{
	auto& ecs = myGame.ECS();

	if (Character == '1')
	{
		ecs.GetEntityByType(EEntityType::MainSprite)->GetComponentRender()->PrevAnimation();
	}

	if (Character == '2')
	{
		ecs.GetEntityByType(EEntityType::MainSprite)->GetComponentRender()->NextAnimation();
	}

	if (Character == '3')
	{
		ecs.SystemCamera().SetCurrentCamera(0);
	}

	if (Character == '4')
	{
		ecs.SystemCamera().SetCurrentCamera(1);
	}
}

JW_FUNCTION_ON_INPUT(OnInput)
{
	auto& ecs = myGame.ECS();

	if (DeviceState.Keys[DIK_ESCAPE])
	{
		myGame.Halt();
	}

	if (DeviceState.Keys[DIK_W])
	{
		ecs.SystemCamera().MoveCurrentCamera(ECameraDirection::Forward);
	}

	if (DeviceState.Keys[DIK_S])
	{
		ecs.SystemCamera().MoveCurrentCamera(ECameraDirection::Backward);
	}

	if (DeviceState.Keys[DIK_A])
	{
		ecs.SystemCamera().MoveCurrentCamera(ECameraDirection::Left);
	}

	if (DeviceState.Keys[DIK_D])
	{
		ecs.SystemCamera().MoveCurrentCamera(ECameraDirection::Right);
	}

	// Mouse left button pressed
	if (DeviceState.CurrentMouse.rgbButtons[0])
	{
		myGame.ECS().SystemPhysics().Pick();
		
		// ECS entity Ray
		myGame.ECS().GetEntityByType(EEntityType::PickingRay)->GetComponentRender()->PtrLine
			->SetLine3DOriginDirection(0, myGame.ECS().SystemPhysics().GetPickingRayOrigin(), myGame.ECS().SystemPhysics().GetPickingRayDirection())
			->UpdateLines();

		XMFLOAT3 tri_a{}, tri_b{}, tri_c{};
		XMStoreFloat3(&tri_a, myGame.ECS().SystemPhysics().GetPickedTrianglePosition(0));
		XMStoreFloat3(&tri_b, myGame.ECS().SystemPhysics().GetPickedTrianglePosition(1));
		XMStoreFloat3(&tri_c, myGame.ECS().SystemPhysics().GetPickedTrianglePosition(2));
		
		myGame.ECS().GetEntityByType(EEntityType::PickedTriangle)->GetComponentRender()->PtrModel
			->SetVertex(0, tri_a, XMFLOAT4(1, 1, 1, 1))
			->SetVertex(1, tri_b, XMFLOAT4(1, 0, 0, 1))
			->SetVertex(2, tri_c, XMFLOAT4(0, 1, 0, 1))
			->UpdateModel();
	}

	// Mouse cursor moved
	if ((DeviceState.CurrentMouse.lX != DeviceState.PreviousMouse.lX) || (DeviceState.CurrentMouse.lY != DeviceState.PreviousMouse.lY))
	{
		// Mouse right button pressed
		if (DeviceState.CurrentMouse.rgbButtons[1])
		{
			ecs.SystemCamera().RotateCurrentCamera(static_cast<float>(DeviceState.CurrentMouse.lY), static_cast<float>(DeviceState.CurrentMouse.lX), 0);
		}
	}
	
	// Mouse wheel scrolled
	if ((DeviceState.CurrentMouse.lZ))
	{
		ecs.SystemCamera().ZoomCurrentCamera(static_cast<float>(DeviceState.CurrentMouse.lZ));
	}
}

JW_FUNCTION_ON_RENDER(OnRender)
{
	// ECS entity Skybox
	myGame.ECS().GetEntityByType(EEntityType::Sky)->GetComponentTransform()->SetPosition(myGame.ECS().SystemCamera().GetCurrentCameraPosition());

	// ECS execute systems
	myGame.ECS().ExecuteSystems();

	// ECS entity Sprite info
	const auto& anim_state = myGame.ECS().GetEntityByType(EEntityType::MainSprite)->GetComponentRender()->AnimationState;
	
	// Text
	static WSTRING s_temp{};
	static WSTRING s_fps{};
	static WSTRING s_anim_id{};
	static WSTRING s_picked_entity{};

	s_fps = L"FPS: " + ConvertIntToWSTRING(myGame.GetFPS(), s_temp);
	s_anim_id = L"Animation ID: " + ConvertIntToWSTRING(anim_state.CurrAnimationID, s_temp);
	s_picked_entity = L"Picked Entity = " + StringToWstring(myGame.ECS().SystemPhysics().GetPickedEntityName());

	myGame.InstantText().BeginRendering();

	myGame.InstantText().RenderText(s_fps, XMFLOAT2(10, 10), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_anim_id, XMFLOAT2(10, 30), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_picked_entity, XMFLOAT2(10, 50), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));

	myGame.InstantText().EndRendering();
}