#include "Texture2D.hpp"
using namespace dkr;

// Static bool determining whether or not devIL has been initialized or not.
bool Texture2D::devilInit = false;

Texture2D::Texture2D(const std::string& _name) {
	image = 0;
	LoadFromFile(_name);
}

Texture2D::~Texture2D() {
	// Clean up the image.
	if (image != 0)
		ilDeleteImage(image);
}

int Texture2D::GetWidth() {
	ilBindImage(image);
	return ilGetInteger(IL_IMAGE_WIDTH);
}
int Texture2D::GetHeight() {
	ilBindImage(image);
	return ilGetInteger(IL_IMAGE_HEIGHT);
}
int Texture2D::GetDepth() {
	ilBindImage(image);
	return ilGetInteger(IL_IMAGE_DEPTH);
}
void* Texture2D::GetPixels() {
	ilBindImage(image);
	return ilGetData();
}

unsigned int Texture2D::Sample(int _u, int _v) {
	if (_u < 0 || _u >= height || _v < 0 || _v >= width)
		return 0;
	return data[_u * width + _v];
}

bool Texture2D::GetLoaded() {
	return image != 0;
}

bool Texture2D::LoadFromFile(const std::string& _name) {
	// Check if devIL has been initialized already
	if (!devilInit) {
		ilInit();
		
		// Make sure images have their origin in the upper left corner.
		//ilEnable(IL_ORIGIN_SET);
		//ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		
		// Make sure we don't call this again.
		devilInit = true;
	}

	// Load the image

	// First generate an image.
	ilGenImages(1, &image);

	// Bind the image so we can load something into it.
	ilBindImage(image);

	// Load the image.
	if (!ilLoadImage(_name.c_str())) {
		ilDeleteImages(1, &image);
		image = 0;
		return false;
	}

	// Make sure this is converted to the right format.
	char* data = new char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * sizeof(unsigned int)];

	// First copy the data.
	ILuint copy = ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_BGRA, IL_UNSIGNED_BYTE, data);

	// Change the format of the image
	ILuint change = ilTexImage(ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, 4, IL_BGRA, IL_UNSIGNED_BYTE, data);
	
	// Delete the temporary data.
	delete [] data;

	// Cache the width and height
	width = GetWidth();
	height = GetHeight();
	this->data = (unsigned int*)GetPixels();

	return true;
}