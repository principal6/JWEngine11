#include "JWInput.h"

using namespace JWEngine;

void JWInput::Create(const HWND hWnd, const HINSTANCE hInstance) noexcept
{
	if (!m_IsCreated)
	{
		m_hWnd = hWnd;

		if (SUCCEEDED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)& m_pDirectInput, nullptr)))
		{
			// @ see
			// #0 (DISCL_BACKGROUND | DISCL_NONEXCLUSIVE)
			// #1 (DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND)
			CreateMouseDevice(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
			CreateKeyboardDevice(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

			m_IsCreated = true;
		}
		else
		{
			JW_ERROR_ABORT("DirectInput8Create() Failed");
		}
	}
}

void JWInput::Destroy() noexcept
{
	if (m_pMouseDevice)
	{
		m_pMouseDevice->Unacquire();
		JW_RELEASE(m_pMouseDevice);
	}
	if (m_pKeyboardDevice)
	{
		m_pKeyboardDevice->Unacquire();
		JW_RELEASE(m_pKeyboardDevice);
	}

	JW_RELEASE(m_pDirectInput);
}

PRIVATE void JWInput::CreateMouseDevice(DWORD dwFlags)
{
	if (FAILED(m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, nullptr)))
	{
		JW_ERROR_ABORT("CreateDevice() failed.");
	}
	
	if (FAILED(m_pMouseDevice->SetDataFormat(&c_dfDIMouse)))
	{
		JW_ERROR_ABORT("SetDataFormat() failed.");
	}

	if (FAILED(m_pMouseDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
	{
		JW_ERROR_ABORT("SetCooperativeLevel() failed.");
	}

	if (FAILED(m_pMouseDevice->Acquire()))
	{
		JW_ERROR_ABORT("Acquire() failed.");
	}
}

PRIVATE void JWInput::CreateKeyboardDevice(DWORD dwFlags)
{
	if (FAILED(m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, nullptr)))
	{
		JW_ERROR_ABORT("CreateDevice() failed.");
	}

	if (FAILED(m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
	{
		JW_ERROR_ABORT("SetDataFormat() failed.");
	}

	if (FAILED(m_pKeyboardDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
	{
		JW_ERROR_ABORT("SetCooperativeLevel() failed.");
	}

	if (FAILED(m_pKeyboardDevice->Acquire()))
	{
		JW_ERROR_ABORT("Acquire() failed.");
	}
}

PRIVATE void JWInput::UpdateDeviceState() noexcept
{
	m_DeviceState.PreviousMouse = m_DeviceState.CurrentMouse;

	m_pMouseDevice->GetDeviceState(sizeof(m_DeviceState.CurrentMouse), (LPVOID)&m_DeviceState.CurrentMouse);
	m_pKeyboardDevice->GetDeviceState(sizeof(m_DeviceState.Keys), (LPVOID)&m_DeviceState.Keys);
}

auto JWInput::GetDeviceState() noexcept->SDirectInputDeviceState&
{
	UpdateDeviceState();

	return m_DeviceState;
}