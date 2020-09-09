#include "Window.h"
#include <sstream>

//WindowClass
Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

const char* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}


// Window
Window::Window(int width, int height, const char* name)
{
	// calculate window size based on desired window size
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
	{
		throw WND_LAST_EXCEPT();
	}
	// Create window and get handle
	hWnd = CreateWindow(
		WindowClass::GetName(),
		name,
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		nullptr,
		nullptr,
		WindowClass::GetInstance(), this
	);
	// Check for error
	if (hWnd == nullptr)
	{
		throw WND_LAST_EXCEPT();
	}
	// show window
	ShowWindow(hWnd, SW_SHOWDEFAULT);
}

Window::~Window()
{
	DestroyWindow(hWnd);
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// Use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
	if (msg == WM_NCCREATE)
	{
		// Extract ptr to window class from creation data
		const CREATESTRUCT* const pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		Window* const pWnd = reinterpret_cast<Window*>(pCreate->lpCreateParams); // TODO: static or reinterpret
		// Set WinAPI-managed user data to store a pointer to Window class
		SetWindowLongPtr(hWnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// Set message proc to non-setup handler now that the set up is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgAdapter));
		// Forward message to indoe class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	// If we get a message before WM_NCCREATE, handle with default
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsgAdapter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// Retrieve ptr to Window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWL_USERDATA));
	// Forward message to Window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(42);
		return 0;
	// When window loses focus clear the keystate to prevent zombie input
	case WM_KILLFOCUS:
		kbd.ClearState();
		break;
	/******** KEYBOARD *********/
	case WM_KEYDOWN:
	// Handle system key commands
	case WM_SYSKEYDOWN:
		if (!(lParam & 0x40000000) || kbd.AutorepeatIsEnabled())
		{
			kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_CHAR:
		kbd.OnChar(static_cast<unsigned char>(wParam));
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// WindowException
Window::WindowException::WindowException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{ }

const char* Window::WindowException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code]: " << GetErrorCode() << std::endl
		<< "[Description]: " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::WindowException::GetType() const noexcept
{
	return "Window Exception";
}

std::string Window::WindowException::TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuff = nullptr;
	DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuff),
		0,
		nullptr
		);
	if (nMsgLen == 0)
	{
		return "Unidentified.";
	}
	std::string errorString = pMsgBuff;
	LocalFree(pMsgBuff);
	return errorString;
}

HRESULT Window::WindowException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::WindowException::GetErrorDescription() const noexcept
{
	return TranslateErrorCode(hr);
}
