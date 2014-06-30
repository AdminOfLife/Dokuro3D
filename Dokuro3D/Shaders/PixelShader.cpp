#include "PixelShader.hpp"
#include "../Renderer/IRenderer.hpp"
using namespace dkr;

PixelShader::PixelShader() {
	/* Set the default variable color */
	SetVariable("Color", inColor);
}

PixelShader::~PixelShader() {
}

void PixelShader::Init(const VertexBuffer& _buffer) {
	/* Calculate what to put into int map */
	intMap.clear();
	for (int i = 0; i < stringMap.size(); ++i) {
		int index = _buffer.GetIndex(stringMap[i].first);
		if (index >= 0) {
			intMap.push_back(IntMapItem(index, stringMap[i].second));
		}
	}
}

SSEVector& PixelShader::Shade(SSEVector* _data) {
	/* Set up the variables */
	for (int i = 0; i < intMap.size(); ++i) {
		*(SSEVector*)(intMap[i].second) = _data[intMap[i].first];
	}

	/* Return the color */
	Shade();

	/* Return the outColor */
	return outColor;
}

void PixelShader::Shade() {
	/* Just return the normal color */
	outColor = inColor;

	return;
}

void PixelShader::SetVariable(const std::string& _name, SSEVector& _var) {
	stringMap.push_back(StringMapItem(_name, (void*)&_var));
}