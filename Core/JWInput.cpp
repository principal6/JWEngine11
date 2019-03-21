#include "JWInput.h"

using namespace JWEngine;

JWInput::~JWInput()
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

void JWInput::Create(const HWND hWnd, const HINSTANCE hInstance)
{
	JW_AVOID_DUPLICATE_CREATION(m_IsValid);

	m_hWnd = hWnd;

	if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_pDirectInput, nullptr)))
	{
		JWAbort("JWInput not created. DirectInput8Create() failed");
	}

	
	//CreateMouseDevice(DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	CreateMouseDevice(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	CreateKeyboardDevice(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	m_IsValid = true;
}

PRIVATE void JWInput::CreateMouseDevice(DWORD dwFlags)
{
	if (FAILED(m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, nullptr)))
		JWAbort("CreateMouseDevice() failed.");
	
	if (FAILED(m_pMouseDevice->SetDataFormat(&c_dfDIMouse)))
		JWAbort("CreateMouseDevice() failed.");

	if (FAILED(m_pMouseDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
		JWAbort("CreateMouseDevice() failed.");

	if (FAILED(m_pMouseDevice->Acquire()))
		JWAbort("CreateMouseDevice() failed.");
}

PRIVATE void JWInput::CreateKeyboardDevice(DWORD dwFlags)
{
	if (FAILED(m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, nullptr)))
		JWAbort("CreateKeyboardDevice() failed.");

	if (FAILED(m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)))
		JWAbort("CreateKeyboardDevice() failed.");

	if (FAILED(m_pKeyboardDevice->SetCooperativeLevel(m_hWnd, dwFlags)))
		JWAbort("CreateKeyboardDevice() failed.");

	if (FAILED(m_pKeyboardDevice->Acquire()))
		JWAbort("CreateKeyboardDevice() failed.");
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