#ifndef TEXTURE2D_DEF
#define TEXTURE2D_DEF

#include <string>
#include "../IncludedLibraries/Devil/il.h"

namespace dkr {
	// A class to represent a texture
	class Texture2D  {
	public:
		// Create a texture2D
		Texture2D(const std::string& _name);
		virtual ~Texture2D();

		// Get information about the texture.
		virtual int GetWidth();
		virtual int GetHeight();
		virtual int GetDepth();

		// Get the pixel data
		virtual void* GetPixels();

		// Sample the texture
		unsigned int Sample(int _u, int _v);
		
		// Check if the file was loaded.
		virtual bool GetLoaded();

		// Load from a file
		virtual bool LoadFromFile(const std::string& _name);

	private:
		// The image identifier
		ILuint image;

		// The width and height of the texture
		int width;
		int height;
		unsigned int* data;

		// Whether or not devIL has been initialized.
		static bool devilInit;
	};
}

#endif