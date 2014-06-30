#include "FPSCamera.hpp"

FPSCamera::FPSCamera() {
	lockMouse = false;
}

FPSCamera::~FPSCamera() {
}

void FPSCamera::Update(float _time, int _width, int _height, HWND _hWnd) {
	if (Input::KeyDown('A')) {
		camera.FPSTranslate(-20*_time, 0, 0);
	} 
	if (Input::KeyDown('S')) {
		camera.FPSTranslate(20*_time, 0, 0);
	} 
	if (Input::KeyDown('W')) {
		camera.FPSTranslate(0, 0, -20*_time);
	} 
	if (Input::KeyDown('R')) {
		camera.FPSTranslate(0, 0, 20*_time);
	} 
	if (Input::KeyDown(VK_SPACE)) {
		camera.FPSTranslate(0, 20*_time, 0);
	}
	if (Input::KeyDown(VK_SHIFT)) {
		camera.FPSTranslate(0, -20*_time, 0);
	}

	// If the there is a right click, then lock the mouse
	if (Input::KeyPressed(VK_RBUTTON)) {
		lockMouse = !lockMouse;
		Input::ResetMouse(_width/2, _height/2, _hWnd);
	} else {
		// Now the mouse
		// Get the difference from the center
		int diffX = Input::GetMouseX() - (_width/2);
		int diffY = Input::GetMouseY() - (_height/2);

		if ((diffY != 0 || diffX != 0) && lockMouse) {
			camera.FPSRotate(-diffX*0.02f, 0);
			camera.FPSRotate(0, -diffY*0.02f);
			Input::ResetMouse(_width/2, _height/2, _hWnd);
		}
	}
}

dkr::Matrix4 FPSCamera::GetMatrix() {
	return camera.GetMatrix();
}

const dkr::Camera& FPSCamera::GetCamera() const {
	return camera;
}