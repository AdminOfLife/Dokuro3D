#ifndef VERTEXBUFFER_DEF
#define VERTEXBUFFER_DEF

#include <string>
#include <vector>
#include <map>

#include "../Maths/Matrix4.hpp"
#include "../Renderer/Texture2D.hpp"

namespace dkr {
	namespace DataType {
		enum DataType {
			POSITION = 0,
			COLOR,
			TEXTURE,
			NORMAL
		};
	}

	class VertexBuffer {
	public:
		VertexBuffer();
		~VertexBuffer();

		/* Clear the data */
		void Clear();

		/* Get the data buffers */
		std::vector<Vector4>& GetPositions() const;
		std::vector<int>& GetPositionsIndex() const;

		std::vector<Vector4>& GetColors() const;
		std::vector<int>& GetColorsIndex() const;
		
		std::vector<Vector4>& GetTextureCoords() const;
		std::vector<int>& GetTextureCoordsIndex() const;

		std::vector<Vector4>& GetNormals() const;
		std::vector<int>& GetNormalsIndex() const;

		std::vector<Vector4>& GetFaceNormals() const;
		std::vector<int>& GetFaceNormalsIndex() const;

		std::vector<Vector4>& GetFaceColors() const;

		/* Get the material */
		const Vector4& GetAmbient() const;
		const Vector4& GetDiffuse() const;
		const Vector4& GetSpecular() const;
		void SetAmbient(const Vector4& _amb);
		void SetDiffuse(const Vector4& _dif);
		void SetSpecular(const Vector4& _spec);

		/* Enable backface culling */
		enum CullType {
			CULL_BACK,
			CULL_FRONT,
			CULL_NONE
		};
		void SetFaceCulling(CullType _type);
		CullType GetFaceCulling() const;

		/* Set the shading model */
		enum ShadeType {
			SHADE_FLAT,
			SHADE_SMOOTH,
			SHADE_PHONG,
		};
		void SetShading(ShadeType _type);
		ShadeType GetShading() const;

		/* Set the draw mode */
		enum DrawType {
			DRAW_LINES,
			DRAW_SOLID
		};
		void SetDrawMode(DrawType _type);
		DrawType GetDrawMode() const;

		/* Set the texturing mode */
		enum TextureType {
			TEXTURE_NONE,
			TEXTURE_BASIC,
			TEXTURE_NORMALMAP,
		};
		void SetTextureMode(TextureType _type);
		TextureType GetTextureMode() const;
		void SetTexture(Texture2D* _texture);
		Texture2D* GetTexture() const;

	private:
		/* The face culling type */
		CullType cullType;

		/* The shading model */
		ShadeType shadeType;

		/* The drawing mode */
		DrawType drawMode;

		/* The texturing mode */
		TextureType textureMode;
		Texture2D* texture;

		/* The data stored contiguosly */
		std::vector<Vector4> positions;
		std::vector<int> positionsIndex;

		std::vector<Vector4> colors;
		std::vector<int> colorsIndex;

		std::vector<Vector4> textureCoords;
		std::vector<int> textureIndex;

		std::vector<Vector4> normals;
		std::vector<int> normalsIndex;

		std::vector<Vector4> faceNormals;
		std::vector<int> faceNormalsIndex;

		std::vector<Vector4> faceColors;

		/* Material data */
		Vector4 ambient;
		Vector4 diffuse;
		Vector4 specular;
	};
}

#endif