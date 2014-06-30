#include "PointLightDemo.hpp"
using namespace dkr;

PointLightDemo::PointLightDemo() {
	rotation = 0.0f;
}

// Initialization and unloading
void PointLightDemo::Init(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	cube.Load("Resources/PLAY.md2");
	cubeId = _memory->AddBuffer(cube.GetFrame());
	_memory->GetBuffer(cubeId).SetDrawMode(VertexBuffer::DRAW_SOLID);
	_memory->GetBuffer(cubeId).SetShading(VertexBuffer::SHADE_PHONG);
	_memory->GetBuffer(cubeId).SetTextureMode(VertexBuffer::TEXTURE_NONE);
	_memory->GetBuffer(cubeId).SetFaceCulling(VertexBuffer::CULL_BACK);

	// Set lights
	_transformer->GetLightInfo()->SetAmbient(AmbientLight(Vector4(0.f)));
	_transformer->GetLightInfo()->AddDirectionalLight(DirectionalLight(Vector4(1.0f, 1.0f, 0.0f, 0.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f)));
	_transformer->GetLightInfo()->AddPointLight(PointLight(Vector4(0, -4.0f, -8.0f, 0.0f), Vector4(1.0f, 0.0f, 0.0f, 0.0f), 0.1f, 0.0f, 0.2f));
}
void PointLightDemo::Unload(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_memory->RemoveBuffer(cubeId);
	_transformer->GetLightInfo()->Clear();
}

// Update and draw the demo.
void PointLightDemo::Update(float time, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	rotation += time * 1.0f;
}
void PointLightDemo::Draw(HWND hWnd, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_transformer->GetTransform().Push(Matrix4::CreateScale(0.1f, 0.1f, 0.1f));
		_transformer->GetTransform().Push(Matrix4::CreateRotation(0, rotation, 0));
			_transformer->GetTransform().Push(Matrix4::CreateTranslation(0, -4.0f, -10.0f));
				_transformer->Transform(cubeId);
				_renderer->DrawSolidTriangles(cubeId);
			_transformer->GetTransform().Pop();
		_transformer->GetTransform().Pop();
	_transformer->GetTransform().Pop();

	// Draw some text
	_renderer->PrintText(std::string("Currently Showing:\n") + 
					  "Custom filled polygon routine. (Using half space functions).\n" +
					  "Ambient light\n" +
					  "Diffuse light\n" +
					  "Specular light\n" +
					  "Blinn Phong shading\n" +
					  "Directional light\n" +
					  "Point light", 0, 0, dkr::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

// Get the how long it takes to expire the demo
float PointLightDemo::Expires() {
	return 20.0f;
}