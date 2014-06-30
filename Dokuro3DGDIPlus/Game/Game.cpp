#include "Game.hpp"
#include <Windows.h>
using namespace dkr;

#include <GraphicsMemory/VertexBuffer.hpp>
#include <Transformer/Transformer.hpp>
#include <Maths/Matrix4.hpp>
#include <Models/ModelMD2.hpp>
#include <Camera/Camera.hpp>
#include <Lights/LightInfo.hpp>
#include <Renderer/Texture2D.hpp>

#include <Threads/ThreadFactory.hpp>

#include "../Input/Input.hpp"
#include "../Camera/FPSCamera.hpp"
#include "../Timer.hpp"

Game::Game(HWND _hWnd, int _width, int _height, int _depth)
	: renderer(_hWnd), thread(NULL), running(true) {
	/* Set the window handle */
	hWnd = _hWnd;

	/* Resize the renderer */
	renderer.Create(_width, _height, _depth);

	/* Create the thread */
	thread = ThreadFactory::GetThread(this);
}

Game::~Game() {
	running = false;
	while (thread->Running()) {}
	delete thread;
}

void Game::Resize() {
	// Get the client bounds
	RECT rect;
	GetClientRect(hWnd, &rect);

	// Recreate the perspective matrix
	if (rect.bottom == 0)
		rect.bottom = 1;

	if (rect.right == renderer.GetWidth() && rect.bottom == renderer.GetHeight() && rect.right != 0 && rect.left != 0)
		return;

	renderer.Create(rect.right, rect.bottom, renderer.GetDepth());

	transformer.GetProjection().Pop();
	transformer.GetProjection().Push(Matrix4::CreatePerspective(1.0f, (rect.right)/(float)(rect.bottom), 0.1f, 1000.0f));
}

