#include "JWGame.h"

using namespace JWEngine;

void JWGame::Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING BaseDirectory, STRING GameFontFileName) noexcept
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	m_BaseDirectory = BaseDirectory;

	m_ClearColor = SClearColor(0.6f, 0.6f, 1.0f);

	m_Window.Create(WindowPosition, WindowSize, Title);
	m_IsWindowCreated = true;

	m_DX.Create(m_Window, m_BaseDirectory);
	m_DX.SetRasterizerState(ERasterizerState::SolidNoCull);
	m_IsDXCreated = true;
	
	m_Input.Create(m_Window.GethWnd(), m_Window.GethInstance());

	m_Camera.Create(m_DX);

	m_InstantText.Create(m_DX, m_Camera, BaseDirectory, KAssetDirectory + GameFontFileName);

	m_DesignerUI.Create(m_DX, m_Camera, BaseDirectory);

	m_MouseCursorImage.Create(m_DX, m_Camera);

	m_IsValid = true;
}

void JWGame::LoadCursorImage(STRING FileName) noexcept
{
	m_MouseCursorImage.LoadImageFromFile(m_BaseDirectory + KAssetDirectory, FileName);
}

void JWGame::SetOnRenderFunction(FP_ON_RENDER OnRender) noexcept
{
	if (OnRender == nullptr)
	{
		JWAbort("FP_ON_RENDER OnRender is nullptr.");
	}

	m_fpOnRender = OnRender;
}

void JWGame::SetOnInputFunction(FP_ON_INPUT OnInput) noexcept
{
	if (OnInput == nullptr)
	{
		JWAbort("FP_ON_INPUT OnInput is nullptr.");
	}

	m_fpOnInput = OnInput;
}

void JWGame::SetRasterizerState(ERasterizerState State) noexcept
{
	m_DX.SetRasterizerState(State);
}

void JWGame::SetBlendState(EBlendState State) noexcept
{
	m_DX.SetBlendState(State);
}

PRIVATE void JWGame::SetDepthStencilState(EDepthStencilState State) noexcept
{
	m_DX.SetDepthStencilState(State);
}

void JWGame::AddOpaqueModel(STRING ModelFileName) noexcept
{
	m_pOpaqueModels.push_back(MAKE_UNIQUE_AND_MOVE(JWModel)());

	m_pOpaqueModels[m_pOpaqueModels.size() - 1]->Create(m_DX, m_Camera);
	m_pOpaqueModels[m_pOpaqueModels.size() - 1]->SetModelData(m_AssimpLoader.LoadObj(m_BaseDirectory + KAssetDirectory, ModelFileName));
}

auto JWGame::GetOpaqueModel(size_t OpaqueModelIndex) const noexcept->JWModel&
{
	return *m_pOpaqueModels[OpaqueModelIndex].get();
}

void JWGame::AddTransparentModel(STRING ModelFileName) noexcept
{
	m_pTransparentModels.push_back(MAKE_UNIQUE_AND_MOVE(JWModel)());

	m_pTransparentModels[m_pTransparentModels.size() - 1]->Create(m_DX, m_Camera);
	m_pTransparentModels[m_pTransparentModels.size() - 1]->SetModelData(m_AssimpLoader.LoadObj(m_BaseDirectory + KAssetDirectory, ModelFileName));
}

auto JWGame::GetTransparentModel(size_t TransparentModelIndex) const noexcept->JWModel&
{
	return *m_pTransparentModels[TransparentModelIndex].get();
}

void JWGame::AddImage(STRING ImageFileName) noexcept
{
	m_p2DImages.push_back(MAKE_UNIQUE_AND_MOVE(JWImage)());

	m_p2DImages[m_p2DImages.size() - 1]->Create(m_DX, m_Camera);
	m_p2DImages[m_p2DImages.size() - 1]->LoadImageFromFile(m_BaseDirectory + KAssetDirectory, ImageFileName);
}

auto JWGame::GetImage(size_t Image2DIndex) const noexcept->JWImage&
{
	return *m_p2DImages[Image2DIndex].get();
}

void JWGame::AddLight(SLightData LightData) noexcept
{
	if (LightData.LightType != ELightType::Invalid)
	{
		m_DesignerUI.AddLightData(LightData);
	}
}

auto JWGame::GetCameraObject() noexcept->JWCamera&
{
	return m_Camera;
}

auto JWGame::GetInstantTextObject() noexcept->JWInstantText&
{
	return m_InstantText;
}

auto JWGame::GetFPS() noexcept->int
{
	return m_FPS;
}

