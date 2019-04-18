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
	// # Render		@ Draw (instanced) bounding spheres
	// # Physics	@ Collision
	// # Render		@ Sprite nstancing

	myLogger.InitializeTime();

	myGame.Create(SPositionInt(0, 30), SSizeInt(800, 600), "JWGame", "megt20all", &myLogger);
	//myGame.LoadCursorImage("cursor_default.png");

	myGame.Camera()
		.SetCameraType(ECameraType::FreeLook)
		.SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));

	// ECS Shared resources
	auto& ecs = myGame.ECS();
	{
		// SharedModel
		ecs.CreateSharedModelFromFile(ESharedModelType::StaticModel, "Decoration_18.obj"); // Shared Model #0
		ecs.CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_light.obj"); // Shared Model #1
		ecs.CreateSharedModelFromFile(ESharedModelType::RiggedModel, "Ezreal_Idle.X") // Shared Model #2
			->AddAnimationFromFile("Ezreal_Punching.X")
			->AddAnimationFromFile("Ezreal_Walk.X")
			->BakeAnimationTexture(SSizeInt(KColorCountPerTexel * KMaxBoneCount, 400), "baked_animation.dds");
		ecs.CreateSharedModelPrimitive(
			ecs.PrimitiveMaker().MakeSphere(100.0f, 16, 7)); // Shared Model #3 (Sphere for Sky)
		ecs.CreateSharedModelPrimitive(
			ecs.PrimitiveMaker().MakeSquare(10.0f, XMFLOAT2(10.0f, 10.0f))); // Shared Model #4
		ecs.CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_camera.obj"); // Shared Model #5
		ecs.CreateSharedModelDynamicPrimitive(
			ecs.PrimitiveMaker().MakeTriangle(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(-1, -1, 0))); // Shared Model #6
	}
	{
		// SharedImage2D
		ecs.CreateSharedImage2D(SPositionInt(160, 10), SSizeInt(100, 40)); // Shared Image2D #0
	}
	{
		// SharedLineModel
		ecs.CreateSharedLineModel() // Shared LineModel #0
			->Make3DGrid(50.0f, 50.0f, 1.0f);
		ecs.CreateSharedLineModel() // Shared LineModel #1 (Picking ray casting representation)
			->AddLine3D(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT4(1, 0, 1, 1))
			->AddEnd();
	}
	{
		// SharedTexture
		ecs.CreateSharedTexture(ESharedTextureType::TextureCubeMap, "skymap.dds"); // Shared Resource #0
		ecs.CreateSharedTexture(ESharedTextureType::Texture2D, "grass.png"); //Shared Resource #1
		ecs.CreateSharedTexture(ESharedTextureType::Texture2D, "Grayscale_Interval_Ten.png"); //Shared Resource #2
		ecs.CreateSharedTextureFromSharedModel(2); //Shared Resource #3
	}
	{
		// Animations texture
		ecs.CreateAnimationTextureFromFile("baked_animation.dds"); //AnimationTexture #0
	}

	auto grid = ecs.CreateEntity(EEntityType::Grid);
	grid->CreateComponentRender()
		->SetLineModel(ecs.GetSharedLineModel(0));

	auto main_sprite = ecs.CreateEntity(EEntityType::MainSprite);
	main_sprite->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(-10.0f, 0.0f, 0.0f))
		->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
	main_sprite->CreateComponentRender()
		->SetModel(ecs.GetSharedModel(2))
		->SetTexture(ecs.GetSharedTexture(3))
		->SetRenderFlag(JWFlagRenderOption_UseTexture | JWFlagRenderOption_UseLighting | JWFlagRenderOption_UseAnimationInterpolation)
		->SetAnimationTexture(ecs.GetAnimationTexture(0))
		->SetAnimation(3);

	auto sky_sphere = ecs.CreateEntity(EEntityType::Sky);
	sky_sphere->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	sky_sphere->CreateComponentRender()
		->SetModel(ecs.GetSharedModel(3))
		->SetTexture(ecs.GetSharedTexture(0))
		->SetVertexShader(EVertexShader::VSSkyMap)
		->SetPixelShader(EPixelShader::PSSkyMap);

	{
		auto jars = ecs.CreateEntity("jars");
		jars->CreateComponentTransform()
			->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
			->SetPosition(XMFLOAT3(10.0f, 0.0f, 0.0f))
			->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
		jars->CreateComponentRender()
			->SetModel(ecs.GetSharedModel(0))
			->SetRenderFlag(JWFlagRenderOption_UseLighting);
		auto jars_physics = jars->CreateComponentPhysics();
		ecs.SystemPhysics().SetBoundingSphereRadius(jars_physics, 1.0f);
	}
	

	auto ambient_light = ecs.CreateEntity("ambient_light");
	ambient_light->CreateComponentLight()
		->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);
	ambient_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0.0f, -5.0f, 0.0f));
	ambient_light->CreateComponentRender()
		->SetModel(ecs.GetSharedModel(1));

	auto directional_light = ecs.CreateEntity("directional_light");
	directional_light->CreateComponentLight()
		->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(5.0f, 5.0f, 0.0f), 0.6f);
	directional_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(8.0f, 8.0f, 0.0f));
	directional_light->CreateComponentRender()
		->SetModel(ecs.GetSharedModel(1));

	{
		auto floor_plane = ecs.CreateEntity("Floor");
		floor_plane->CreateComponentTransform()
			->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
			->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
		floor_plane->CreateComponentRender()
			->SetModel(ecs.GetSharedModel(4))
			->SetTexture(ecs.GetSharedTexture(1));
		auto floor_plane_physics = floor_plane->CreateComponentPhysics();
		ecs.SystemPhysics().SetBoundingSphereRadius(floor_plane_physics, 1.0f);
	}

	auto image_gamma = ecs.CreateEntity("IMG_Gamma");
	image_gamma->CreateComponentRender()
		->SetImage2D(ecs.GetSharedImage2D(0))
		->SetTexture(ecs.GetSharedTexture(2));

	auto cam = ecs.CreateEntity("Camera");
	cam->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0, 2, 0));
	cam->CreateComponentRender()
		->SetModel(ecs.GetSharedModel(5));

	auto picked_tri = ecs.CreateEntity(EEntityType::PickedTriangle);
	picked_tri->CreateComponentRender()
		->SetModel(ecs.GetSharedModel(6))
		->SetRenderFlag(JWFlagRenderOption_AlwaysSolidNoCull);

	auto ray = ecs.CreateEntity(EEntityType::PickingRay);
	ray->CreateComponentRender()
		->SetLineModel(ecs.GetSharedLineModel(1));

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
		myGame.ECS().SystemRender().ToggleWireFrame();
	}

	if (VK == VK_F2)
	{
		myGame.ECS().SystemRender().ToggleNormalDrawing();
	}

	if (VK == VK_F3)
	{
		myGame.ECS().SystemRender().ToggleLighting();
	}
}

JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(OnWindowsCharKeyInput)
{
	if (Character == '1')
	{
		myGame.ECS().GetEntityByType(EEntityType::MainSprite)->GetComponentRender()->PrevAnimation();
	}

	if (Character == '2')
	{
		myGame.ECS().GetEntityByType(EEntityType::MainSprite)->GetComponentRender()->NextAnimation();
	}
}

JW_FUNCTION_ON_INPUT(OnInput)
{
	if (DeviceState.Keys[DIK_ESCAPE])
	{
		myGame.Halt();
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
			myGame.Camera().RotateCamera(static_cast<float>(DeviceState.CurrentMouse.lY), static_cast<float>(DeviceState.CurrentMouse.lX), 0);
		}
	}
	
	// Mouse wheel scrolled
	if ((DeviceState.CurrentMouse.lZ))
	{
		myGame.Camera().ZoomCamera(static_cast<float>(DeviceState.CurrentMouse.lZ));
	}
}

JW_FUNCTION_ON_RENDER(OnRender)
{
	// ECS entity Skybox
	myGame.ECS().GetEntityByType(EEntityType::Sky)->GetComponentTransform()->SetPosition(myGame.Camera().GetPosition());

	// ECS execute systems
	myGame.ECS().ExecuteSystems();

	// ECS entity Sprite info
	const auto& anim_state = myGame.ECS().GetEntityByType(EEntityType::MainSprite)->GetComponentRender()->AnimationState;
	
	// Text
	static WSTRING s_temp{};
	static WSTRING s_fps{};
	static WSTRING s_anim_id{};
	static WSTRING s_ray{};
	static WSTRING s_picked_entity{};

	s_fps = L"FPS: " + ConvertIntToWSTRING(myGame.GetFPS(), s_temp);
	s_anim_id = L"Animation ID: " + ConvertIntToWSTRING(anim_state.CurrAnimationID, s_temp);
	
	auto ray_dir = myGame.ECS().SystemPhysics().GetPickingRayDirection();
	s_ray = L"Ray Direction: ( ";
	s_ray += ConvertFloatToWSTRING(XMVectorGetX(ray_dir), s_temp) + L", ";
	s_ray += ConvertFloatToWSTRING(XMVectorGetY(ray_dir), s_temp) + L", ";
	s_ray += ConvertFloatToWSTRING(XMVectorGetZ(ray_dir), s_temp) + L" )";

	s_picked_entity = L"Picked Entity = " + StringToWstring(myGame.ECS().SystemPhysics().GetPickedEntityName());

	myGame.InstantText().BeginRendering();

	myGame.InstantText().RenderText(s_fps, XMFLOAT2(10, 10), XMFLOAT4(0, 0.5f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_anim_id, XMFLOAT2(10, 30), XMFLOAT4(0, 0.5f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_ray, XMFLOAT2(10, 50), XMFLOAT4(0, 0.5f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_picked_entity, XMFLOAT2(10, 70), XMFLOAT4(0, 0.5f, 0.7f, 1.0f));

	myGame.InstantText().EndRendering();
}