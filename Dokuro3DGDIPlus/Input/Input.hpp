#ifndef INPUT_DEF
#define INPUT_DEF

#include <Windows.h>

class Input {
public:
	// Whether or not a key is down.
	static bool KeyDown(int _code);

	// A key has just been pressed
	static bool KeyPressed(int _code);

	// A key has just been unpressed
	static bool KeyReleased(int _code);

	// The mouse position
	static int GetMouseX();
	static int GetMouseY();

	// Get the changes
	static int GetMouseDeltaX();
	static int GetMouseDeltaY();

	// Ignore any changes in the mouse position
	static void IgnoreMouseChange(bool _value);
	static bool GetIgnoreMouseChange();	
	
	// Reset the mouse coords
	static void ResetMouse(int _x, int _y, HWND _hWnd);

	// Update the input
	static void Update(HWND _hWnd);

private:
	static bool mouseIgnore;

	static POINT oldMouse;
	static POINT mouse;
};

#endif