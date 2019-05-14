#include "JWGame.h"

using namespace JWEngine;

static JWGame* gs_pJWGame{};

JW_FUNCTION_ON_WINDOWS_RESIZE(OnResize)
{
	BOOL is_fullscreen{ false };
	IDXGIOutput* dxgi_output{};

	gs_pJWGame->DX().GetSwapChain()->GetFullscreenState(&is_fullscreen, &dxgi_output);

	if (is_fullscreen == TRUE)
	{
		auto mode{ gs_pJWGame->DX().GetFullScreenDisplayMode() };
		gs_pJWGame->UpdateWindowSize(mode);

		gs_pJWGame->DX().SetToFullScreenDisplayMode();
	}
	else
	{
		auto mode{ gs_pJWGame->DX().GetWindowedDisplayMode() };
		gs_pJWGame->UpdateWindowSize(mode);

		gs_pJWGame->DX().SetToWindowedDisplayMode();
	}

	gs_pJWGame->UpdateECSSize();

	JW_RELEASE(dxgi_output);
}

void JWGame::Create(EAllowedDisplayMode DisplayMode, SPosition2 WindowPosition, STRING WindowTitle,
	STRING GameFontFileName, JWLogger& Logger) noexcept
{
	gs_pJWGame = this;

	m_pLogger = &Logger;

	LOG_METHOD();

	if (!m_IsCreated)
	{
		char current_directory[KMaxFileLength];
		GetCurrentDirectory(KMaxFileLength, current_directory);

		STRING base_directory = current_directory;
		auto find_project_name = base_directory.find(KProjectName);
		base_directory = base_directory.substr(0, find_project_name - 1);
		base_directory += "\\";

		LOG_ADD("Base directory: " + base_directory);

		m_BaseDirectory = base_directory;

		m_WindowSize = ConvertEAllowedDisplayModeToModeSize(DisplayMode);
		m_Window.Create(WindowPosition, m_WindowSize, WindowTitle);
		m_Window.SetOnWindowsResizeFunction(OnResize);
		m_IsWindowCreated = true;

		LOG_ADD("Window created");

		m_ClearColor = SClearColor(0.6f, 0.6f, 1.0f);
		m_DX.Create(m_Window.GethWnd(), m_WindowSize, DisplayMode, m_BaseDirectory, m_ClearColor);
		m_DX.SetRasterizerState(ERasterizerState::SolidNoCull);
		
		m_IsDXCreated = true;

		LOG_ADD("DX created");

		m_Input.Create(m_Window.GethWnd(), m_Window.GethInstance());

		LOG_ADD("Input created");

		m_InstantText.Create(m_DX, m_WindowSize, m_BaseDirectory, KAssetDirectory + GameFontFileName);

		LOG_ADD("Instant text created");

		m_MouseCursorImage.Create(m_DX, m_WindowSize);

		m_RawPixelSetter.Create(m_DX, m_WindowSize);

		m_ECS.Create(m_DX, m_Window.GethWnd(), m_WindowSize, m_BaseDirectory);

		LOG_ADD("ECS created");

		m_IsCreated = true;
	}

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

void JWGame::SetGameDisplayMode(EAllowedDisplayMode Mode) noexcept
{
	// @important
	// First of all, update m_WindowSize.
	UpdateWindowSize(Mode);

	// Change display mode.
	m_DX.SetCurrentDisplayMode(Mode);

	// @important
	UpdateECSSize();
}

void JWGame::UpdateWindowSize(EAllowedDisplayMode Mode) noexcept
{
	m_WindowSize = KAllowedDisplayModes[static_cast<DisplayModeIndex>(Mode)];
}

void JWGame::UpdateECSSize() noexcept
{
	m_ECS.SystemCamera().UpdateCamerasProjectionMatrix();
	m_ECS.SystemRender().UpdateImage2Ds();
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
			// Update input device state
			m_InputDeviceState = m_Input.GetDeviceState();

			// Update mouse cursor position
			m_MouseCursorPosition.x += static_cast<float>(m_InputDeviceState.CurrentMouse.lX);
			m_MouseCursorPosition.y += static_cast<float>(m_InputDeviceState.CurrentMouse.lY);

			// Limit mouse cursor position to window size
			m_MouseCursorPosition.x = max(m_MouseCursorPosition.x, 0);
			m_MouseCursorPosition.x = min(m_MouseCursorPosition.x, m_WindowSize.floatX());
			m_MouseCursorPosition.y = max(m_MouseCursorPosition.y, 0);
			m_MouseCursorPosition.y = min(m_MouseCursorPosition.y, m_WindowSize.floatY());

			// Call the outter OnInput function
			m_fpOnInput(m_InputDeviceState);

			// Begin the drawing process
			m_DX.BeginDrawing();

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

			// Advance frame count
			++m_FrameCount;

			m_TimeNow = m_Clock.now();
			m_ElapsedTime = std::chrono::duration_cast<TIME_UNIT_MS>(m_TimeNow - m_TimePrev);
			if (m_ElapsedTime.count() >= 1000)
			{
				// Save FPS
				m_FPS = m_FrameCount;

				// Reset FPSCount
				m_FrameCount = 0;

				// Reset timer
				m_TimePrev = m_TimeNow;
			}
		}
	}

	// Destroy all the objects
	m_ECS.Destroy();
	m_RawPixelSetter.Destroy();
	m_InstantText.Destroy();
	m_Input.Destroy();
	m_DX.Destroy();
	m_Window.Destroy();

	LOG_METHOD_FINISH();

	LOG_SAVE(m_BaseDirectory);
}

void JWGame::Halt() noexcept
{
	m_IsRunning = false;
}