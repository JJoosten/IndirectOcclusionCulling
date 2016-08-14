#include <cfc/base.h>

#include <string.h>
#include <stdio.h>

#include "window.h"


cfc::window::window()
{
	memset(&m_cursorScroll[0], 0, m_cursorScroll.size() * sizeof(float));
	memset(&m_cursorX[0], 0, m_cursorX.size() * sizeof(float));
	memset(&m_cursorY[0], 0, m_cursorY.size() * sizeof(float));
	memset(m_cursorDown, 0, sizeof(m_cursorDown));
	m_touchDevice = false;
	m_multiTouchDevice = false;
	m_keyboardPresent = true;
	m_mousePresent = true;
	m_requestStop = false;
	m_fullscreenMode = fullscreenMode::Unknown;
	m_numKeysDown = 0;
}

cfc::window::~window()
{
}

void cfc::window::_removeKeyDown(int keyID)
{
	for (int i = 0; i < m_numKeysDown; i++)
	{
		if (m_keysDown[i] == keyID)
		{
			_removeKeyDownIndex(i);
		}
	}
}

void cfc::window::_removeKeyDownIndex(int keyID)
{
	for (int i = keyID; i < m_numKeysDown - 1; i++)
	{
		m_keysDown[i] = m_keysDown[i + 1];
	}
	--m_numKeysDown;
}

void cfc::window::_addKeyDown(int keyID)
{
	if (IsKeyDown(keyID))
		return;
	m_keysDown[m_numKeysDown++] = keyID;
	if (m_numKeysDown >= 32)
		CFC_BREAKPOINT;
}

void cfc::window::_updateDeltas()
{
	for (int id = 0; id < 16; id++)
	{
		m_cursorLScroll[id] = m_cursorScroll[id];
		m_cursorLX[id] = m_cursorX[id];
		m_cursorLY[id] = m_cursorY[id];
	}
}

void cfc::window::_doKeyDown(int index, int keyCode)
{
	if (index == 0)
		_addKeyDown(keyCode);

	eventData ev;
	ev.index = index;
	ev.keyCode = keyCode;
	stl_node_helpers::foreach(OnKeyDown, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doKeyUp(int index, int keyCode)
{
	if (index == 0)
		_removeKeyDown(keyCode);

	eventData ev;
	ev.index = index;
	ev.keyCode = keyCode;
	stl_node_helpers::foreach(OnKeyUp, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doKeyChar(int index, int keyCharCode)
{
	eventData ev;
	ev.index = index;
	ev.keyCode = keyCharCode;
	stl_node_helpers::foreach(OnKeyChar, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doCursorDown(int index, float x, float y, int buttonIndex)
{
	if (index < sizeof(m_cursorX) / sizeof(m_cursorX[0]))
	{
		m_cursorX[index] = x;
		m_cursorY[index] = y;
		if (buttonIndex < 8)
			m_cursorDown[index][buttonIndex] = true;
	}

	eventData ev;
	ev.index = index;
	ev.x = x;
	ev.y = y;
	ev.button = (cfc::window::cursorButton)buttonIndex;
	stl_node_helpers::foreach(OnCursorDown, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doCursorUp(int index, float x, float y, int buttonIndex)
{
	if (index < sizeof(m_cursorX) / sizeof(m_cursorX[0]))
	{
		m_cursorX[index] = x;
		m_cursorY[index] = y;
		if (buttonIndex < 8)
			m_cursorDown[index][buttonIndex] = false;
	}

	eventData ev;
	ev.index = index;
	ev.x = x;
	ev.y = y;
	ev.button = (cfc::window::cursorButton)buttonIndex;
	stl_node_helpers::foreach(OnCursorUp, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doCursorScroll(int index, float scroll)
{
	m_cursorScroll[index] = scroll;

	eventData ev;
	ev.index = index;
	ev.scroll = scroll;
	stl_node_helpers::foreach(OnCursorDown, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doCursorMove(int index, float x, float y)
{
	if (index < sizeof(m_cursorX) / sizeof(m_cursorX[0]))
	{
		m_cursorX[index] = x;
		m_cursorY[index] = y;
	}

	eventData ev;
	ev.index = index;
	ev.x = x;
	ev.y = y;
	stl_node_helpers::foreach(OnCursorMove, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doResize(int x, int y)
{
	m_width = x;
	m_height = y;

	eventData ev;
	ev.x = (float)x;
	ev.y = (float)y;
	stl_node_helpers::foreach(OnResize, [&ev](event& nd) {
		nd.get()(ev);
	});
}

void cfc::window::_doPaint()
{
	eventData ev;

}