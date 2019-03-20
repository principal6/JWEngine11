#include "JWGame.h"

using namespace JWEngine;

PRIVATE inline auto JWGame::GetFileNameWithBaseDirectory(const STRING& FileName) const noexcept->STRING
{
	return m_BaseDirectory + FileName;
}

void JWGame::Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING BaseDirectory, STRING GameFontFileName) noexcept
{
	AVOID_DUPLICATE_CREATION(m_IsValid);

	m_BaseDirectory = BaseDirectory;

	m_ClearColor = SClearColor(0.6f, 0.6f, 1.0f);

	m_Window.Create(WindowPosition, WindowSize, Title);
	m_IsWindowCreated = true;

	m_DX.Create(m_Window, m_BaseDirectory);
	m_IsDXCreated = true;
	
	m_Camera.Create(m_DX);

	m_InstantText.Create(m_DX, m_Camera, BaseDirectory, GameFontFileName);

	m_IsValid = true;
}

void JWGame::SetRenderFunction(FP_RENDER Render) noexcept
{
	if (Render == nullptr)
	{
		JWAbort("FP_MAINLOOP MainLoop is nullptr.");
	}

	m_fpRender = Render;
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

void JWGame::AddOpaqueModel(STRING Directory, STRING ModelFileName) noexcept
{
	m_pOpaqueModels.push_back(MAKE_UNIQUE_AND_MOVE(JWModel)());

	m_pOpaqueModels[m_pOpaqueModels.size() - 1]->Create(m_DX, m_Camera);
	m_pOpaqueModels[m_pOpaqueModels.size() - 1]->LoadModelObj(GetFileNameWithBaseDirectory(Directory), ModelFileName);
}

auto JWGame::GetOpaqueModel(size_t ModelIndex) const noexcept->JWModel&
{
	return *m_pOpaqueModels[ModelIndex].get();
}

void JWGame::AddTransparentModel(STRING Directory, STRING TransparentModelFileName) noexcept
{
	m_pTransparentModels.push_back(MAKE_UNIQUE_AND_MOVE(JWModel)());

	m_pTransparentModels[m_pTransparentModels.size() - 1]->Create(m_DX, m_Camera);
	m_pTransparentModels[m_pTransparentModels.size() - 1]->LoadModelObj(GetFileNameWithBaseDirectory(Directory), TransparentModelFileName);
}

auto JWGame::GetTransparentModel(size_t TransparentModelIndex) const noexcept->JWModel&
{
	return *m_pTransparentModels[TransparentModelIndex].get();
}

void JWGame::AddImage(STRING Directory, STRING ImageFileName) noexcept
{
	m_p2DImages.push_back(MAKE_UNIQUE_AND_MOVE(JWImage)());

	m_p2DImages[m_p2DImages.size() - 1]->Create(m_DX, m_Camera);
	m_p2DImages[m_p2DImages.size() - 1]->LoadImageFromFile(GetFileNameWithBaseDirectory(Directory), ImageFileName);
}

auto JWGame::GetImage(size_t Image2DIndex) const noexcept->JWImage&
{
	return *m_p2DImages[Image2DIndex].get();
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
	if (!m_fpRender)
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

			m_DX.BeginDrawing(m_ClearColor);

			// Call the outter render function.
			m_fpRender();

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

void JWGame::DrawModelsAndImages() noexcept
{
	m_DX.SetDefaultVS();
	m_DX.SetDefaultPS();

	// Draw 3D models
	SetDepthStencilState(EDepthStencilState::ZEnabled);
	SetBlendState(EBlendState::Opaque);
	DrawAllOpaqueModels();

	SetBlendState(EBlendState::Transprent);
	DrawAllTransparentModels();

	// Draw 2d images
	// with z-buffer disabled, in order to draw them on top of everything else
	SetBlendState(EBlendState::Opaque);
	SetDepthStencilState(EDepthStencilState::ZDisabled);
	DrawAll2DImages();
}

void JWGame::DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept
{
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