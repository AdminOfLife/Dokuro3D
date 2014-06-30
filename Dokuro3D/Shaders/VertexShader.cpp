#include "VertexShader.hpp"
#include "../Transformer/Transformer.hpp"
using namespace dkr;

VertexShader::VertexShader() {
	/* Set the default variable in and out positions */
	SetVariable("Position", inPosition, outPosition);
}

VertexShader::~VertexShader() {
}

void VertexShader::Init(const VertexBuffer& _buffer, Transformer* _transformer) {
	/* Calculate what to put into int map */
	intMap.clear();
	for (int i = 0; i < stringMap.size(); ++i) {
		int index = _buffer.GetIndex(stringMap[i].first);
		if (index >= 0) {
			intMap.push_back(IntMapItem(index, stringMap[i].second));
		}
	}

	transformer = _transformer;

}

void VertexShader::Shade(SSEVector* _data, SSEVector* _out) {
	/* Set up the variables */
	for (int i = 0; i < intMap.size(); ++i) {
		*(SSEVector*)intMap[i].second.first = _data[intMap[i].first];
	}

	/* Shade */
	Shade();

	/* Set the out variables */
	for (int i = 0; i < intMap.size(); ++i) {
		_out[intMap[i].first] = *(SSEVector*)intMap[i].second.second;
	}

}

void VertexShader::Shade() {
	SSEMatrix matrix;
	matrix *= GetTransformer()->GetStack("Projection").Peek();
	matrix *= GetTransformer()->GetStack("Transform").Peek();
	matrix *= GetTransformer()->GetStack("Camera").Peek();			
	matrix.Transform(inPosition, outPosition);

	outPosition.x /= outPosition.w;
	outPosition.y /= outPosition.w;
	outPosition.z /= outPosition.w;
	outPosition.w = 1;
}

void VertexShader::SetVariable(const std::string& _name, SSEVector& _inVar, SSEVector& _outVar) {
	stringMap.push_back(StringMapItem(_name, std::pair<void*, void*>((void*)&_inVar, (void*)&_outVar)));
}

Transformer* VertexShader::GetTransformer() {
	return transformer;
}