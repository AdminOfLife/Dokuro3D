#ifndef TRANSFORMER_DEF
#define TRANSFORMER_DEF

#include "../Maths/MatrixStack.hpp"
#include "../GraphicsMemory/VertexBuffer.hpp"
#include "../Lights/LightInfo.hpp"
#include "../GraphicsMemory/GraphicsMemory.hpp"
#include "../Renderer/IRenderTarget.hpp"

#include <string>
#include <map>

namespace dkr {
	// Forward declaration to avoid errors
	class VertexBuffer;
	class Transformer {
	public:
		Transformer();
		virtual ~Transformer();
		
		// Set the graphics memory
		void SetGraphicsMemory(GraphicsMemory* _memory);
		void SetRenderer(IRenderTarget* _renderer);

		// Transform a buffer in graphics memory.
		void Transform(int _bufferId);

		// Transform the points in a buffer
		void TransformPoints(int _bufferId);

		/* Get a matrix stack */
		MatrixStack& GetTransform();
		MatrixStack& GetProjection();
		MatrixStack& GetCamera();
		MatrixStack& GetTexture();

		/* Set the light's */
		void SetLightInfo(LightInfo* _info);
		LightInfo* GetLightInfo();

	private:
		/* No copy! */
		Transformer(const Transformer& _trans);
		Transformer& operator= (const Transformer& _trans);

		/* The graphics memory to work with */
		GraphicsMemory* memory;

		/* The rendere to work with */
		IRenderTarget* renderer;

		/* The matrix stacks */
		MatrixStack transform, camera, projection, texture;

		/* The lighting information */
		LightInfo* lightInfo;
	};
}

#endif