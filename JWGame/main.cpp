#include "../Core/JWLogger.h"
#include "JWGame.h"

using namespace JWEngine;

static JWGame myGame;

JW_LOGGER_DECL;

JW_FUNCTION_ON_WINDOWS_KEY_DOWN(OnWindowsKeyDown);
JW_FUNCTION_ON_WINDOWS_CHAR_INPUT(OnWindowsCharKeyInput);
JW_FUNCTION_ON_INPUT(OnInput);
JW_FUNCTION_ON_RENDER(OnRender);

int main()
{
	// TODO:
	// # Terrain	@ Use SComponentTransform (partially done... only for translation!)
	// # Physics	@ Collision
	// # Physics	@ Light/Camera representations must be pickable but not subject to physics, so these must be NonPhysical type
	// # Render		@ Sprite instancing
	// # Cursor		@ Image cursor input VS. Raw input

	JW_LOGGER_INITIALIZE;

	myGame.Create(EAllowedDisplayMode::w800h600, SPosition2(0, 30), "JWGame", "megt20all");
	//myGame.LoadCursorImage("cursor_default.png");

	// ECS Shared resources
	auto& ecs = myGame.ECS();
	{
		// SharedModel
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "jar.mobj"); // Shared Model #0
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_light.mobj"); // Shared Model #1
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::RiggedModel, "Ezreal_Idle.X", L"Ezreal_mip.dds") // Shared Model #2
			->AddAnimationFromFile("Ezreal_Punching.X")
			->AddAnimationFromFile("Ezreal_Walk.X");
		//->BakeAnimationTexture(SSizeInt(KColorCountPerTexel * KMaxBoneCount, 400), "baked_animation.dds");

		ecs.SystemRender().CreateSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeSphere(100.0f, 16, 7)); // Shared Model #3 (Sphere for Sky)
		ecs.SystemRender().CreateSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeSquare(10.0f, XMFLOAT2(10.0f, 10.0f))); // Shared Model #4

		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_camera.mobj"); // Shared Model #5

		ecs.SystemRender().CreateDynamicSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeTriangle(
				XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0))); // Shared Model #6 (Picked triangle)
		ecs.SystemRender().CreateDynamicSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeHexahedron()); // Shared Model #7 (View Frustum representation)

		ecs.SystemRender().CreateSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeSphere(0.1f, 8, 3, XMFLOAT3(0, 1, 0), XMFLOAT3(0, 1, 0))
		); // Shared Model #8 (Representation for debugging a 3d point)

		ecs.SystemRender().CreateSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeCube(1.0f)); // Shared Model #9 (Box)

		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "oil_drum.mobj"); // Shared Model #10

		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "recycling_bin.mobj"); // Shared Model #11
	}
	{
		// SharedImage2D
		ecs.SystemRender().CreateSharedImage2D(SPosition2(160, 10), SSize2(100, 40)); // Shared Image2D #0
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
		//auto terrain = ecs.SystemRender().CreateSharedTerrainFromHeightMap("heightmap_gray_128.tif", 100.0f, 4.0f);
		//ecs.SystemRender().TerrainGenerator().SaveTerrainAsTRN("heightmap_gray_128.trn", *terrain);

		//ecs.SystemRender().CreateSharedTerrainFromTRN("heightmap_gray_128.trn"); // Shared Terrain #0
	}

	ecs.SystemRender().SetSystemRenderFlag(
		JWFlagSystemRenderOption_UseLighting | JWFlagSystemRenderOption_DrawCameras | JWFlagSystemRenderOption_UseFrustumCulling);

	auto debug_point = ecs.CreateEntity(EEntityType::Point3D);
	{
		auto transform = debug_point->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 0.0f, 10.0f, 1.0f);

		auto render = debug_point->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(8));
	}

	auto view_frustum = ecs.CreateEntity(EEntityType::ViewFrustum);
	{
		auto render = view_frustum->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(7));
		render->SetRenderFlag(JWFlagComponentRenderOption_UseTransparency);
	}

	/*
	auto grid = ecs.CreateEntity(EEntityType::Grid);
	{
		auto render = grid->CreateComponentRender();
		render->SetLineModel(ecs.SystemRender().GetSharedLineModel(0));
	}
	*/

	auto picked_tri = ecs.CreateEntity(EEntityType::PickedTriangle);
	{
		auto render = picked_tri->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(6));
		render->SetRenderFlag(JWFlagComponentRenderOption_AlwaysSolidNoCull);
	}

	auto ray = ecs.CreateEntity(EEntityType::PickingRay);
	{
		auto render = ray->CreateComponentRender();
		render->SetLineModel(ecs.SystemRender().GetSharedLineModel(1));
	}

	auto sky_sphere = ecs.CreateEntity(EEntityType::Sky);
	{
		auto transform = sky_sphere->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		auto render = sky_sphere->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(3));
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(0));
		render->SetVertexShader(EVertexShader::VSSkyMap);
		render->SetPixelShader(EPixelShader::PSSkyMap);
		render->AddRenderFlag(JWFlagComponentRenderOption_NeverDrawNormals);
	}

	auto main_sprite = ecs.CreateEntity(EEntityType::MainSprite);
	{
		auto transform = main_sprite->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(0.0f, 2.8f, 0.0f, 1.0f);
		transform->ScalingFactor = { 0.02f, 0.02f, 0.02f };
		
		auto physics = main_sprite->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(3.0f, 0.0f, -0.5f, 0.0f);
		physics->SetMassToInfinite();

		auto render = main_sprite->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(2));
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(4));
		render->SetRenderFlag(
			JWFlagComponentRenderOption_UseDiffuseTexture | JWFlagComponentRenderOption_GetLit |
			JWFlagComponentRenderOption_UseAnimationInterpolation);
		render->SetAnimationTexture(ecs.SystemRender().GetAnimationTexture(0));
		render->SetAnimation(3);
	}

	/*
	auto terrain = ecs.CreateEntity(EEntityType::MainTerrain);
	{
		auto terrain_data = ecs.SystemRender().GetSharedTerrain(0);
		
		auto transform = terrain->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(-10.0f, -10.0f, 10.0f, 1.0f);

		auto physics = terrain->CreateComponentPhysics();
		physics->BoundingSphere = terrain_data->WholeBoundingSphere;
		physics->SubBoundingSpheres = terrain_data->SubBoundingSpheres;

		auto render = terrain->CreateComponentRender();
		render->SetTerrain(terrain_data);
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(1));
		render->SetTexture(ETextureType::Normal, ecs.SystemRender().GetSharedTexture(2));
		render->AddRenderFlag(JWFlagComponentRenderOption_GetLit);
	}
	*/

	auto camera_0 = ecs.CreateEntity("camera_0");
	{
		auto transform = camera_0->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 12.0f, -20.0f, 1.0f);
		transform->RotatePitchYawRoll(XMFLOAT3(XM_PIDIV2 * 1.3f, 0, 0), true);

		auto physics = camera_0->CreateComponentPhysics();
		physics->SetMassToInfinite();
		
		auto camera = camera_0->CreateComponentCamera();
		camera->CreatePerspectiveCamera(ECameraType::FreeLook);

		auto render = camera_0->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(5));
	}

	auto jar = ecs.CreateEntity("jar");
	{
		auto transform = jar->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(10.0f, 8.0f, 0.0f, 1.0f);
		transform->ScalingFactor = { 0.05f, 0.05f, 0.05f };

		auto physics = jar->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(0.8f, 0.0f, 0.6f, 0.0f);
		physics->SetMassByKilogram(1.0f);

		auto render = jar->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(0));
		render->SetRenderFlag(JWFlagComponentRenderOption_GetLit);
	}

	auto ambient_light = ecs.CreateEntity("ambient_light");
	{
		auto transform = ambient_light->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 10.0f, 0.0f, 1.0f);
		
		auto light = ambient_light->CreateComponentLight();
		light->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);

		auto physics = ambient_light->CreateComponentPhysics();
		physics->SetMassToInfinite();

		auto render = ambient_light->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(1));

		//ecs.DestroyEntityByName("ambient_light");
	}
	
	auto directional_light = ecs.CreateEntity("directional_light");
	{
		auto transform = directional_light->CreateComponentTransform();
		transform->Position = XMVectorSet(3.0f, 10.0f, -3.0f, 1.0f);

		auto light = directional_light->CreateComponentLight();
		light->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), 0.6f);

		auto physics = directional_light->CreateComponentPhysics();
		physics->SetMassToInfinite();

		auto render = directional_light->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(1));

		//ecs.DestroyEntityByName("directional_light");
	}

	auto image_gamma = ecs.CreateEntity("IMG_Gamma");
	{
		auto render = image_gamma->CreateComponentRender();
		render->SetImage2D(ecs.SystemRender().GetSharedImage2D(0));
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(3));
	}
	
	auto camera_1 = ecs.CreateEntity("camera_1");
	{
		auto transform = camera_1->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 12.0f, 0.0f, 1.0f);
		
		auto physics = camera_1->CreateComponentPhysics();
		physics->SetMassToInfinite();
		
		auto camera = camera_1->CreateComponentCamera();
		camera->CreatePerspectiveCamera(ECameraType::FreeLook);

		auto render = camera_1->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(5));
	}

	auto box = ecs.CreateEntity("box");
	{
		auto transform = box->CreateComponentTransform();
		transform->ScalingFactor = XMVectorSet(32, 1.0f, 32, 0.0f);
		transform->Position = XMVectorSet(0.0f, -2.0f, 0.0f, 1.0f);

		auto physics = box->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(24.0f);
		physics->SetMassToInfinite();

		auto render = box->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(9));
	}
	
	auto oil_drum = ecs.CreateEntity("oil_drum");
	{
		auto transform = oil_drum->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(14.0f, 8.0f, 0.0f, 1.0f);
		transform->ScalingFactor = { 0.1f, 0.1f, 0.1f };

		auto physics = oil_drum->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(1.1f);
		physics->SetMassByKilogram(20.0f);

		auto render = oil_drum->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(10));
		render->SetRenderFlag(JWFlagComponentRenderOption_GetLit);
	}

	auto recycling_bin = ecs.CreateEntity("recycling_bin");
	{
		auto transform = recycling_bin->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(18.0f, 8.0f, 0.0f, 1.0f);
		transform->ScalingFactor = { 0.06f, 0.06f, 0.06f };

		auto physics = recycling_bin->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(1.6f, 0.0f, 0.26f, 0.0f);
		physics->SetMassByKilogram(6.0f);

		auto render = recycling_bin->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModel(11));
		render->SetRenderFlag(JWFlagComponentRenderOption_GetLit);
	}

	ecs.SystemCamera().SetCurrentCamera(0);

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
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_DrawBoundingSpheres);
	}

	if (VK == VK_F5)
	{
		ecs.SystemRender().ToggleSystemRenderFlag(JWFlagSystemRenderOption_DrawSubBoundingSpheres);
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

	if (Character == '6')
	{
		ecs.SystemPhysics().ToggleSystemPhysicsFlag(JWFlagSystemPhysicsOption_ApplyForces);
	}

	if (Character == '7')
	{
		ecs.SystemPhysics().ZeroAllVelocities();
		ecs.GetEntityByName("jar")->GetComponentTransform()->Position = XMVectorSet(10.0f, 8.0f, 0.0f, 1.0f);
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
		ecs.SystemPhysics().PickEntity();

		// ECS entity Ray
		ecs.GetEntityByType(EEntityType::PickingRay)->GetComponentRender()->PtrLine
			->SetLine3DOriginDirection(0, ecs.SystemPhysics().GetPickingRayOrigin(), ecs.SystemPhysics().GetPickingRayDirection(), 1000.0f)
			->UpdateLines();

		static const auto KPickedTriangleDisplacement = XMVectorSet(0, 0.01f, 0, 0);
		ecs.GetEntityByType(EEntityType::PickedTriangle)->GetComponentRender()->PtrModel
			->SetVertex(0, ecs.SystemPhysics().GetPickedTriangleVertex(0) + KPickedTriangleDisplacement, XMFLOAT4(0, 0.8f, 0.2f, 1))
			->SetVertex(1, ecs.SystemPhysics().GetPickedTriangleVertex(1) + KPickedTriangleDisplacement, XMFLOAT4(0, 0.8f, 0.2f, 1))
			->SetVertex(2, ecs.SystemPhysics().GetPickedTriangleVertex(2) + KPickedTriangleDisplacement, XMFLOAT4(0, 0.8f, 0.2f, 1))
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
	ecs.GetEntityByType(EEntityType::Point3D)->GetComponentTransform()->Position = ecs.SystemPhysics().GetPickedPoint();

	// ECS entity Skybox
	ecs.GetEntityByType(EEntityType::Sky)->GetComponentTransform()->Position = ecs.SystemCamera().GetCurrentCameraPosition();
	
	// ECS Physics - Gravity
	ecs.SystemPhysics().ApplyUniversalGravity();

	// ECS execute systems
	ecs.ExecuteSystems();

	uint32_t anim_id{};
	// ECS entity Sprite info
	auto main_sprite = ecs.GetEntityByType(EEntityType::MainSprite);
	if (main_sprite)
	{
		const auto& anim_state = main_sprite->GetComponentRender()->AnimationState;
		anim_id = anim_state.CurrAnimationID;
	}

	// Text
	static WSTRING s_fps{};
	static WSTRING s_anim_id{};
	static WSTRING s_picked_entity{};
	static WSTRING s_cull_count{};
	static WSTRING s_cull_count2{};
	static WSTRING s_dt{};
	s_fps = L"FPS: " + TO_WSTRING(myGame.GetFPS());
	s_anim_id = L"Animation ID: " + TO_WSTRING(anim_id);
	s_picked_entity = L"Picked Entity = " + StringToWstring(ecs.SystemPhysics().GetPickedEntityName());
	s_cull_count = L"Frustum culled entities = " + TO_WSTRING(ecs.SystemRender().GetFrustumCulledEntityCount());
	s_cull_count2 = L"Frustum culled terrain nodes = " + TO_WSTRING(ecs.SystemRender().GetFrustumCulledTerrainNodeCount());
	s_dt = L"Delta time = " + TO_WSTRING(ecs.GetDeltaTime());
	
	myGame.InstantText().BeginRendering();

	myGame.InstantText().RenderText(s_fps, XMFLOAT2(10, 10), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_anim_id, XMFLOAT2(10, 30), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_picked_entity, XMFLOAT2(10, 50), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_cull_count, XMFLOAT2(10, 70), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_cull_count2, XMFLOAT2(10, 90), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_dt, XMFLOAT2(10, 110), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));

	myGame.InstantText().EndRendering();
}