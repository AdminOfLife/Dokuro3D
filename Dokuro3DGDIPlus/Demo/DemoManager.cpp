#include "DemoManager.hpp"

DemoManager::DemoManager(HWND _hWnd, int _width, int _height, int _depth)
	: Game(_hWnd, _width, _height, _depth) {
	currentDemo = 0;
	timePassed = 0;

	// Add demos
	AddDemo(new WireframeDemo());
	AddDemo(new SolidDemo());
	AddDemo(new SolidLitDemo());
	AddDemo(new PaintersSortDemo());
	AddDemo(new SpecularModelDemo());
	AddDemo(new SpecularModelSmoothDemo());
	AddDemo(new ZbufferDemo());
	AddDemo(new SpecularModelPhongDemo());
	AddDemo(new PointLightDemo());
	AddDemo(new TexturePhongDemo());
	AddDemo(new AnimatedDemo());
	AddDemo(new ResizeDemo(_hWnd));
	AddDemo(new CameraDemo());
	AddDemo(new CameraDemo2());
	AddDemo(new ScaleRotateTranslateDemo());

	// Start the thread
	thread->Start();
}

DemoManager::~DemoManager() {
	for (unsigned int i = 0; i < demos.size(); ++i) {
		delete demos[i];
	}
}

// Add a demo to the manager.
void DemoManager::AddDemo(Demo* _demo) {
	demos.push_back(_demo);
}

// Run the manager
int DemoManager::Run() {
	// Set up the transformer and renderer.
	transformer.SetGraphicsMemory(&memory);
	transformer.SetRenderer(&renderer);
	renderer.SetGraphicsMemory(&memory);

	// Set up some variables so that we can accurately determine the frame rate.
	long int startTicks = GetTickCount();
	long int ticks = 0;
	long int frames = 0;
	WCHAR fpsString[10];

	// Create a light info
	transformer.SetLightInfo(&lightInfo);

	// Init the first demo
	if (demos.size() > 0)
		demos[0]->Init(&memory, &transformer, &renderer);

	// Create a timer
	Timer timer;

	while (true) {
		// Calculate the frame rate
		ticks = GetTickCount() - startTicks;
		if (ticks >= 1000) {
			wsprintf(fpsString, L"%i", frames);
			SetWindowText(hWnd, fpsString);
			frames = 0;
			startTicks = GetTickCount();
		}

		// Calculate the time the last frame took.
		float time = (float)timer.GetTime();

		// Clear the screen
		renderer.Clear(dkr::Color(0.0f, 0.5f, 1.0f, 0.0f));

		// If there is no demo then don't do any of this stuff.
		if (demos.size() > 0 || currentDemo < (int)demos.size()) {

			// Switch demos if necessary
			if (timePassed >= demos[currentDemo]->Expires() || Input::KeyPressed(VK_RIGHT)) {
				// Unload the current demo
				demos[currentDemo]->Unload(&memory, &transformer, &renderer);

				// Load the next demo
				currentDemo = (currentDemo + 1) % demos.size();
				demos[currentDemo]->Init(&memory, &transformer, &renderer);

				// Reset the time
				timePassed = 0.0f;
			}

			// Update and draw the demo
			demos[currentDemo]->Update(time, &memory, &transformer, &renderer);
			demos[currentDemo]->Draw(hWnd, &memory, &transformer, &renderer);
		
		}

		// Resize the window if we need to
		Resize();

		// Flip the back buffer
		renderer.Flip();

		// Update the input manager
		Input::Update(hWnd);

		// Keep count of how many frames we do each second
		++frames;

		// Increment the passed time
		this->timePassed += time;

		// Tick the timer
		timer.Tick();
	}

	return 0;
}