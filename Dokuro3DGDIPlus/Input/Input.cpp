#include "Input.hpp"

bool Input::mouseIgnore = false;
POINT Input::oldMouse;
POINT Input::mouse;

bool Input::KeyDown(int _code) {
	return (GetAsyncKeyState(_code) & 0x8000) == 0x8000;
}

bool Input::KeyPressed(int _code) {
	return (GetAsyncKeyState(_code) & 0x0001) == 0x0001;
}

bool Input::KeyReleased(int _code) {
	return false;
}

int Input::GetMouseX() {
	return mouse.x;
}

int Input::GetMouseY() {
	return mouse.y;
}

int Input::GetMouseDeltaX() {
	return mouse.x - oldMouse.x;
}

int Input::GetMouseDeltaY() {
	return mouse.y - oldMouse.y;
}

void Input::IgnoreMouseChange(bool _value) {
	mouseIgnore = _value;
}

bool Input::GetIgnoreMouseChange() {
	return mouseIgnore;
}

void Input::ResetMouse(int _x, int _y, HWND _hWnd) {
	mouse.x = _x;
	mouse.y = _y;

	ClientToScreen(_hWnd, &mouse);
	oldMouse = mouse;

	SetCursorPos(mouse.x, mouse.y);
}

void Input::Update(HWND _hWnd) {
	if (!mouseIgnore) {
		oldMouse = mouse;

		GetCursorPos(&mouse);
		ScreenToClient(_hWnd, &mouse);
	}
}