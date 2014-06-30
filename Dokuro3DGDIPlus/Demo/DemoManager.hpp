#ifndef DEMOMANAGER_DEF
#define DEMOMANAGER_DEF

#include <vector>
#include "../Game/Game.hpp"
#include "../Input/Input.hpp"
#include "../Timer.hpp"

#include "Demo.hpp"
#include "Demos/WireframeDemo/WireframeDemo.hpp"
#include "Demos/SolidDemo/SolidDemo.hpp"
#include "Demos/SolidLitDemo/SolidLitDemo.hpp"
#include "Demos/SpecularModel/SpecularModelDemo.hpp"
#include "Demos/SpecularModelSmooth/SpecularModelSmoothDemo.hpp"
#include "Demos/SpecularModelPhong/SpecularModelPhongDemo.hpp"
#include "Demos/PaintersSortDemo/PaintersSortDemo.hpp"
#include "Demos/ZbufferDemo/ZbufferDemo.hpp"
#include "Demos/PointLightDemo/PointLightDemo.hpp"
#include "Demos/TexturePhongDemo/TexturePhongDemo.hpp"
#include "Demos/AnimatedDemo/AnimatedDemo.hpp"
#include "Demos/ResizeDemo/ResizeDemo.hpp"
#include "Demos/CameraDemo/CameraDemo.hpp"
#include "Demos/CameraDemo2/CameraDemo2.hpp"
#include "Demos/ScaleRotateTranslate/ScaleRotateTranslate.hpp"

class DemoManager : public Game {
public:
	DemoManager(HWND _hWnd, int _width, int _height, int _depth);
	~DemoManager();

	// Add a demo to the manager.
	void AddDemo(Demo* _demo);

	// Run the demo manager.
	virtual int Run();

private:

	// The current demo.
	int currentDemo;

	// The time passed in the current demo.
	float timePassed;

	// A list of managed demos
	std::vector<Demo*> demos;

	// The light info
	LightInfo lightInfo;
};

#endif