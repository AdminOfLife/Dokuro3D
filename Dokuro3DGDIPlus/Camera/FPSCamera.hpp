#ifndef FPSCAMERA_DEF
#define FPSCAMERA_DEF

#include <Camera/Camera.hpp>
#include <Windows.h>
#include "../Input/Input.hpp"

class FPSCamera {
public:
	FPSCamera();
	~FPSCamera();

	/* Update the camera */
	void Update(float _time, int _width, int _height, HWND _hWnd);

	/* Get the matrix */
	dkr::Matrix4 GetMatrix();

	/* Get the camera */
	const dkr::Camera& GetCamera() const;

private:
	dkr::Camera camera;
	bool lockMouse;
};

#endif