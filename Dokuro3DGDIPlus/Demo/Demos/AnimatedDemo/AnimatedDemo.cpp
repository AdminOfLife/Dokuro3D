#include "AnimatedDemo.hpp"
using namespace dkr;

AnimatedDemo::AnimatedDemo() {
	rotation = 0.0f;
}

// Initialization and unloading
void AnimatedDemo::Init(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	cube.Load("Resources/PLAY.md2");
	cubeId = _memory->AddBuffer(cube.GetFrame());
	_memory->GetBuffer(cubeId).SetDrawMode(VertexBuffer::DRAW_SOLID);
	_memory->GetBuffer(cubeId).SetShading(VertexBuffer::SHADE_PHONG);
	_memory->GetBuffer(cubeId).SetTextureMode(VertexBuffer::TEXTURE_BASIC);
	_memory->GetBuffer(cubeId).SetFaceCulling(VertexBuffer::CULL_BACK);

	// Set up an animation
	ModelMD2::Animation testAnimation;
	testAnimation.AddFrame("frame000");
	testAnimation.AddFrame("frame001");
	testAnimation.AddFrame("frame002");
	testAnimation.AddFrame("frame003");
	testAnimation.AddFrame("frame004");
	testAnimation.AddFrame("frame005");
	testAnimation.AddFrame("frame006");
	testAnimation.AddFrame("frame007");
	testAnimation.AddFrame("frame008");
	testAnimation.AddFrame("frame009");
	testAnimation.AddFrame("frame010");
	testAnimation.AddFrame("frame011");
	testAnimation.AddFrame("frame012");
	testAnimation.AddFrame("frame013");
	testAnimation.AddFrame("frame014");
	testAnimation.AddFrame("frame015");
	testAnimation.AddFrame("frame016");
	testAnimation.AddFrame("frame017");
	testAnimation.AddFrame("frame018");
	testAnimation.AddFrame("frame019");
	testAnimation.AddFrame("frame020");
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
	testAnimation.AddFrame("frame037");

	cube.SetAnimation(testAnimation);

	// Set lights
	_transformer->GetLightInfo()->SetAmbient(AmbientLight(Vector4(0.3f)));
	_transformer->GetLightInfo()->AddDirectionalLight(DirectionalLight(Vector4(1.0f, 1.0f, 0.0f, 0.0f), Vector4(0.9f)));
}
void AnimatedDemo::Unload(dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	_memory->RemoveBuffer(cubeId);
	_transformer->GetLightInfo()->Clear();
}

// Update and draw the demo.
void AnimatedDemo::Update(float time, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	rotation += time * 1.0f;

	// Animated the model
	cube.Update(time * 8.0f);

}
void AnimatedDemo::Draw(HWND hWnd, dkr::GraphicsMemory* _memory, dkr::Transformer* _transformer, dkr::IRenderTarget* _renderer) {
	
	_memory->RecreateBuffer(cubeId, cube.GetAnimatedFrame());

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
					  "Perspective correct texturing\n"
					  "Animated MD2 Model\n", 0, 0, dkr::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

// Get the how long it takes to expire the demo
float AnimatedDemo::Expires() {
	return 10.0f;
}