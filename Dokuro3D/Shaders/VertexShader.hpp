#ifndef VERTEXSHADER_DEF
#define VERTEXSHADER_DEF

#include "../Maths/SSEMatrix.hpp"
#include "../Maths/VertexBuffer.hpp"

#include "../AlignedVector.hpp"
#include <map>
#include <vector>

namespace dkr {
	class Transformer;
	class VertexShader {
	public:
		VertexShader();
		virtual ~VertexShader();

		/* Initialize the shader with a certain buffer */
		virtual void Init(const VertexBuffer& _buffer, Transformer* _transformer);
		
		/* Shade the vertex */
		void Shade(SSEVector* _data, SSEVector* _out);
		virtual void Shade();

	protected:
		/* Set's a SSEVector variable to be filled by an id every pixel */
		virtual void SetVariable(const std::string& _name, SSEVector& _inVar, SSEVector& _outVar);

		/* Get the current transformer */
		Transformer* GetTransformer();

		/* The input position */
		SSEVector inPosition;

		/* The output position */
		SSEVector outPosition;

	private:
		/* A list of strings and SSEVector references. */
		typedef std::vector<std::pair<std::string, std::pair<void*, void*> > > StringMap;
		typedef std::pair<std::string, std::pair<void*, void*> > StringMapItem;
		StringMap stringMap;

		/* A list of integers to SSEVector references */
		typedef std::vector<std::pair<int, std::pair<void*, void*> > > IntMap;
		typedef std::pair<int, std::pair<void*, void*> > IntMapItem;
		IntMap intMap;

		/* The current transformer to get matrices from */
		Transformer* transformer;
	};

}

#endif