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
	assert(!m_IsCreated);

	m_hWnd = hWnd;

	assert(SUCCEEDED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)& m_pDirectInput, nullptr)));
	
	//CreateMouseDevice(DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	CreateMouseDevice(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	CreateKeyboardDevice(DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	m_IsCreated = true;
}

PRIVATE void JWInput::CreateMouseDevice(DWORD dwFlags)
{
	assert(SUCCEEDED(m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouseDevice, nullptr)));
	
	assert(SUCCEEDED(m_pMouseDevice->SetDataFormat(&c_dfDIMouse)));

	assert(SUCCEEDED(m_pMouseDevice->SetCooperativeLevel(m_hWnd, dwFlags)));

	assert(SUCCEEDED(m_pMouseDevice->Acquire()));
}

PRIVATE void JWInput::CreateKeyboardDevice(DWORD dwFlags)
{
	assert(SUCCEEDED(m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDevice, nullptr)));

	assert(SUCCEEDED(m_pKeyboardDevice->SetDataFormat(&c_dfDIKeyboard)));

	assert(SUCCEEDED(m_pKeyboardDevice->SetCooperativeLevel(m_hWnd, dwFlags)));

	assert(SUCCEEDED(m_pKeyboardDevice->Acquire()));
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