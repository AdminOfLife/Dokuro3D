#include "ResizeDemo.hpp"
#include <cmath>
using namespace dkr;

ResizeDemo::ResizeDemo(HWND hWnd) {
	rotation = 0.0f;
	hwnd = hWnd;
	resized = false;

	minWidth = 500;
	maxWidth = 800;
	minHeight = 500;
	maxHeight = 800;
}

// Initialization and unloading
void ResizeDemo::Init(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	cube.Load("Resources/Cube.md2");
	cubeId = _memory->AddBuffer(cube.GetFrame());
	_memory->GetBuffer(cubeId).SetDrawMode(VertexBuffer::DRAW_SOLID);
	_memory->GetBuffer(cubeId).SetShading(VertexBuffer::SHADE_FLAT);
	_memory->GetBuffer(cubeId).SetTextureMode(VertexBuffer::TEXTURE_BASIC);
	_memory->GetBuffer(cubeId).SetFaceCulling(VertexBuffer::CULL_BACK);
	_memory->GetBuffer(cubeId).SetSpecular(Vector4(0.0f));

	// Set lights
	_transformer->GetLightInfo()->SetAmbient(AmbientLight(Vector4(0.2f)));
	_transformer->GetLightInfo()->AddDirectionalLight(DirectionalLight(Vector4(1.0f, 1.0f, 0.0f, 0.0f), Vector4(1.0f)));
}
void ResizeDemo::Unload(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_memory->RemoveBuffer(cubeId);
	_transformer->GetLightInfo()->Clear();

	// Reset the size
	SetWindowPos(hwnd, NULL, 0, 0, (int)oldWidth, (int)oldHeight, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_SHOWWINDOW);
}

// Update and draw the demo.
void ResizeDemo::Update(float time, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	rotation += time * 1.0f;

	if (!resized) {
		oldWidth = _renderer->GetWidth();
		oldHeight = _renderer->GetHeight();
		resized = true;
	}

	// Reduce and increase the width and height
	width = minWidth + ((1.0f + sinf(rotation))/2) * (maxWidth - minWidth);
	height = minHeight + ((1.0f + cosf(rotation))/2) * (maxHeight - minHeight);

	// Reduce and increase the size of the window
	SetWindowPos(hwnd, NULL, 0, 0, (int)width, (int)height, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_SHOWWINDOW);
}
void ResizeDemo::Draw(HWND hWnd, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_transformer->GetTransform().Push(Matrix4::CreateScale(2.0f, 2.0f, 2.0f));
		_transformer->GetTransform().Push(Matrix4::CreateRotation(rotation / 3, rotation, rotation / 2));
			_transformer->GetTransform().Push(Matrix4::CreateTranslation(0, 0.0f, -10.0f));
				_transformer->Transform(cubeId);
				_renderer->DrawSolidTriangles(cubeId);
			_transformer->GetTransform().Pop();
		_transformer->GetTransform().Pop();
	_transformer->GetTransform().Pop();

	// Draw some text
	_renderer->PrintText(std::string("Currently Showing:\n") + 
					  "Custom filled polygon routine. (Using half space functions).\n" +
					  "Perspective correct texturing\n" +
					  "Resizeable window without stretching", 0, 0, dkr::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

// Get the how long it takes to expire the demo
float ResizeDemo::Expires() {
	return 10.0f;
}