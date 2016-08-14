#pragma once

#include <cfc/base.h>
#include <cfc/stl/stl_node.hpp>
#include <cfc/stl/stl_lambda1.hpp>
#include <cfc/stl/stl_array.hpp>

CFC_NAMESPACE1(cfc)

class CFC_API window : public cfc::object
{
public:
	window();
	virtual ~window();

	enum class cursorButton
	{
		Unknown = 0,
		Left = 1,
		Right = 2,
		Middle = 3,
		Forward=4,
		Backwards=5,
		Finger = 1,
		ScrollDown=16,
		ScrollUp=17
	};

	enum class fullscreenMode
	{
		Unknown = 0,
		Windowed,
		BorderlessFullscreen,
		ExclusiveFullscreen
	};

	struct eventData
	{
		int index = 0;
		union
		{
			float x;
			int keyCode = 0;
			float scroll;
		};
		float y = 0.0f;
		cursorButton button=cursorButton::Unknown;
	};

	typedef stl_node<stl_lambda1<const eventData&> > event;

	bool IsRequestingStop() const							{ return m_requestStop;  }
	bool IsKeyboardPresent() const							{ return m_keyboardPresent; }
	bool IsMousePresent() const								{ return m_mousePresent; }
	bool IsTouchDevice() const								{ return m_touchDevice; }
	bool IsCapableOfMultitouch() const						{ return m_multiTouchDevice; }

	int GetWidth() const									{ return m_width; }
	int GetHeight() const									{ return m_height; }
	float GetWidthAsFloat() const							{ return (float)m_width; }
	float GetHeightAsFloat() const							{ return (float)m_height; }
	fullscreenMode GetFullscreenMode() const				{ return m_fullscreenMode; }	

	float GetCursorX(int cursorID=0)						{ return m_cursorX[cursorID]; }
	float GetCursorY(int cursorID=0)						{ return m_cursorY[cursorID]; }
	float GetCursorScroll(int cursorID = 0)					{ return m_cursorScroll[cursorID]; }
	float GetCursorDeltaX(int cursorID = 0)					{ return m_cursorX[cursorID] - m_cursorLX[cursorID]; }
	float GetCursorDeltaY(int cursorID = 0)					{ return m_cursorY[cursorID] - m_cursorLY[cursorID]; }
	float GetCursorDeltaScroll(int cursorID = 0)			{ return m_cursorScroll[cursorID] - m_cursorLScroll[cursorID]; }

	bool IsCursorDown(int cursorButton, int cursorID = 0)	{ return m_cursorDown[cursorID][cursorButton]; }
	bool IsKeyDown(int keyID)								{ for (int i = 0; i < m_numKeysDown; i++) {if (m_keysDown[i] == keyID) return true; } return false;	}
	int GetKeyDownByIndex(int index)						{ return m_keysDown[index]; }
	int GetNumberOfKeysDown()								{ return m_numKeysDown; }

	virtual void SetCursorXY(int x, int y)					{}

	virtual void UpdateTitle(const char* title)				{}

	virtual void Update()									{}
	virtual void* GetPlatformSpecificHandle()				{ return nullptr;  }

	event OnCursorMove;
	event OnCursorDown;
	event OnCursorUp;
	event OnCursorScroll;
	event OnKeyDown;
	event OnKeyUp;
	event OnKeyChar;
	event OnResize;
	event OnPaint;

protected:
	void _updateDeltas();
	void _doKeyDown(int index, int keyCode);
	void _doKeyUp(int index, int keyCode);
	void _doKeyChar(int index, int keyCharCode);
	void _doCursorDown(int index, float x, float y, int buttonIndex);
	void _doCursorUp(int index, float x, float y, int buttonIndex);
	void _doCursorMove(int index, float x, float y);
	void _doCursorScroll(int index, float scroll);
	void _doResize(int x, int y);
	void _doPaint();

	fullscreenMode m_fullscreenMode;
	int m_width;
	int m_height;
	bool m_touchDevice, m_multiTouchDevice, m_keyboardPresent, m_mousePresent, m_requestStop;
private:
	void _removeKeyDown(int keyID);
	void _removeKeyDownIndex(int keyID);
	void _addKeyDown(int keyID);

	stl_array<float, 16> m_cursorX;
	stl_array<float, 16> m_cursorY;
	stl_array<float, 16> m_cursorScroll;
	stl_array<float, 16> m_cursorLX; // Last mouse X on frame
	stl_array<float, 16> m_cursorLY; // Last mouse Y on frame
	stl_array<float, 16> m_cursorLScroll; // Last scroll on frame
	bool m_cursorDown[16][8];
	stl_array<int, 32> m_keysDown;
	int m_numKeysDown;
};

CFC_END_NAMESPACE1(cfc)