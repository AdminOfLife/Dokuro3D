#include "SolidDemo.hpp"
using namespace dkr;

SolidDemo::SolidDemo() {
	rotation = 0.0f;
}

// Initialization and unloading
void SolidDemo::Init(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	cube.Load("Resources/Cube.md2");
	cubeId = _memory->AddBuffer(cube.GetFrame());
	_memory->GetBuffer(cubeId).SetDrawMode(VertexBuffer::DRAW_SOLID);
	_memory->GetBuffer(cubeId).SetShading(VertexBuffer::SHADE_FLAT);
	_memory->GetBuffer(cubeId).SetTextureMode(VertexBuffer::TEXTURE_NONE);
	_memory->GetBuffer(cubeId).SetFaceCulling(VertexBuffer::CULL_BACK);

	// Set lights
	_transformer->GetLightInfo()->SetAmbient(AmbientLight(Vector4(0.3f)));
}
void SolidDemo::Unload(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_memory->RemoveBuffer(cubeId);
}

// Update and draw the demo.
void SolidDemo::Update(float time, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	rotation += time * 1.0f;
}
void SolidDemo::Draw(HWND hWnd, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_transformer->GetTransform().Push(Matrix4::CreateScale(1.0f, 1.0f, 1.0f));
		_transformer->GetTransform().Push(Matrix4::CreateRotation(rotation / 2, rotation, rotation / 3));
			_transformer->GetTransform().Push(Matrix4::CreateTranslation(0, 0, -10.0f));
				_transformer->Transform(cubeId);
				_renderer->DrawSolidTriangles(cubeId);
			_transformer->GetTransform().Pop();
		_transformer->GetTransform().Pop();
	_transformer->GetTransform().Pop();

	// Draw some text
	_renderer->PrintText(std::string("Currently Showing:\n") +
					  "Custom filled polygon routine. (Using half space functions).\n" +
					  "Ambient lighting\n", 
					  0, 0, dkr::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

// Get the how long it takes to expire the demo
float SolidDemo::Expires() {
	return 10.0f;
}