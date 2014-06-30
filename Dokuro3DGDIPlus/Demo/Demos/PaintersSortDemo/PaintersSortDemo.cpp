#include "PaintersSortDemo.hpp"
using namespace dkr;

PaintersSortDemo::PaintersSortDemo() {
	rotation = 0.0f;
}

// Initialization and unloading
void PaintersSortDemo::Init(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	cube.Load("Resources/Cream.md2");
	cubeId = _memory->AddBuffer(cube.GetFrame());
	_memory->GetBuffer(cubeId).SetDrawMode(VertexBuffer::DRAW_SOLID);
	_memory->GetBuffer(cubeId).SetShading(VertexBuffer::SHADE_FLAT);
	_memory->GetBuffer(cubeId).SetTextureMode(VertexBuffer::TEXTURE_NONE);
	_memory->GetBuffer(cubeId).SetFaceCulling(VertexBuffer::CULL_BACK);
	_memory->GetBuffer(cubeId).SetSpecular(Vector4(0.0f, 0.0f, 0.0f, 0.0f));

	other.Load("Resources/PLAY.md2");
	otherId = _memory->AddBuffer(other.GetFrame());
	_memory->GetBuffer(otherId).SetDrawMode(VertexBuffer::DRAW_SOLID);
	_memory->GetBuffer(otherId).SetShading(VertexBuffer::SHADE_FLAT);
	_memory->GetBuffer(otherId).SetTextureMode(VertexBuffer::TEXTURE_NONE);
	_memory->GetBuffer(otherId).SetFaceCulling(VertexBuffer::CULL_BACK);
	_memory->GetBuffer(otherId).SetSpecular(Vector4(0.0f, 0.0f, 0.0f, 0.0f));

	// Set lights
	_transformer->GetLightInfo()->SetAmbient(AmbientLight(Vector4(0.3f)));
	_transformer->GetLightInfo()->AddDirectionalLight(DirectionalLight(Vector4(1.0f, 1.0f, 0.0f, 0.0f), Vector4(0.9f)));
}
void PaintersSortDemo::Unload(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_memory->RemoveBuffer(cubeId);
	_memory->RemoveBuffer(otherId);
	_transformer->GetLightInfo()->Clear();
}

// Update and draw the demo.
void PaintersSortDemo::Update(float time, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	rotation += time * 1.0f;
}
void PaintersSortDemo::Draw(HWND hWnd, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_transformer->GetTransform().Push(Matrix4::CreateScale(0.1f, 0.1f, 0.1f));
		_transformer->GetTransform().Push(Matrix4::CreateTranslation(0.0f, -2.0f, -4.0f));
			_transformer->GetTransform().Push(Matrix4::CreateRotation(0, rotation, 0));
				_transformer->GetTransform().Push(Matrix4::CreateTranslation(0, 0, -10.0f));
					_transformer->Transform(cubeId);
					_renderer->DrawSolidTriangles(cubeId);
				_transformer->GetTransform().Pop();
			_transformer->GetTransform().Pop();
		_transformer->GetTransform().Pop();
	_transformer->GetTransform().Pop();

	_transformer->GetTransform().Push(Matrix4::CreateScale(0.1f, 0.1f, 0.1f));
		_transformer->GetTransform().Push(Matrix4::CreateTranslation(0.0f, -2.0f, -4.0f));
			_transformer->GetTransform().Push(Matrix4::CreateRotation(0, rotation + 3.141f, 0));
				_transformer->GetTransform().Push(Matrix4::CreateTranslation(0, 0, -10.0f));
					_transformer->Transform(otherId);
					_renderer->DrawSolidTriangles(otherId);
				_transformer->GetTransform().Pop();
			_transformer->GetTransform().Pop();
		_transformer->GetTransform().Pop();
	_transformer->GetTransform().Pop();

	// Draw some text
	_renderer->PrintText(std::string("Currently Showing:\n") + 
					  "Custom filled polygon routine. (Using half space functions).\n" +
					  "Ambient light\n" +
					  "Diffuse light\n" +
					  "Flat Shading\n" +
					  "Painters sort\n" +
					  "Directional light", 0, 0, dkr::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

// Get the how long it takes to expire the demo
float PaintersSortDemo::Expires() {
	return 10.0f;
}