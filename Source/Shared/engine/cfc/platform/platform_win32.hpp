#pragma once

#include <cfc/base.h>
#include <cfc/core/context.h>
#include <cfc/core/logging.h>
#include <cfc/core/io.h>
#include <cfc/core/hashing.h>
#include <cfc/core/random.h>
#include <cfc/core/timing.h>
#include <cfc/core/window.h>

#include <windows.h>
#include <cfc/stl/stl_string.hpp>
#include <cfc/stl/stl_string_advanced.hpp>

CFC_NAMESPACE3(cfc, platform, win32)

class window : public cfc::window
{
public:
	window(cfc::context& context, int width = 1280, int height = 720, cfc::window::fullscreenMode fsMode = cfc::window::fullscreenMode::Windowed, const char* windowTitle = "CfcWindow", const char* windowClassName = "CfcWindowClass") :
		m_context(context)
	{
		// Set Width/height
		m_width = width;
		m_height = height;

		// Set fullscreen mode
		m_fullscreenMode = fsMode;

		// Convert strings to multibyte
		stl_string sclassName(windowClassName, windowClassName + strlen(windowClassName));
		stl_string swindowTitle(windowTitle, windowTitle + strlen(windowTitle));
		stl_string wclassName = stl_string_advanced::utf16_fromUtf8(sclassName); 
		stl_string wwindowTitle= stl_string_advanced::utf16_fromUtf8(swindowTitle);  

		// Add zero terminators
		stl_string_advanced::utf16_push_back(wclassName, 0);
		stl_string_advanced::utf16_push_back(wwindowTitle, 0);

		// Get the instance
		HMODULE hInstance = GetModuleHandle(NULL);

		// Initialize the window class.
		WNDCLASSEXW windowClass = { 0 };
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = hInstance;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.lpszClassName = (wchar_t*)wclassName.c_str();
		RegisterClassExW(&windowClass);

		RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

		// Create the window and store a handle to it.
		m_hwnd = CreateWindowW(
			windowClass.lpszClassName,
			(wchar_t*)wwindowTitle.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,		// We have no parent window.
			nullptr,		// We aren't using menus.
			hInstance,
			this);


		BOOL ret = IsWindowUnicode(m_hwnd);
		if (!ret)
		{
			CFC_BREAKPOINT;
		}

		ShowWindow(m_hwnd, SW_SHOW);
	}
	~window()
	{

	}

	virtual void UpdateTitle(const char* title)
	{
		SetWindowText(m_hwnd, title);
	}

	virtual void Update()
	{
		_updateDeltas();

		// Clear message.
		MSG msg;
		memset(&msg, 0, sizeof(MSG));

		// Process any messages in the queue.
		BOOL res;
		//if(GetMessage(&msg, NULL, 0, 0))
		while ((res=PeekMessageW(&msg, m_hwnd, 0, 0, PM_REMOVE)) == 1)
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);	
		}

	}

	virtual void* GetPlatformSpecificHandle() { return m_hwnd; }

	virtual void SetCursorXY(int x, int y)
	{
		RECT rect = { 0 };
		GetWindowRect(m_hwnd, &rect);

		SetCursorPos(rect.left + x, rect.bottom - y);
	}

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//printf("Processing msg.. %d %d %x\n", message, __threadid(), hWnd);
		//return DefWindowProcW(hWnd, message, wParam, lParam);
		window* pWindow = reinterpret_cast<window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

