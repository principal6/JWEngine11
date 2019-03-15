#include "JWGame.h"

using namespace JWEngine;

void JWGame::Create(int Width, int Height, STRING Title) noexcept
{
	m_ClearColor = SClearColor(0.6f, 0.6f, 1.0f);

	m_Window.Create(800, 600, Title);
	m_IsWindowCreated = true;

	m_DX.Create(m_Window);
	m_IsDXCreated = true;
	
	m_Camera.Create(m_DX);
	
	m_Mesh.Create(m_DX, m_Camera);
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

auto JWGame::GetCameraObject() noexcept->JWCamera&
{
	return m_Camera;
}

auto JWGame::GetModelObject() noexcept->JWModel&
{
	return m_Mesh;
}