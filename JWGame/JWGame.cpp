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

	m_RawPixelSetter.Create(m_DX);

	m_ECS.Create(m_DX, m_Camera, BaseDirectory);

	m_IsValid = true;
}

void JWGame::LoadCursorImage(STRING FileName) noexcept
{
	m_MouseCursorImage.LoadImageFromFile(m_BaseDirectory + KAssetDirectory, FileName);
}

void JWGame::SetFunctionOnRender(FP_ON_RENDER Function) noexcept
{
	if (Function == nullptr)
	{
		JWAbort("FP_ON_RENDER Function is nullptr.");
	}

	m_fpOnRender = Function;
}

void JWGame::SetFunctionOnInput(FP_ON_INPUT Function) noexcept
{
	if (Function == nullptr)
	{
		JWAbort("FP_ON_INPUT Function is nullptr.");
	}

	m_fpOnInput = Function;
}

void JWGame::SetFunctionOnWindowsKeyDown(FP_ON_WINDOWS_KEY_DOWN Function) noexcept
{
	if (Function == nullptr)
	{
		JWAbort("FP_ON_WINDOWS_KEYDOWN Function is nullptr.");
	}

	m_Window.SetOnWindowsKeyDownFunction(Function);
}

void JWGame::SetFunctionOnWindowsCharInput(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept
{
	if (Function == nullptr)
	{
		JWAbort("FP_ON_WINDOWS_CHAR_INPUT Function is nullptr.");
	}

	m_Window.SetOnWindowsCharInputFunction(Function);
}

void JWGame::ToggleWireFrame() noexcept
{
	if (m_RasterizerState == ERasterizerState::WireFrame)
	{
		m_RasterizerState = ERasterizerState::SolidNoCull;
	}
	else
	{
		m_RasterizerState = ERasterizerState::WireFrame;
	}
	
	m_DX.SetRasterizerState(m_RasterizerState);
}

void JWGame::SetRasterizerState(ERasterizerState State) noexcept
{
	m_RasterizerState = State;
	m_DX.SetRasterizerState(m_RasterizerState);
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

auto JWGame::Camera() noexcept->JWCamera&
{
	return m_Camera;
}

auto JWGame::InstantText() noexcept->JWInstantText&
{
	return m_InstantText;
}

auto JWGame::RawPixelSetter() noexcept->JWRawPixelSetter&
{
	return m_RawPixelSetter;
}

auto JWGame::ECS() noexcept->JWECS&
{
	return m_ECS;
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
		JWAbort("m_pMainLoop is nullptr.\nYou must call JWGame::SetFunctionOnRender()");
	}
}

void JWGame::Run() noexcept
{
	CheckValidity();

	m_IsRunning = true;

	MSG msg{};

	while (m_IsRunning)
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

			// Update camera position into DefaultPSCBDefault
			m_DX.UpdatePSCBCamera(m_Camera.GetPositionFloat4());

			// Begin the drawing process
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

			// End the drawing process
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

void JWGame::Terminate() noexcept
{
	m_IsRunning = false;
}

void JWGame::UpdateEntities() noexcept
{
	m_ECS.UpdateAll();
}

void JWGame::DrawDesignerUI() noexcept
{
	// Draw designer UI
	m_DesignerUI.Draw();
	m_DesignerUI.DrawMiniAxis();
}

void JWGame::DrawImages() noexcept
{
	// Draw 2D images
	// with Z-buffer disabled, in order to draw them on top of everything else
	// Set blend state
	m_DX.SetBlendState(EBlendState::Opaque);
	DrawAll2DImages();
}

void JWGame::DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept
{
	if (m_RasterizerState == ERasterizerState::WireFrame)
	{
		ToggleWireFrame();

		m_DX.SetBlendState(EBlendState::Transprent);
		m_InstantText.DrawInstantText(Text, Position, FontColorRGB);

		ToggleWireFrame();
	}
	else
	{
		m_DX.SetBlendState(EBlendState::Transprent);
		m_InstantText.DrawInstantText(Text, Position, FontColorRGB);
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