int Game::Run() {
	Color color((unsigned char)200, 100, 0, 0);

	/* Create a timer */
	Timer timer;

	/* Current ticks */
	long int startTicks = GetTickCount();
	long int ticks = 0;
	long int frames = 0;
	WCHAR fpsString[10];

	/* Create some lights */
	LightInfo lightInfo;
	lightInfo.SetAmbient(AmbientLight(Vector4(0.5f, 0.5f, 0.5f, 0.0f)));
	lightInfo.AddDirectionalLight(DirectionalLight(Vector4(1, 1, 0, 0), Vector4(1.0f, 1.0f, 1.0f, 0.0f)));
	//lightInfo.AddPointLight(PointLight(Vector4(0, 0, 0, 0), Vector4(0.0f, 1.0f, 0.0f, 0.0f), 0.0f, 0.1f, 0.0f));
	//lightInfo.AddSpotLight(SpotLight(Vector4(0, 0, 0, 0), Vector4(1.0f, 1.0f, 1.0f, 1.0f), Vector4(1.0f, 0.0f, 0.0f, 0.0f), 0.1f, 0.1f, 0.1f, 0.1f, 0.0f));
	transformer.SetLightInfo(&lightInfo);
	
	/* Set the memory on the transformer and renderer */
	transformer.SetGraphicsMemory(&memory);
	transformer.SetRenderer(&renderer);
	renderer.SetGraphicsMemory(&memory);

	/* A camera */
	FPSCamera camera;
	float rot = 0;
	float red = 0;
	float blue = 0;
	float green = 0;

	/* Try loading a model */
	ModelMD2 model("miku.md2");
	//ModelMD2 cube("sphere.md2");
	ModelMD2 animatedTest("CREAM.md2");

	// Set up an animation
	ModelMD2::Animation testAnimation;
	/*testAnimation.AddFrame("frame000");
	testAnimation.AddFrame("frame001");
	testAnimation.AddFrame("frame002");
	*/testAnimation.AddFrame("frame003");
	testAnimation.AddFrame("frame004");
	testAnimation.AddFrame("frame005");
	testAnimation.AddFrame("frame006");
	testAnimation.AddFrame("frame007");
	testAnimation.AddFrame("frame008");
	testAnimation.AddFrame("frame009");
	testAnimation.AddFrame("frame010");/*
	testAnimation.AddFrame("frame011");
	testAnimation.AddFrame("frame012");
	testAnimation.AddFrame("frame013");
	testAnimation.AddFrame("frame014");
	testAnimation.AddFrame("frame015");
	testAnimation.AddFrame("frame016");
	testAnimation.AddFrame("frame017");
	testAnimation.AddFrame("frame018");
	testAnimation.AddFrame("frame019");
	/*testAnimation.AddFrame("frame020");
	testAnimation.AddFrame("frame021");
	testAnimation.AddFrame("frame022");
	testAnimation.AddFrame("frame023");
	testAnimation.AddFrame("frame024");
	testAnimation.AddFrame("frame025");
	testAnimation.AddFrame("frame026");
	testAnimation.AddFrame("frame027");
	testAnimation.AddFrame("frame028");
	testAnimation.AddFrame("frame029");
	testAnimation.AddFrame("frame030");
	testAnimation.AddFrame("frame031");
	testAnimation.AddFrame("frame032");
	testAnimation.AddFrame("frame033");
	testAnimation.AddFrame("frame034");
	testAnimation.AddFrame("frame035");	
	testAnimation.AddFrame("frame036");
	testAnimation.AddFrame("frame037");*/

	animatedTest.SetAnimation(testAnimation);

	// Create a test renderer.
	int bufferId = memory.AddBuffer(model.GetFrame());
	//int cubeId = memory.AddBuffer(cube.GetFrame());
	int animatedId = memory.AddBuffer(animatedTest.GetAnimatedFrame());

	while (running) {
		/* Get frame rate */
		ticks = GetTickCount() - startTicks;
		if (ticks >= 1000) {
			wsprintf(fpsString, L"%i", frames);
			SetWindowText(hWnd, fpsString);
			frames = 0;
			startTicks = GetTickCount();
		}

		renderer.Clear(Color((unsigned char)255, 0, 50, 255));

		// Move the light to the camera
		if (Input::KeyPressed('G')) {
			//lightInfo.GetSpotLights()[0].SetPosition(camera.GetCamera().GetPosition());
			//lightInfo.GetSpotLights()[0].SetDirection(camera.GetCamera().GetForwardVector());
		}

		/* Draw miku */
		transformer.GetCamera().Push(camera.GetMatrix());

		transformer.GetTransform().Push(Matrix4::CreateRotation(0, rot, 0));
			transformer.GetTransform().Push(Matrix4::CreateTranslation(0, 0, 0.0f));
				transformer.Transform(bufferId);
				renderer.DrawSolidTriangles(bufferId);
			transformer.GetTransform().Pop();
		transformer.GetTransform().Pop();

		// Get the animated model
		memory.RecreateBuffer(animatedId, animatedTest.GetAnimatedFrame());
		
		transformer.GetTransform().Push(Matrix4::CreateRotation(0, 0, 0));
			transformer.GetTransform().Push(Matrix4::CreateTranslation(0, 0, -100.0f));
				transformer.Transform(animatedId);
				renderer.DrawSolidTriangles(animatedId);
			transformer.GetTransform().Pop();
		transformer.GetTransform().Pop();

		/* Draw cube */
		transformer.GetTransform().Push(Matrix4::CreateScale(1.0f, 1.0f, 1.0f));
			transformer.GetTransform().Push(Matrix4::CreateRotation(0, 0, 0));
				transformer.GetTransform().Push(Matrix4::CreateTranslation(0, 0, 0.0f));
					//transformer.Transform(cubeId);
					//renderer.DrawSolidTriangles(cubeId);
				transformer.GetTransform().Pop();
			transformer.GetTransform().Pop();
		transformer.GetTransform().Pop();

		transformer.GetCamera().Pop();

		renderer.Flip();
		Resize();

		camera.Update((float)timer.GetTime(), renderer.GetWidth(), renderer.GetHeight(), hWnd);
		Input::Update(hWnd);
		rot += 0.01f;
		red += 0.0001f;
		blue += 0.001f;
		green += -0.002f;

		if (red >= 1.0f)
			red -= 1.0f;
		if (blue >= 1.0f)
			blue -= 1.0f;
		if (red >= 1.0f)
			red -= 1.0f;

		// Update the animation
		animatedTest.Update(0.1f);

		// Update timer
		timer.Tick();

		/* Increment frames */
		++frames;
	}

	return 0;
}