PRIVATE void JWGame::CheckValidity() const noexcept
{
	if (!m_IsWindowCreated)
	{
		JWAbort("Win32 window not created.\nYou must call JWGame::Create()");
	}
	if (!m_IsDXCreated)
	{
		JWAbort("DirectX objects not created.\nYou must call JWGame::Create()");
	}
	if (!m_fpOnRender)
	{
		JWAbort("m_pMainLoop is nullptr.\nYou must call JWGame::SetMainLoopFunction()");
	}
}

void JWGame::Run() noexcept
{
	CheckValidity();

	MSG msg{};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Advance FPSCount
			++m_FPSCount;

			// Update input device state
			m_InputDeviceState = m_Input.GetDeviceState();

			// Update mouse cursor position
			m_MouseCursorPosition.x += static_cast<float>(m_InputDeviceState.CurrentMouse.lX);
			m_MouseCursorPosition.y += static_cast<float>(m_InputDeviceState.CurrentMouse.lY);

			// Limit mouse cursor position to window size
			m_MouseCursorPosition.x = max(m_MouseCursorPosition.x, 0);
			m_MouseCursorPosition.x = min(m_MouseCursorPosition.x, static_cast<float>(m_Window.GetWidth()));
			m_MouseCursorPosition.y = max(m_MouseCursorPosition.y, 0);
			m_MouseCursorPosition.y = min(m_MouseCursorPosition.y, static_cast<float>(m_Window.GetHeight()));

			// Call the outter OnInput function
			m_fpOnInput(m_InputDeviceState);

			m_DX.BeginDrawing(m_ClearColor);

			// Call the outter OnRender function.
			m_fpOnRender();

			// Draw mouse cursor if it exists
			if (m_MouseCursorImage.IsImageLoaded())
			{
				m_MouseCursorImage.SetPosition(m_MouseCursorPosition);
				m_MouseCursorImage.UpdateAll();
				m_MouseCursorImage.Draw();
			}

			m_DX.EndDrawing();

			if (m_Timer.GetElapsedTimeMilliSec() >= 1000)
			{
				// Save FPS
				m_FPS = static_cast<int>(m_FPSCount);

				// Reset FPSCount
				m_FPSCount = 0;

				// Reset timer
				m_Timer.ResetTimer();
			}
		}
	}
}

void JWGame::DrawDesignerUI() noexcept
{
	// Enable Z-buffer
	SetDepthStencilState(EDepthStencilState::ZEnabled);

	// Draw designer UI
	SetBlendState(EBlendState::Opaque);
	m_DesignerUI.Draw();
}

void JWGame::DrawModels() noexcept
{
	// Enable Z-buffer (3D Drawing)
	SetDepthStencilState(EDepthStencilState::ZEnabled);

	// Draw 3D models
	DrawAllOpaqueModels();
	SetBlendState(EBlendState::Transprent);
	DrawAllTransparentModels();
}

void JWGame::DrawImages() noexcept
{
	// Disable Z-buffer (2D Drawing)
	SetDepthStencilState(EDepthStencilState::ZDisabled);

	// Draw 2D images
	// with Z-buffer disabled, in order to draw them on top of everything else
	SetBlendState(EBlendState::Opaque);
	DrawAll2DImages();
}

void JWGame::DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept
{
	SetDepthStencilState(EDepthStencilState::ZDisabled);

	SetBlendState(EBlendState::Transprent);
	m_InstantText.DrawInstantText(Text, Position, FontColorRGB);
}

PRIVATE void JWGame::DrawAllOpaqueModels() const noexcept
{
	if (m_pOpaqueModels.size())
	{
		for (auto& iterator_model : m_pOpaqueModels)
		{
			iterator_model->Draw();
		}
	}
}

PRIVATE void JWGame::DrawAllTransparentModels() const noexcept
{
	if (m_pTransparentModels.size())
	{
		//VECTOR<size_t> draw_oreder;

		for (size_t iterator_index{}; iterator_index < m_pTransparentModels.size(); ++iterator_index)
		{
			//m_pTransparentModels[iterator_model]->GetDistanceFromCamera();
			m_pTransparentModels[iterator_index]->Draw();
		}
	}
}

PRIVATE void JWGame::DrawAll2DImages() const noexcept
{
	if (m_p2DImages.size())
	{
		for (auto& iterator_2d_image : m_p2DImages)
		{
			iterator_2d_image->UpdateAll();
			iterator_2d_image->Draw();
		}
	}
}