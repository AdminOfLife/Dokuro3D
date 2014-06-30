#ifndef DRAWBUFFER_DEF
#define DRAWBUFFER_DEF

#include "../Maths/Matrix4.hpp"
#include "VertexBuffer.hpp"
#include "../Renderer/Texture2D.hpp"
#include "../Lights/LightInfo.hpp"

#include <vector>

namespace dkr {
	class DrawBuffer {
	public:
		/* The structure for a triangle to be drawn */
		struct Triangle
		{
			// Vertex values
			int vertexIndices[3];
			Vector4 vertices[3];
			Vector4 vertexColors[3];
			Vector4 vertexNormals[3];
			Vector4 vertexTextureCoord[3];
			float originalZ[3];

			// Lighting info
			Vector4 cameraSpace[3];
			Vector4 worldSpace[3];
			LightInfo* info;
			Vector4 ambient;
			Vector4 diffuse;
			Vector4 specular;

			// Face values
			Vector4 faceColor;
			Vector4 faceNormal;
			float averageZ;

			// The texture
			Texture2D* texture;

			static int Sort(Triangle _i, Triangle _j);
		};

		DrawBuffer();
		~DrawBuffer();

		// Allocate the draw buffer from a vertexBuffer
		void Allocate(const VertexBuffer& _buffer);

		// Clear the drawbuffer.
		void Clear();

		// Get the positions of the vertices.
		std::vector<Vector4>& GetPositions();

		// Get the triangle list
		std::vector<Triangle>& GetTriangles();

		// Sort the triangles (painters algorithm)
		void SortTriangles();
	private:
		std::vector<Vector4> positions;
		std::vector<Triangle> triangles;
	};
}

#endif