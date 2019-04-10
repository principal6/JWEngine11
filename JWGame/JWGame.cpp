#include "JWGame.h"

using namespace JWEngine;

void JWGame::Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING GameFontFileName, JWLogger* PtrLogger) noexcept
{
	m_pLogger = PtrLogger;

	LOG_METHOD();

	assert(!m_IsCreated);

	char current_directory[KMaxFileLength];
	GetCurrentDirectory(KMaxFileLength, current_directory);

	STRING base_directory = current_directory;
	auto find_project_name = base_directory.find(KProjectName);
	base_directory = base_directory.substr(0, find_project_name - 1);
	base_directory += "\\";

	LOG_ADD("Base directory: " + base_directory);

	m_BaseDirectory = base_directory;

	m_ClearColor = SClearColor(0.6f, 0.6f, 1.0f);

	m_Window.Create(WindowPosition, WindowSize, Title);
	m_IsWindowCreated = true;

	LOG_ADD("Window created");

	m_DX.Create(m_Window, m_BaseDirectory);
	m_DX.SetRasterizerState(ERasterizerState::SolidNoCull);
	m_IsDXCreated = true;

	LOG_ADD("DX created");
	
	m_Input.Create(m_Window.GethWnd(), m_Window.GethInstance());

	LOG_ADD("Input created");

	m_Camera.Create(m_DX);

	LOG_ADD("Camera created");

	m_InstantText.Create(m_DX, m_Camera, m_BaseDirectory, KAssetDirectory + GameFontFileName);

	LOG_ADD("Instant text created");

	m_MouseCursorImage.Create(m_DX, m_Camera);

	m_RawPixelSetter.Create(m_DX);

	m_ECS.Create(m_DX, m_Camera, m_BaseDirectory);

	LOG_ADD("ECS created");

	m_IsCreated = true;

	LOG_METHOD_FINISH();
}

void JWGame::LoadCursorImage(STRING FileName) noexcept
{
	m_MouseCursorImage.LoadImageCursorFromFile(m_BaseDirectory + KAssetDirectory, FileName);

	m_IsMouseCursorLoaded = true;
}

void JWGame::SetFunctionOnRender(FP_ON_RENDER Function) noexcept
{
	assert(Function);
	m_fpOnRender = Function;
}

void JWGame::SetFunctionOnInput(FP_ON_INPUT Function) noexcept
{
	assert(Function);
	m_fpOnInput = Function;
}

void JWGame::SetFunctionOnWindowsKeyDown(FP_ON_WINDOWS_KEY_DOWN Function) noexcept
{
	assert(Function);
	m_Window.SetOnWindowsKeyDownFunction(Function);
}

void JWGame::SetFunctionOnWindowsCharInput(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept
{
	assert(Function);
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

void JWGame::Run() noexcept
{
	LOG_METHOD();

	assert(m_IsWindowCreated);
	assert(m_IsDXCreated);
	assert(m_fpOnRender);

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
			if (m_IsMouseCursorLoaded)
			{
				m_MouseCursorImage.SetPosition(m_MouseCursorPosition);
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

	LOG_METHOD_FINISH();

	LOG_SAVE(m_BaseDirectory);
}

void JWGame::Terminate() noexcept
{
	m_IsRunning = false;
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