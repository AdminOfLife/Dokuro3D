#ifndef PIXELSHADER_DEF
#define PIXELSHADER_DEF

#include "../Maths/Matrix.hpp"
#include "../Maths/VertexBuffer.hpp"
#include "../Renderer/Color.hpp"
#include "../AlignedVector.hpp"
#include <vector>

namespace dkr {
	class IRenderer;
	class PixelShader {
	public:
		PixelShader();
		virtual ~PixelShader();

		/* Initialize the shader with a certain buffer */
		virtual void Init(const VertexBuffer& _buffer);
		
		/* Shade the pixels */
		SSEVector& Shade(SSEVector* _data);
		virtual void Shade();

	protected:
		/* Set's a SSEVector variable to be filled by an id every pixel */
		virtual void SetVariable(const std::string& _name, SSEVector& _var);

		/* The color of the current pixel */
		SSEVector inColor;
		SSEVector outColor;

	private:
		/* A list of strings and SSEVector references. */
		typedef std::vector<std::pair<std::string, void*> > StringMap;
		typedef std::pair<std::string, void*> StringMapItem;
		std::vector<std::pair<std::string, void*> > stringMap;

		/* A list of integers to SSEVector references */
		typedef std::vector<std::pair<int, void*> > IntMap;
		typedef std::pair<int, void*> IntMapItem;
		std::vector<std::pair<int, void*> > intMap;
	};
}

#endif