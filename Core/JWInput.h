#pragma once

#define DIRECTINPUT_VERSION 0x0800

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#include "JWCommon.h"
#include <dinput.h>

namespace JWEngine
{
	struct SDirectInputDeviceState
	{
		char Keys[KInputKeyCount]{};
		DIMOUSESTATE CurrentMouse{};
		DIMOUSESTATE PreviousMouse{};
	};

	class JWInput final
	{
	public:
		JWInput() = default;
		~JWInput();

		void Create(const HWND hWnd, const HINSTANCE hInstance);

		auto GetDeviceState() noexcept->SDirectInputDeviceState&;

	private:
		void CreateMouseDevice(DWORD dwFlags);
		void CreateKeyboardDevice(DWORD dwFlags);

		void UpdateDeviceState() noexcept;

	private:
		bool m_IsValid{ false };
		HWND m_hWnd{};

		LPDIRECTINPUT8			m_pDirectInput{};
		LPDIRECTINPUTDEVICE8	m_pKeyboardDevice{};
		LPDIRECTINPUTDEVICE8	m_pMouseDevice{};

		SDirectInputDeviceState	m_DeviceState{};
	};
};