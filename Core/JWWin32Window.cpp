#include "JWWin32Window.h"

using namespace JWEngine;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hWnd);
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void JWWin32Window::Create(int Width, int Height, const STRING& Title) noexcept
{
	m_hInstance = GetModuleHandleA(nullptr);

	m_WindowSize.Width = Width;
	m_WindowSize.Height = Height;

	WNDCLASSEXA wc{};
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIconA(NULL, IDI_APPLICATION);
	wc.hCursor = nullptr; //LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = nullptr; //(HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "JWEngine";
	wc.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

	if (!RegisterClassExA(&wc))
	{
		JWAbort("RegisterClassExA() failed.");
	}

	m_hWnd = CreateWindowExA(NULL, wc.lpszClassName, Title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			Width, Height, nullptr, nullptr, m_hInstance, nullptr);

	if (!m_hWnd)
	{
		JWAbort("CreateWindowExA() failed.");
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
}

auto JWWin32Window::GetWidth() const noexcept->int
{
	return m_WindowSize.Width;
}

auto JWWin32Window::GetHeight() const noexcept->int
{
	return m_WindowSize.Height;
}

auto JWWin32Window::GethWnd() const noexcept->HWND
{
	return m_hWnd;
}

auto JWWin32Window::GethInstance() const noexcept->HINSTANCE
{
	return m_hInstance;
}