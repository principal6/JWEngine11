#include "JWGame.h"

using namespace JWEngine;

void JWGame::Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING GameFontFileName, JWLogger* PtrLogger) noexcept
{
	assert(PtrLogger);

	m_pLogger = PtrLogger;

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

		m_Window.Create(WindowPosition, WindowSize, Title);
		m_IsWindowCreated = true;

		m_hWnd = m_Window.GethWnd();

		LOG_ADD("Window created");

		m_ClearColor = SClearColor(0.6f, 0.6f, 1.0f);
		m_DX.Create(m_Window, m_BaseDirectory, m_ClearColor);
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

void JWGame::ToggleWireFrame() noexcept
{
	m_ECS.SystemRender().ToggleWireFrame();
}

void JWGame::SetUniversalRasterizerState(ERasterizerState State) noexcept
{
	m_ECS.SystemRender().SetUniversalRasterizerState(State);
}

void JWGame::CastPickingRay() noexcept
{
	// Get mouse cursor position in screen space (in client rect)
	GetCursorPos(&m_MouseClientPosition);
	ScreenToClient(m_hWnd, &m_MouseClientPosition);
	
	// Normalize mouse cursor position
	m_NormalizedMousePosition.x = (static_cast<float>(m_MouseClientPosition.x) / m_Window.GetWidth()) * 2.0f - 1.0f;
	m_NormalizedMousePosition.y = (static_cast<float>(m_MouseClientPosition.y) / m_Window.GetHeight()) * 2.0f - 1.0f;

	auto MatrixView = m_Camera.GetViewMatrix();
	auto MatrixProjection = m_Camera.GetProjectionMatrix();

	m_PickingRayViewSpacePosition = XMVectorSet(0, 0, 0.001f, 0);
	m_PickingRayViewSpaceDirection = XMVectorSet(
		+m_NormalizedMousePosition.x / XMVectorGetX(MatrixProjection.r[0]),
		-m_NormalizedMousePosition.y / XMVectorGetY(MatrixProjection.r[1]),
		1.0f,
		0.0f
	);

	auto MatrixViewInverse = XMMatrixInverse(nullptr, MatrixView);
	m_PickingRayOrigin = XMVector3TransformCoord(m_PickingRayViewSpacePosition, MatrixViewInverse);
	m_PickingRayDirection = XMVector3TransformNormal(m_PickingRayViewSpaceDirection, MatrixViewInverse);
}

void JWGame::PickEntityTriangle() noexcept
{
	m_ECS.PickEntityTriangle(m_PickingRayOrigin, m_PickingRayDirection);
}

auto JWGame::GetPickingRayOrigin() const noexcept->const XMVECTOR&
{
	return m_PickingRayOrigin;
}

auto JWGame::GetPickingRayDirection() const noexcept->const XMVECTOR&
{
	return m_PickingRayDirection;
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
			m_MouseCursorPosition.x = min(m_MouseCursorPosition.x, static_cast<float>(m_Window.GetWidth()));
			m_MouseCursorPosition.y = max(m_MouseCursorPosition.y, 0);
			m_MouseCursorPosition.y = min(m_MouseCursorPosition.y, static_cast<float>(m_Window.GetHeight()));

			// Call the outter OnInput function
			m_fpOnInput(m_InputDeviceState);

			// Update camera position into DefaultPSCBDefault
			m_DX.UpdatePSCBCamera(m_Camera.GetPositionFloat4());

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
	m_Camera.Destroy();
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