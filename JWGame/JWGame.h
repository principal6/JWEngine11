#pragma once

#include "../Core/JWWin32Window.h"
#include "../Core/JWDX.h"
#include "../Core/JWCamera.h"
#include "../Core/JWImage.h"
#include "../Core/JWInstantText.h"
#include "../Core/JWTimer.h"
#include "../Core/JWDesignerUI.h"
#include "../Core/JWInput.h"
#include "../Core/JWRawPixelSetter.h"
#include "../ECS/JWECS.h"

namespace JWEngine
{
	using FP_ON_INPUT = void(*)(SDirectInputDeviceState&);
	using FP_ON_RENDER = void(*)(void);

#define JW_FUNCTION_ON_INPUT(FunctionName) void FunctionName(SDirectInputDeviceState& DeviceState)
#define JW_FUNCTION_ON_RENDER(FunctionName) void FunctionName()

	class JWGame
	{
	public:
		JWGame() = default;
		~JWGame() = default;

		void Create(SPositionInt WindowPosition, SSizeInt WindowSize, STRING Title, STRING BaseDirectory, STRING GameFontFileName) noexcept;

		void LoadCursorImage(STRING FileName) noexcept;

		void SetFunctionOnInput(FP_ON_INPUT Function) noexcept;
		void SetFunctionOnRender(FP_ON_RENDER Function) noexcept;
		void SetFunctionOnWindowsKeyDown(FP_ON_WINDOWS_KEY_DOWN Function) noexcept;
		void SetFunctionOnWindowsCharInput(FP_ON_WINDOWS_CHAR_INPUT Function) noexcept;

		void ToggleWireFrame() noexcept;
		void SetRasterizerState(ERasterizerState State) noexcept;

		void AddImage(STRING ImageFileName) noexcept;
		auto GetImage(size_t Image2DIndex) const noexcept->JWImage&;

		// Object getter
		auto Camera() noexcept->JWCamera&;
		auto InstantText() noexcept->JWInstantText&;
		auto RawPixelSetter() noexcept->JWRawPixelSetter&;
		auto ECS() noexcept->JWECS&;

		auto GetFPS() noexcept->int;

		void Run() noexcept;
		void Terminate() noexcept;

		void UpdateEntities() noexcept;

		void DrawDesignerUI() noexcept;
		void DrawImages() noexcept;
		void DrawInstantText(STRING Text, XMFLOAT2 Position, XMFLOAT3 FontColorRGB) noexcept;

	private:
		void CheckValidity() const noexcept;

		void DrawAll2DImages() const noexcept;

	private:
		bool m_IsValid{ false };
		bool m_IsWindowCreated{ false };
		bool m_IsDXCreated{ false };
		bool m_IsRunning{ false };
		
		STRING m_BaseDirectory;
		SClearColor m_ClearColor{};

		FP_ON_INPUT m_fpOnInput{};
		FP_ON_RENDER m_fpOnRender{};

		JWWin32Window m_Window{};
		JWDX m_DX{};
		JWInput m_Input{};
		SDirectInputDeviceState m_InputDeviceState{};
		JWCamera m_Camera{};
		JWInstantText m_InstantText{};
		JWDesignerUI m_DesignerUI{};
		JWRawPixelSetter m_RawPixelSetter{};
		JWECS m_ECS{};

		JWTimer m_Timer{};
		long long m_FPSCount{};
		int m_FPS{};

		VECTOR<UNIQUE_PTR<JWImage>> m_p2DImages;

		JWImage m_MouseCursorImage{};
		XMFLOAT2 m_MouseCursorPosition{};
		
		ERasterizerState m_RasterizerState{ ERasterizerState::SolidNoCull };
	};
};