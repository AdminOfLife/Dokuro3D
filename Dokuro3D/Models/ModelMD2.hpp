#ifndef MODELMD2_DEF
#define MODELMD2_DEF

#include <string>
#include <fstream>
#include <map>

#include "../Renderer/IRenderTarget.hpp"
#include "../Transformer/Transformer.hpp"

#include "../GraphicsMemory/VertexBuffer.hpp"
#include "../Maths/Vector4.hpp"

#include "../Renderer/Texture2D.hpp"

#include "../Exceptions/DokuroException.hpp"

namespace dkr {

	class ModelMD2 {
	public:
		// Class representing an animation. 
		class Animation {
		public:
			Animation();
			Animation(bool _loop);

			// Add a frame (add these in order)
			void AddFrame(const std::string& _name);

			// Update the animation.
			void Update(float _time);

			// Get the current (interpolated) frame
			const VertexBuffer& GetFrame(const ModelMD2& _model);

		private:
			// Linearly interpolate between two points.
			Vector4 Lerp(const Vector4& _start, const Vector4& _end, float _value) const;

			bool loop;
			float frame;
			std::vector<std::string> frameNames;

			// Contain a vertexbuffer to avoid copying
			VertexBuffer buffer;
		};

		ModelMD2();
		ModelMD2(const std::string& _filename);
		ModelMD2(std::istream& _in, const std::string& _workDir);
		~ModelMD2();

		/* Load a model from a file */
		void Load(const std::string& _filename);
		/* Load a model from a stream */
		void Load(std::istream& _in, const std::string& _workDir);

		/* Set the animation */
		void SetAnimation(std::string _anim);

		/* Set the frame of the animation */
		void SetFrame(float _frame);

		/* Get a frame from the animation */
		const VertexBuffer& GetFrame(std::string _ident) const;
		const VertexBuffer& GetFrame() const;
		const VertexBuffer& GetAnimatedFrame();

		/* Set the animation */
		void SetAnimation(const Animation& _anim);

		/* Update the model (for animation) */
		void Update(float _time);

	private:
		// The MD2 header struct
		struct Header {
			char ident[4]; // The identifier for the md2 model (should be "IDP2")
			int version; // Quake 2 used version 8

			int texturewidth; // The size of the texture for the model
			int textureheight;

			int framesize; // The size of a frame

			int numSkins; // The number of skins this model has
			int numVertices; // Number of vertices each frame has
			int numTextureCoords; // The number of texture coords each frame has
			int numTris; // The number of triangles
			int numGLcmds; // The number of opengl commands to use (We're not using opengl, so not very useful)
			int numFrames; // The number of frames

			int offsetSkins; // Offset for the skins data
			int offsetTexture; // offset texture coordinate data
			int offsetTris; // offset triangle data
			int offsetFrames; // offset frame data
			int offsetGLcmds; // offset OpenGL command data
			int offsetEnd; // offset end of file
		};

		// A vector from the file
		struct Vector {
			float x, y, z;
		};

		// A texture name
		struct TextureName {
			char name[64];
		};

		// A texuter coordinate
		struct TextureCoord {
			short s, t;
		};

		// Represents a triangle
		struct Triangle {
			unsigned short vertices[3]; // Indices to the vertices
			unsigned short textureCoords[3]; // Indices to the texture coordinates
		};

		// Represents a vertex (compressed)
		struct Vertex {
			unsigned char v[3]; // Compressed x, y and z coords
			unsigned char normalIndex; // An index into the normals
		};

		// Information for a single frame
		struct FrameInfo {
			Vector scale; // The scale to use to uncompress the model
			Vector translate; // The translate the points by this to uncompress them
			char name[16]; // A name for the frame
		};

		// An actual frame
		struct Frame {
			FrameInfo info;
			Vertex* vertices; // The vertex data
		};

		// The normal table
		static Vector normals[];

		// The header of the model
		Header header;

		// The texture
		Texture2D* texture;
		Texture2D* normalMap;

		// Data for the frames
		std::map<std::string, VertexBuffer> frames;

		// The current animation.
		Animation anim;
	};

}

#endif