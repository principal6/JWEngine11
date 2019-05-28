#pragma once

#include "../Core/JWWin32Window.h"
#include "../Core/JWDX.h"
#include "../Core/JWImage.h"
#include "../Core/JWImageCursor.h"
#include "../Core/JWInstantText.h"
#include "../Core/JWInput.h"
#include "../Core/JWRawPixelSetter.h"
#include "../ECS/JWECS.h"

namespace JWEngine
{
	static constexpr const char* KProjectName{ "JWGame" };
	static constexpr const char* KLogFileName{ "Log.txt" };

	using FP_ON_INPUT = void(*)(SDirectInputDeviceState&);
	using FP_ON_RENDER = void(*)(void);

#define JW_FUNCTION_ON_INPUT(FunctionName) void FunctionName(SDirectInputDeviceState& DeviceState)
#define JW_FUNCTION_ON_RENDER(FunctionName) void FunctionName()

	class JWGame
	{
	public:
		JWGame() = default;
		~JWGame() = default;

		void Create(EAllowedDisplayMode DisplayMode, SPosition2 WindowPosition, STRING WindowTitle, STRING GameFontFileName) noexcept;

		void LoadCursorImage(STRING FileName) noexcept;

		void SetFunctionOnInput(FP_ON_INPUT Function) noexcept;
		void SetFunctionOnRender(FP_ON_RENDER Function) noexcept;
		void SetFunctionOnWindowsKeyDown(FP_ON_WINDOWS_KEY_DOWN Function) noexcept;
		void SetFunctionOnWindowsCharInput(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept;

		void SetGameDisplayMode(EAllowedDisplayMode Mode) noexcept;
		void UpdateWindowSize(EAllowedDisplayMode Mode) noexcept;
		void UpdateECSSize() noexcept;

	public:

		// ---------------------
		// --- Object getter ---
		auto& InstantText() noexcept { return m_InstantText; }
		auto& RawPixelSetter() noexcept { return m_RawPixelSetter; }
		auto& ECS() noexcept { return m_ECS; }
		auto& DX() noexcept { return m_DX; }
		auto& Window() noexcept { return m_Window; }
	
		// Getter
		const auto& GetFPS() noexcept { return m_FPS; };
		const auto& GetWindowSize() noexcept { return m_WindowSize; }

		// Base directory
		auto GetBaseDirectory() const noexcept { return m_BaseDirectory; }

		void Run() noexcept;
		void Halt() noexcept;

	private:
		bool					m_IsCreated{ false };
		bool					m_IsWindowCreated{ false };
		bool					m_IsDXCreated{ false };
		bool					m_IsRunning{ false };
		bool					m_IsMouseCursorLoaded{ false };
		
		STRING					m_BaseDirectory;
		SSize2					m_ScreenResolution{};
		SSize2					m_WindowSize{};
		SClearColor				m_ClearColor{};

		FP_ON_INPUT				m_fpOnInput{};
		FP_ON_RENDER			m_fpOnRender{};

		JWWin32Window			m_Window{};
		JWDX					m_DX{};
		JWInput					m_Input{};
		SDirectInputDeviceState	m_InputDeviceState{};
		JWInstantText			m_InstantText{};
		JWRawPixelSetter		m_RawPixelSetter{};
		JWECS					m_ECS{};

		STEADY_CLOCK			m_Clock{};
		// This is for FPS calculation
		TIME_UNIT_MS			m_FPSElapsedTime{};
		TIME_POINT				m_FPSTimeNow{};
		TIME_POINT				m_FPSTimePrev{};
		uint32_t				m_FrameCount{};
		uint32_t				m_FPS{};

		// This is for Physics update
		TIME_UNIT_MS			m_FrameDeltaTime{};
		TIME_POINT				m_FrameStartTime{};
		TIME_POINT				m_FrameStartTimePrev{};

		JWImageCursor			m_MouseCursorImage{};
		XMFLOAT2				m_MouseCursorPosition{};
	};
};