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
	// # Terrain	@ Terrain picking
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
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::RiggedModel, "Ezreal_Idle.X", L"Ezreal_mip.dds") // Shared Model #2
			->AddAnimationFromFile("Ezreal_Punching.X")
			->AddAnimationFromFile("Ezreal_Walk.X")
			->BakeAnimationTexture(SSizeInt(KColorCountPerTexel * KMaxBoneCount, 400), "baked_animation.dds");

		ecs.SystemRender().CreateSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeSphere(100.0f, 16, 7)); // Shared Model #3 (Sphere for Sky)
		ecs.SystemRender().CreateSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeSquare(10.0f, XMFLOAT2(10.0f, 10.0f))); // Shared Model #4

		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_camera.obj"); // Shared Model #5

		ecs.SystemRender().CreateDynamicSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeTriangle(
				XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(-1, -1, 0))); // Shared Model #6 (Picked triangle) == Legacy
		ecs.SystemRender().CreateDynamicSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeHexahedron()); // Shared Model #7 (View Frustum representation)

		ecs.SystemRender().CreateSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeSphere(2.0f, 16, 7)); // Shared Model #8 (Representation for debugging a 3d point)
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
		ecs.SystemRender().CreateSharedTexture(ESharedTextureType::Texture2D, "terra_diffuse_mip.dds"); //Shared Resource #1
		ecs.SystemRender().CreateSharedTexture(ESharedTextureType::Texture2D, "terra_normal_mip.dds"); //Shared Resource #2
		ecs.SystemRender().CreateSharedTexture(ESharedTextureType::Texture2D, "Grayscale_Interval_Ten.png"); //Shared Resource #3
		ecs.SystemRender().CreateSharedTextureFromSharedModel(2); //Shared Resource #4
	}
	{
		// Animations texture
		ecs.SystemRender().CreateAnimationTextureFromFile("baked_animation.dds"); //AnimationTexture #0
	}
	{
		// Terrain
		//auto terrain = ecs.SystemRender().CreateSharedTerrainFromHeightMap("heightmap_rgb_test_20x15.tif", 20.0f);
		//ecs.SystemRender().TerrainGenerator().SaveTerrainAsTRN("heightmap_rgb_test_20x15.trn", *terrain);

		ecs.SystemRender().CreateSharedTerrainFromTRN("heightmap_rgb_test_20x15.trn"); // Shared Terrain #0
	}

	ecs.SystemRender().SetSystemRenderFlag(
		JWFlagSystemRenderOption_UseLighting | JWFlagSystemRenderOption_DrawCameras | JWFlagSystemRenderOption_UseFrustumCulling);

	auto debug_point = ecs.CreateEntity(EEntityType::Point3D);
	debug_point->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0, 0, 10.0f));
	debug_point->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(8));

	auto camera_0 = ecs.CreateEntity("camera_0");
	camera_0->CreateComponentTransform()
		->SetPosition(XMFLOAT3(3, 12, 3))
		->RotatePitchYawRoll(XM_PIDIV2, 0, 0, true);
	camera_0->CreateComponentPhysics();
	camera_0->CreateComponentCamera()
		->CreatePerspectiveCamera(ECameraType::FreeLook, myGame.GetWindowWidth(), myGame.GetWindowHeight());
	camera_0->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(5));

	ecs.SystemCamera().SetCurrentCamera(0);

	auto view_frustum = ecs.CreateEntity(EEntityType::ViewFrustum);
	view_frustum->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(7))
		->SetRenderFlag(JWFlagComponentRenderOption_UseTransparency);

	auto grid = ecs.CreateEntity(EEntityType::Grid);
	grid->CreateComponentRender()
		->SetLineModel(ecs.SystemRender().GetSharedLineModel(0));

	auto picked_tri = ecs.CreateEntity(EEntityType::PickedTriangle);
	picked_tri->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(6))
		->SetRenderFlag(JWFlagComponentRenderOption_AlwaysSolidNoCull);

	auto ray = ecs.CreateEntity(EEntityType::PickingRay);
	ray->CreateComponentRender()
		->SetLineModel(ecs.SystemRender().GetSharedLineModel(1));

	{
		auto main_sprite = ecs.CreateEntity(EEntityType::MainSprite);
		main_sprite->CreateComponentTransform()
			->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
			->SetPosition(XMFLOAT3(-10.0f, 0.0f, 0.0f))
			->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
		main_sprite->CreateComponentPhysics();
		main_sprite->CreateComponentRender()
			->SetModel(ecs.SystemRender().GetSharedModel(2))
			->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(4))
			->SetRenderFlag(JWFlagComponentRenderOption_UseDiffuseTexture | JWFlagComponentRenderOption_GetLit | JWFlagComponentRenderOption_UseAnimationInterpolation)
			->SetAnimationTexture(ecs.SystemRender().GetAnimationTexture(0))
			->SetAnimation(3);
		ecs.SystemPhysics().SetBoundingSphere(main_sprite, 3.0f, XMFLOAT3(0, 0, 0));
	}

	auto sky_sphere = ecs.CreateEntity(EEntityType::Sky);
	sky_sphere->CreateComponentTransform()
		->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
		->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	sky_sphere->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(3))
		->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(0))
		->SetVertexShader(EVertexShader::VSSkyMap)
		->SetPixelShader(EPixelShader::PSSkyMap);

	{
		auto jars = ecs.CreateEntity("jars");
		jars->CreateComponentTransform()
			->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
			->SetPosition(XMFLOAT3(10.0f, 0.0f, 0.0f))
			->SetScalingFactor(XMFLOAT3(0.05f, 0.05f, 0.05f));
		jars->CreateComponentPhysics();
		jars->CreateComponentRender()
			->SetModel(ecs.SystemRender().GetSharedModel(0))
			->SetRenderFlag(JWFlagComponentRenderOption_GetLit);
		ecs.SystemPhysics().SetBoundingSphere(jars, 1.1f, XMFLOAT3(0, 0.7f, 0.3f));
	}

	auto ambient_light = ecs.CreateEntity("ambient_light");
	ambient_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0.0f, 5.0f, 0.0f));
	ambient_light->CreateComponentLight()
		->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);
	ambient_light->CreateComponentPhysics();
	ambient_light->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(1));
	//ecs.DestroyEntityByName("ambient_light");

	auto directional_light = ecs.CreateEntity("directional_light");
	directional_light->CreateComponentTransform()
		->SetPosition(XMFLOAT3(3.0f, 3.0f, 3.0f));
	directional_light->CreateComponentLight()
		->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), 0.6f);
	directional_light->CreateComponentPhysics();
	directional_light->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(1));
	//ecs.DestroyEntityByName("directional_light");

	{
		auto terrain_data = ecs.SystemRender().GetSharedTerrain(0);
		auto terrain = ecs.CreateEntity("terrain");
		terrain->CreateComponentTransform()
			->SetWorldMatrixCalculationOrder(EWorldMatrixCalculationOrder::ScaleRotTrans)
			->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
		terrain->CreateComponentPhysics();
		terrain->CreateComponentRender()
			->SetTerrain(terrain_data)
			->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(1))
			->SetTexture(ETextureType::Normal, ecs.SystemRender().GetSharedTexture(2))
			->AddRenderFlag(JWFlagComponentRenderOption_GetLit);
		ecs.SystemPhysics().SetBoundingSphere(terrain, terrain_data->WholeBoundingSphereRadius, terrain_data->WholeBoundingSphereOffset);
		ecs.SystemPhysics().SetSubBoundingSpheres(terrain, terrain_data->SubBoundingSpheres);
	}

	auto image_gamma = ecs.CreateEntity("IMG_Gamma");
	image_gamma->CreateComponentRender()
		->SetImage2D(ecs.SystemRender().GetSharedImage2D(0))
		->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(3));

	auto cam = ecs.CreateEntity("camera_1");
	cam->CreateComponentTransform()
		->SetPosition(XMFLOAT3(0, 2, 0));
	cam->CreateComponentPhysics();
	cam->CreateComponentCamera()
		->CreatePerspectiveCamera(ECameraType::FreeLook, myGame.GetWindowWidth(), myGame.GetWindowHeight());
	cam->CreateComponentRender()
		->SetModel(ecs.SystemRender().GetSharedModel(5));

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
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_DrawNormals);
	}

	if (VK == VK_F3)
	{
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_UseLighting);
	}

	if (VK == VK_F4)
	{
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_DrawBoundingVolumes);
	}

	if (VK == VK_F5)
	{
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_DrawSubBoundingVolumes);
	}

	if (VK == VK_F6)
	{
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_DrawCameras);
	}

	if (VK == VK_F7)
	{
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_DrawViewFrustum);
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

	if (Character == '5')
	{
		// Frustum culling
		const auto& frustum = ecs.SystemCamera().GetCapturedViewFrustum();

		ecs.GetEntityByType(EEntityType::ViewFrustum)->GetComponentRender()->PtrModel
			// Left plane
			->SetVertex(0, frustum.NLU, XMFLOAT4(1, 0, 0, 0.5f))
			->SetVertex(1, frustum.FLU, XMFLOAT4(1, 0, 0, 0.5f))
			->SetVertex(2, frustum.FLD, XMFLOAT4(1, 0, 0, 0.5f))
			->SetVertex(3, frustum.NLD, XMFLOAT4(1, 0, 0, 0.5f))
			// Right plane
			->SetVertex(4, frustum.NRU, XMFLOAT4(1, 0, 0, 0.5f))
			->SetVertex(5, frustum.FRU, XMFLOAT4(1, 0, 0, 0.5f))
			->SetVertex(6, frustum.FRD, XMFLOAT4(1, 0, 0, 0.5f))
			->SetVertex(7, frustum.NRD, XMFLOAT4(1, 0, 0, 0.5f))
			->UpdateModel();
	}

	if (Character == 't')
	{
		ecs.GetEntityByName("ambient_light")->GetComponentTransform()->Translate(XMFLOAT3(0.1f, 0, 0));
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
		ecs.SystemPhysics().Pick();
		
		// ECS entity Ray
		ecs.GetEntityByType(EEntityType::PickingRay)->GetComponentRender()->PtrLine
			->SetLine3DOriginDirection(0, ecs.SystemPhysics().GetPickingRayOrigin(), ecs.SystemPhysics().GetPickingRayDirection())
			->UpdateLines();

		XMFLOAT3 tri_a{}, tri_b{}, tri_c{};
		XMStoreFloat3(&tri_a, ecs.SystemPhysics().GetPickedTrianglePosition(0));
		XMStoreFloat3(&tri_b, ecs.SystemPhysics().GetPickedTrianglePosition(1));
		XMStoreFloat3(&tri_c, ecs.SystemPhysics().GetPickedTrianglePosition(2));
		
		ecs.GetEntityByType(EEntityType::PickedTriangle)->GetComponentRender()->PtrModel
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
	auto& ecs = myGame.ECS();

	// 3D Point for debugging
	//ecs.GetEntityByType(EEntityType::Point3D)->GetComponentTransform()->SetPosition(...);

	// ECS entity Skybox
	ecs.GetEntityByType(EEntityType::Sky)->GetComponentTransform()->SetPosition(ecs.SystemCamera().GetCurrentCameraPosition());

	// ECS execute systems
	ecs.ExecuteSystems();

	// ECS entity Sprite info
	const auto& anim_state = ecs.GetEntityByType(EEntityType::MainSprite)->GetComponentRender()->AnimationState;
	
	// Text
	static WSTRING s_temp{};
	static WSTRING s_fps{};
	static WSTRING s_anim_id{};
	static WSTRING s_picked_entity{};
	static WSTRING s_cull_count{};
	static WSTRING s_cull_count2{};

	s_fps = L"FPS: " + ConvertIntToWSTRING(myGame.GetFPS(), s_temp);
	s_anim_id = L"Animation ID: " + ConvertIntToWSTRING(anim_state.CurrAnimationID, s_temp);
	s_picked_entity = L"Picked Entity = " + StringToWstring(ecs.SystemPhysics().GetPickedEntityName());
	s_cull_count = L"Frustum culled entities = " + ConvertIntToWSTRING(ecs.SystemRender().GetFrustumCulledEntityCount(), s_temp);
	s_cull_count2 = L"Frustum culled terrain nodes = " + ConvertIntToWSTRING(ecs.SystemRender().GetFrustumCulledTerrainNodeCount(), s_temp);

	myGame.InstantText().BeginRendering();

	myGame.InstantText().RenderText(s_fps, XMFLOAT2(10, 10), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_anim_id, XMFLOAT2(10, 30), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_picked_entity, XMFLOAT2(10, 50), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_cull_count, XMFLOAT2(10, 70), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_cull_count2, XMFLOAT2(10, 90), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));

	myGame.InstantText().EndRendering();
}