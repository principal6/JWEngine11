#pragma once

#include "../Core/JWWin32Window.h"
#include "../Core/JWDX.h"
#include "../Core/JWCamera.h"
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

		void Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING GameFontFileName, JWLogger* PtrLogger = nullptr) noexcept;

		void LoadCursorImage(STRING FileName) noexcept;

		void SetFunctionOnInput(FP_ON_INPUT Function) noexcept;
		void SetFunctionOnRender(FP_ON_RENDER Function) noexcept;
		void SetFunctionOnWindowsKeyDown(FP_ON_WINDOWS_KEY_DOWN Function) noexcept;
		void SetFunctionOnWindowsCharInput(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept;

		// ---------------------
		// --- Object getter ---
		auto& Camera() noexcept { return m_Camera; }
		auto& InstantText() noexcept { return m_InstantText; }
		auto& RawPixelSetter() noexcept { return m_RawPixelSetter; }
		auto& ECS() noexcept { return m_ECS; }
	
		// FPS
		auto GetFPS() noexcept { return m_FPS; };

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
		SClearColor				m_ClearColor{};

		FP_ON_INPUT				m_fpOnInput{};
		FP_ON_RENDER			m_fpOnRender{};

		JWWin32Window			m_Window{};
		JWDX					m_DX{};
		JWInput					m_Input{};
		SDirectInputDeviceState	m_InputDeviceState{};
		JWCamera				m_Camera{};
		JWInstantText			m_InstantText{};
		JWRawPixelSetter		m_RawPixelSetter{};
		JWECS					m_ECS{};

		JWLogger*				m_pLogger{};
		
		STEADY_CLOCK			m_Clock{};
		TIME_UNIT_MS			m_ElapsedTime{};
		TIME_POINT				m_TimeNow{};
		TIME_POINT				m_TimePrev{};
		uint32_t				m_FrameCount{};
		uint32_t				m_FPS{};

		JWImageCursor			m_MouseCursorImage{};
		XMFLOAT2				m_MouseCursorPosition{};
	};
};