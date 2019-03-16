#include "JWGame.h"

using namespace JWEngine;

PRIVATE inline auto JWGame::GetFileNameWithBaseDirectory(STRING& FileName) noexcept->STRING
{
	return m_BaseDirectory + FileName;
}

void JWGame::Create(int Width, int Height, STRING Title, STRING Directory) noexcept
{
	m_BaseDirectory = Directory;

	m_ClearColor = SClearColor(0.6f, 0.6f, 1.0f);

	m_Window.Create(800, 600, Title);
	m_IsWindowCreated = true;

	m_DX.Create(m_Window, Directory);
	m_IsDXCreated = true;
	
	m_Camera.Create(m_DX);
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

PRIVATE void JWGame::SetBlendState(EBlendState State) noexcept
{
	m_DX.SetBlendState(State);
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

auto JWGame::GetCameraObject() noexcept->JWCamera&
{
	return m_Camera;
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
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
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
			m_DX.BeginDrawing(m_ClearColor);

			// Call the outter render function.
			m_fpRender();

			m_DX.EndDrawing();
		}
	}
}

void JWGame::DrawAllModels() noexcept
{
	SetBlendState(EBlendState::Opaque);
	DrawAllOpaqueModels();

	SetBlendState(EBlendState::Transprent);
	DrawAllTransparentModels();
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

		for (size_t iterator_model{}; iterator_model < m_pTransparentModels.size(); ++iterator_model)
		{
			//m_pTransparentModels[iterator_model]->GetDistanceFromCamera();
			m_pTransparentModels[iterator_model]->Draw();
		}
	}
}