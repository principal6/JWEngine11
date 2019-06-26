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
	// # Render		@ Sprite instancing
	// # Cursor		@ Image cursor input VS. Raw input

	JW_LOGGER_INITIALIZE;

	myGame.Create(EAllowedDisplayMode::w800h600, SPosition2(0, 30), "JWGame", "megt20all");
	//myGame.LoadCursorImage("cursor_default.png");

	// ECS Shared resources
	auto& ecs = myGame.ECS();
	{
		// SharedModel
		ecs.SystemRender().CreateDynamicSharedModelFromModelData(
			ecs.SystemRender().PrimitiveMaker().MakeTriangle(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0)), "PICKED_TRIANGLE");
		ecs.SystemRender().CreateSharedModelFromModelData(ESharedModelType::StaticModel,
			ecs.SystemRender().PrimitiveMaker().MakeSphere(0.1f, 8, 3, XMFLOAT3(0, 1, 0), XMFLOAT3(0, 1, 0)), "POINT_3D");
		ecs.SystemRender().CreateSharedModelFromModelData(ESharedModelType::StaticModel,
			ecs.SystemRender().PrimitiveMaker().MakeSphere(0.1f, 8, 3, XMFLOAT3(1, 0, 0), XMFLOAT3(1, 1, 0)), "POINT_3D_A");
		ecs.SystemRender().CreateSharedModelFromModelData(ESharedModelType::StaticModel,
			ecs.SystemRender().PrimitiveMaker().MakeSphere(0.1f, 8, 3, XMFLOAT3(0, 0, 1), XMFLOAT3(1, 0, 1)), "POINT_3D_B");
		ecs.SystemRender().CreateDynamicSharedModelFromModelData(ecs.SystemRender().PrimitiveMaker().MakeHexahedron(), "VIEW_FRUSTUM");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_camera.mobj", "CAMERA");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "simple_light.mobj", "LIGHT");
		ecs.SystemRender().CreateSharedModelFromModelData(ESharedModelType::StaticModel, 
			ecs.SystemRender().PrimitiveMaker().MakeSphere(100.0f, 16, 7), "SKY_SPHERE");

		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::RiggedModel, "Ezreal_Idle.X", "EZREAL", L"Ezreal_mip.dds")
			->AddAnimationFromFile("Ezreal_Punching.X")
			->AddAnimationFromFile("Ezreal_Walk.X");
		//->BakeAnimationTexture(SSizeInt(KColorCountPerTexel * KMaxBoneCount, 400), "baked_animation.dds");

		ecs.SystemRender().CreateSharedModelFromModelData(ESharedModelType::StaticModel, 
			ecs.SystemRender().PrimitiveMaker().MakeCube(1.0f), "box");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "jar.mobj", "jar");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "oil_drum.mobj", "oil_drum");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::StaticModel, "recycling_bin.mobj", "recycling_bin");

		ecs.SystemRender().CreateSharedModelFromModelData(ESharedModelType::CollisionMesh,
			ecs.SystemRender().PrimitiveMaker().MakeCube(1.0f), "CM_box");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::CollisionMesh, "jar_cm.mobj", "CM_jar");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::CollisionMesh, "oil_drum_cm.mobj", "CM_oil_drum");
		ecs.SystemRender().CreateSharedModelFromFile(ESharedModelType::CollisionMesh, "recycling_bin_cm.mobj", "CM_recycling_bin");
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
		ecs.SystemRender().CreateSharedTextureFromSharedModel("EZREAL"); //Shared Resource #4
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

	// SystemRender setting
	ecs.SystemRender().SetSystemRenderFlag(
		JWFlagSystemRenderOption_UseLighting | JWFlagSystemRenderOption_DrawCameras | JWFlagSystemRenderOption_UseFrustumCulling);

	// SystemPhysics setting
	//ecs.SystemPhysics().AddMaterialFrictionData("my_material", 1.0f, 0.8f);
	//ecs.SystemPhysics().GetMaterialFrictionDataByMaterialName("my_material");

	{
		auto debug_point = ecs.CreateEntity(EEntityType::Point3D);

		auto transform = debug_point->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 0.0f, 10.0f, 1.0f);

		auto render = debug_point->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("POINT_3D"));
	}

	{
		auto VIEW_FRUSTUM = ecs.CreateEntity(EEntityType::ViewFrustum);

		auto render = VIEW_FRUSTUM->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("VIEW_FRUSTUM"));
		render->SetRenderFlag(JWFlagComponentRenderOption_UseTransparency);
	}

	{
		auto grid = ecs.CreateEntity(EEntityType::Grid);

		auto render = grid->CreateComponentRender();
		render->SetLineModel(ecs.SystemRender().GetSharedLineModel(0));
	}
	
	{
		auto image_gamma = ecs.CreateEntity("IMG_Gamma");

		auto render = image_gamma->CreateComponentRender();
		render->SetImage2D(ecs.SystemRender().GetSharedImage2D(0));
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(3));
	}

	{
		auto picked_tri = ecs.CreateEntity(EEntityType::PickedTriangle);

		auto render = picked_tri->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("PICKED_TRIANGLE"));
		render->SetRenderFlag(JWFlagComponentRenderOption_AlwaysSolidNoCull);
	}

	{
		auto ray = ecs.CreateEntity(EEntityType::PickingRay);

		auto render = ray->CreateComponentRender();
		render->SetLineModel(ecs.SystemRender().GetSharedLineModel(1));
	}

	{
		auto SKY_SPHERE = ecs.CreateEntity(EEntityType::Sky);

		auto transform = SKY_SPHERE->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		auto render = SKY_SPHERE->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("SKY_SPHERE"));
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(0));
		render->SetVertexShader(EVertexShader::VSSkyMap);
		render->SetPixelShader(EPixelShader::PSSkyMap);
		render->AddRenderFlag(JWFlagComponentRenderOption_NeverDrawNormals);
	}

	{
		auto main_sprite = ecs.CreateEntity(EEntityType::MainSprite);

		auto transform = main_sprite->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(0.0f, 2.8f, 0.0f, 1.0f);
		transform->ScalingFactor = { 0.02f, 0.02f, 0.02f };
		
		auto physics = main_sprite->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(3.0f, 0.0f, -0.5f, 0.0f);
		//physics->SetMassByKilogram();

		auto render = main_sprite->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("EZREAL"));
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(4));
		render->SetRenderFlag(
			JWFlagComponentRenderOption_UseDiffuseTexture | JWFlagComponentRenderOption_GetLit |
			JWFlagComponentRenderOption_UseAnimationInterpolation);
		render->SetAnimationTexture(ecs.SystemRender().GetAnimationTexture(0));
		render->SetAnimation(3);
	}

	/*
	{
		auto terrain = ecs.CreateEntity(EEntityType::MainTerrain);

		auto terrain_data = ecs.SystemRender().GetSharedTerrain(0);
		
		auto transform = terrain->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(-10.0f, -10.0f, 10.0f, 1.0f);

		auto physics = terrain->CreateComponentPhysics();
		physics->SetMassToInfinite();
		physics->BoundingSphere = terrain_data->WholeBoundingSphere;
		physics->SubBoundingSpheres = terrain_data->SubBoundingSpheres;

		auto render = terrain->CreateComponentRender();
		render->SetTerrain(terrain_data);
		render->SetTexture(ETextureType::Diffuse, ecs.SystemRender().GetSharedTexture(1));
		render->SetTexture(ETextureType::Normal, ecs.SystemRender().GetSharedTexture(2));
		render->AddRenderFlag(JWFlagComponentRenderOption_GetLit);
	}
	*/

	{
		auto camera_0 = ecs.CreateEntity("camera_0");

		auto transform = camera_0->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 12.0f, -20.0f, 1.0f);
		transform->RotatePitchYawRoll(XMFLOAT3(XM_PIDIV2 * 1.3f, 0, 0), true);

		auto physics = camera_0->CreateComponentPhysics();
		
		auto camera = camera_0->CreateComponentCamera();
		camera->CreatePerspectiveCamera(ECameraType::FreeLook);

		auto render = camera_0->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("CAMERA"));
	}

	{
		auto camera_1 = ecs.CreateEntity("camera_1");

		auto transform = camera_1->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 12.0f, 0.0f, 1.0f);

		auto physics = camera_1->CreateComponentPhysics();

		auto camera = camera_1->CreateComponentCamera();
		camera->CreatePerspectiveCamera(ECameraType::FreeLook);

		auto render = camera_1->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("CAMERA"));
	}

	{
		auto ambient_light = ecs.CreateEntity("ambient_light");

		auto transform = ambient_light->CreateComponentTransform();
		transform->Position = XMVectorSet(0.0f, 10.0f, 0.0f, 1.0f);
		
		auto light = ambient_light->CreateComponentLight();
		light->MakeAmbientLight(XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f);

		auto physics = ambient_light->CreateComponentPhysics();

		auto render = ambient_light->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("LIGHT"));

		//ecs.DestroyEntityByName("ambient_light");
	}
	
	{
		auto directional_light = ecs.CreateEntity("directional_light");

		auto transform = directional_light->CreateComponentTransform();
		transform->Position = XMVectorSet(3.0f, 10.0f, -3.0f, 1.0f);

		auto light = directional_light->CreateComponentLight();
		light->MakeDirectionalLight(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f), 0.6f);

		auto physics = directional_light->CreateComponentPhysics();

		auto render = directional_light->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("LIGHT"));

		//ecs.DestroyEntityByName("directional_light");
	}

	{
		auto box = ecs.CreateEntity("box");

		auto transform = box->CreateComponentTransform();
		transform->ScalingFactor = XMVectorSet(32, 1.0f, 32, 0.0f);
		transform->Position = XMVectorSet(0.0f, -10.0f, 0.0f, 1.0f);

		auto physics = box->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(30.0f);
		physics->SetMassToInfinite();
		physics->SetCollisionMesh(ecs.SystemRender().GetSharedModelByName("CM_box"));

		auto render = box->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("box"));
	}

	{
		auto jar = ecs.CreateEntity("jar");

		auto transform = jar->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(10.0f, 0.0f, 0.0f, 1.0f);
		transform->ScalingFactor = { 0.05f, 0.05f, 0.05f };

		auto physics = jar->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(0.8f, 0.0f, 0.6f, 0.0f);
		physics->SetMassByKilogram(1.0f);
		physics->SetCollisionMesh(ecs.SystemRender().GetSharedModelByName("CM_jar"));
		physics->FrictionData = ecs.SystemPhysics().GetMaterialFrictionData(EPhysicsMaterial::Ceramic);
		physics->Restitution = 0.4f;

		auto render = jar->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("jar"));
		render->SetRenderFlag(JWFlagComponentRenderOption_GetLit);
	}
	
	{
		auto oil_drum = ecs.CreateEntity("oil_drum");

		auto transform = oil_drum->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(-10.0f, 0.0f, 0.0f, 1.0f);
		//transform->SetPitchYawRoll(XMFLOAT3(0, 0, -0.6f));
		transform->ScalingFactor = { 0.1f, 0.1f, 0.1f };

		auto physics = oil_drum->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(1.1f);
		//physics->SetMassByKilogram(20.0f);
		physics->SetCollisionMesh(ecs.SystemRender().GetSharedModelByName("CM_oil_drum"));
		physics->FrictionData = ecs.SystemPhysics().GetMaterialFrictionData(EPhysicsMaterial::Iron);

		auto render = oil_drum->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("oil_drum"));
		render->SetRenderFlag(JWFlagComponentRenderOption_GetLit);
	}

	{
		auto recycling_bin = ecs.CreateEntity("recycling_bin");

		auto transform = recycling_bin->CreateComponentTransform();
		transform->WorldMatrixCalculationOrder = EWorldMatrixCalculationOrder::ScaleRotTrans;
		transform->Position = XMVectorSet(18.0f, 10.0f, 0.0f, 1.0f);
		transform->ScalingFactor = { 0.06f, 0.06f, 0.06f };

		auto physics = recycling_bin->CreateComponentPhysics();
		physics->BoundingSphere = SBoundingSphereData(1.6f, 0.0f, 0.26f, 0.0f);
		//physics->SetMassByKilogram(6.0f);
		//physics->SetCollisionMesh(ecs.SystemRender().GetSharedModelByName("CM_recycling_bin"));
		//physics->FrictionData = ecs.SystemPhysics().GetMaterialFrictionData(EPhysicsMaterial::Iron);

		auto render = recycling_bin->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("recycling_bin"));
		render->SetRenderFlag(JWFlagComponentRenderOption_GetLit);
	}

	{
		auto closest_a0 = ecs.CreateEntity("closest_a0");
		auto transform = closest_a0->CreateComponentTransform();
		auto render = closest_a0->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("POINT_3D_A"));
	}

	{
		auto closest_a = ecs.CreateEntity("closest_a1");
		auto transform = closest_a->CreateComponentTransform();
		auto render = closest_a->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("POINT_3D_A"));
	}

	{
		auto closest_a = ecs.CreateEntity("closest_a2");
		auto transform = closest_a->CreateComponentTransform();
		auto render = closest_a->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("POINT_3D_A"));
	}

	{
		auto closest_b = ecs.CreateEntity("closest_b0");
		auto transform = closest_b->CreateComponentTransform();
		auto render = closest_b->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("POINT_3D_B"));
	}

	{
		auto closest_b = ecs.CreateEntity("closest_b1");
		auto transform = closest_b->CreateComponentTransform();
		auto render = closest_b->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("POINT_3D_B"));
	}

	{
		auto closest_b = ecs.CreateEntity("closest_b2");
		auto transform = closest_b->CreateComponentTransform();
		auto render = closest_b->CreateComponentRender();
		render->SetModel(ecs.SystemRender().GetSharedModelByName("POINT_3D_B"));
	}

	ecs.SystemCamera().SetCurrentCamera(0);

	// For debugging - turn the physics by pressing number key '6'.
	ecs.SystemPhysics().ToggleSystemPhysicsFlag(JWFlagSystemPhysicsOption_ApplyForces);

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
		ecs.GetEntityByName("jar")->GetComponentPhysics()->Velocity += XMVectorSet(-1.0f, 0, 0, 0);
	}

	if (Character == '8')
	{
		ecs.SystemPhysics().ZeroAllVelocities();

		ecs.GetEntityByName("jar")->GetComponentTransform()->Position = XMVectorSet(0.0f, 8.0f, 0.0f, 1.0f);
		ecs.GetEntityByName("oil_drum")->GetComponentTransform()->Position = XMVectorSet(-10.0f, 0.0f, 0.0f, 1.0f);
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

	//ecs.GetEntityByName("closest_a0")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestPointA0();
	//ecs.GetEntityByName("closest_a1")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestPointA1();
	//ecs.GetEntityByName("closest_a2")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestPointA2();

	ecs.GetEntityByName("closest_a0")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestFaceA().V0;
	ecs.GetEntityByName("closest_a1")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestFaceA().V1;
	ecs.GetEntityByName("closest_a2")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestFaceA().V2;
	
	//ecs.GetEntityByName("closest_b0")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestPointB0();
	//ecs.GetEntityByName("closest_b1")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestPointB1();
	//ecs.GetEntityByName("closest_b2")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestPointB2();

	ecs.GetEntityByName("closest_b0")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestFaceB().V0;
	ecs.GetEntityByName("closest_b1")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestFaceB().V1;
	ecs.GetEntityByName("closest_b2")->GetComponentTransform()->Position = ecs.SystemPhysics().GetClosestFaceB().V2;


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
	static WSTRING s_is_there_collision{};
	static WSTRING s_penetration_depth{};

	s_fps = L"FPS: " + TO_WSTRING(myGame.GetFPS());
	s_anim_id = L"Animation ID: " + TO_WSTRING(anim_id);
	s_picked_entity = L"Picked Entity = " + StringToWstring(ecs.SystemPhysics().GetPickedEntityName());
	s_cull_count = L"Frustum culled entities = " + TO_WSTRING(ecs.SystemRender().GetFrustumCulledEntityCount());
	s_cull_count2 = L"Frustum culled terrain nodes = " + TO_WSTRING(ecs.SystemRender().GetFrustumCulledTerrainNodeCount());
	s_dt = L"Delta time = " + TO_WSTRING(ecs.GetDeltaTime());
	s_is_there_collision = L"Fine collision detected? = ";
	s_is_there_collision += ecs.SystemPhysics().IsThereAnyActualCollision() ? L"TRUE" : L"FALSE";
	s_penetration_depth = L"Penetration depth = ";
	s_penetration_depth += TO_WSTRING(ecs.SystemPhysics().GetPenetrationDepth());

	myGame.InstantText().BeginRendering();

	myGame.InstantText().RenderText(s_fps, XMFLOAT2(10, 10), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_anim_id, XMFLOAT2(10, 30), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_picked_entity, XMFLOAT2(10, 50), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_cull_count, XMFLOAT2(10, 70), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_cull_count2, XMFLOAT2(10, 90), XMFLOAT4(0, 0.2f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_dt, XMFLOAT2(10, 110), XMFLOAT4(0, 0.7f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_is_there_collision, XMFLOAT2(10, 130), XMFLOAT4(0, 0.7f, 0.7f, 1.0f));
	myGame.InstantText().RenderText(s_penetration_depth, XMFLOAT2(10, 150), XMFLOAT4(0, 0.7f, 0.7f, 1.0f));

	myGame.InstantText().EndRendering();
}