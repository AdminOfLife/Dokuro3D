#include "ScaleRotateTranslate.hpp"
using namespace dkr;

ScaleRotateTranslateDemo::ScaleRotateTranslateDemo() {
	rotation = 0.0f;
}

// Initialization and unloading
void ScaleRotateTranslateDemo::Init(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
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
void ScaleRotateTranslateDemo::Unload(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_memory->RemoveBuffer(cubeId);
	_transformer->GetLightInfo()->Clear();
}

// Update and draw the demo.
void ScaleRotateTranslateDemo::Update(float time, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	rotation += time * 1.0f;
	translation.x = cosf(rotation) * 10.0f;
	translation.y = cosf(rotation / 5) * 10.0f;
	translation.z = sinf(rotation) * 10.0f;
	scale.x = 1.0f + sinf(rotation / 2);
	scale.y = 1.0f + cosf(rotation / 3);
	scale.z = 1.0f + sinf(rotation);
}
void ScaleRotateTranslateDemo::Draw(HWND hWnd, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_transformer->GetTransform().Push(Matrix4::CreateScale(scale.x, scale.y, scale.z) * Matrix4::CreateScale(3.0f, 3.0f, 3.0f));
		_transformer->GetTransform().Push(Matrix4::CreateRotation(rotation / 3, rotation, rotation / 2));
			_transformer->GetTransform().Push(Matrix4::CreateTranslation(translation.x, translation.y, translation.z) * Matrix4::CreateTranslation(0, 0, -30));
				_transformer->Transform(cubeId);
				_renderer->DrawSolidTriangles(cubeId);
			_transformer->GetTransform().Pop();
		_transformer->GetTransform().Pop();
	_transformer->GetTransform().Pop();

	// Draw some text
	_renderer->PrintText(std::string("Currently Showing:\n") + 
					  "Custom filled polygon routine. (Using half space functions).\n" +
					  "Perspective correct texturing\n" +
					  "Model transformation in world space", 0, 0, dkr::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

// Get the how long it takes to expire the demo
float ScaleRotateTranslateDemo::Expires() {
	return 10.0f;
}