#		define GET_X_LPARAM(lParam) (lParam & 0xFFFF)
#		define GET_Y_LPARAM(lParam) ((lParam >> 16) & 0xFFFF)

		switch (message)
		{
		case WM_CREATE:
			{
				// Save the user pointer passed in to CreateWindow.
				LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
			}
			break;
		case WM_KEYDOWN:
			if (pWindow)
				pWindow->_doKeyDown(0, static_cast<UINT8>(wParam));
			return 0;
		case WM_KEYUP:
			if (pWindow)
				pWindow->_doKeyUp(0, static_cast<UINT8>(wParam));
			return 0;
		case WM_CHAR:
			if (pWindow)
				pWindow->_doKeyChar(0, (int)wParam);
			return 0;
		case WM_LBUTTONDOWN:
			if (pWindow)
				pWindow->_doCursorDown(0, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), (int)cfc::window::cursorButton::Left);
			return 0;
		case WM_RBUTTONDOWN:
			if (pWindow)
				pWindow->_doCursorDown(0, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), (int)cfc::window::cursorButton::Right);
			return 0;
		case WM_MBUTTONDOWN:
			if (pWindow)
				pWindow->_doCursorDown(0, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), (int)cfc::window::cursorButton::Middle);
			return 0;
		case WM_LBUTTONUP:
			if (pWindow)
				pWindow->_doCursorUp(0, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), (int)cfc::window::cursorButton::Left);
			return 0;			 
		case WM_RBUTTONUP:		 
			if (pWindow)		 
				pWindow->_doCursorUp(0, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), (int)cfc::window::cursorButton::Right);
			return 0;			 
		case WM_MBUTTONUP:		 
			if (pWindow)		 
				pWindow->_doCursorUp(0, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam), (int)cfc::window::cursorButton::Middle);
			return 0;
		case WM_MOUSEMOVE:
			if (pWindow)
				pWindow->_doCursorMove(0, (float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
			return 0;
		case WM_MOUSEWHEEL:
			if (pWindow)
			{
				short val = (short)GET_Y_LPARAM(wParam);
				float scrollDelta = (float)val;
				scrollDelta /= 120.0f;
				pWindow->m_scroll += scrollDelta;
				pWindow->_doCursorScroll(0, pWindow->m_scroll);
			}
			break;
		case WM_SIZE:
			if (pWindow)
				pWindow->_doResize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_PAINT:
			if (pWindow)
				pWindow->_doPaint();
			break;
		case WM_DESTROY:
			if (pWindow)
				pWindow->m_requestStop = true;
			PostQuitMessage(0);
			break;
		}

#		undef GET_X_LPARAM
#		undef GET_Y_LPARAM

		// Handle any messages the switch statement didn't.
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
protected:
	float m_scroll = 0.0f;
	HWND m_hwnd;
	cfc::context& m_context;
};

class io : public cfc::io
{
	virtual u32_64 GetFileSize(const char* path)
	{
		FILE* f = fopen(path, "rb");
		if (!f)
			return 0;
		fseek(f, 0, SEEK_END);
		u32_64 size = ftell(f);
		fclose(f);
		return size;
	}
	virtual bool Exists(const char* path)
	{
		FILE* f=fopen(path, "rb");
		if(f)
		{
			fclose(f);
			return true;
		}
		return false;
	}
	virtual cfc::iobuffer ReadFileToMemory(const char* path)
	{
		cfc::iobuffer outBuffer;
		usize size = GetFileSize(path);
		FILE* f = fopen(path, "rb");
		if (f)
		{
			outBuffer.size = size;
			outBuffer.data = (u8*)malloc(outBuffer.size);
			fread(outBuffer.data, 1, outBuffer.size, f);
			fclose(f);
			return outBuffer;
		}
		return outBuffer;
	}
	virtual bool ShowOpenFileDialog(const char* filter, char* destination, i32 destinationBufferSize) 
	{ 
		OPENFILENAMEA ofn;       // common dialog gpu_box structure
		char szFile[260];       // buffer for file name

								// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = HWND_DESKTOP;
		ofn.lpstrFile = szFile;
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
		// use the contents of szFile to initialize itself.
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		// Display the Open dialog gpu_box. 

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			strncpy(destination, ofn.lpstrFile, destinationBufferSize);
			return true;
		}
		return false;
	}
};

CFC_END_NAMESPACE3(cfc, platform, win32)

