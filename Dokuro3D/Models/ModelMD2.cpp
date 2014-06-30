#include "ModelMD2.hpp"
using namespace dkr;

// The animation class
ModelMD2::Animation::Animation() {
	loop = true;
	frame = 0.0f;
}

ModelMD2::Animation::Animation(bool _loop) {
	loop = _loop;
	frame = 0.0f;
}

void ModelMD2::Animation::AddFrame(const std::string& _name) {
	frameNames.push_back(_name);
}

void ModelMD2::Animation::Update(float _time) {
	frame += _time;

	// If we are looping, wrap the frame around.
	if (loop)
		while (frame > frameNames.size()) frame -= frameNames.size();

	// If we aren't looping.. do nothing.
}

Vector4 ModelMD2::Animation::Lerp(const Vector4& _start, const Vector4& _end, float _value) const {
	// Get the difference between the start and the end
	Vector4 diff = _end - _start;

	// Get the interpolated value
	return _start + diff * _value;
}

// Get the current frame.
const VertexBuffer& ModelMD2::Animation::GetFrame(const ModelMD2& _model) {
	// If there are no frames, we can quit here
	if (frameNames.size() <= 0 || (frame > frameNames.size() && !loop))
		throw DokuroException("Can't animate without any frames!");

	// Now get the current frame and the next frame and interpolate the values between each other.
	int currentFrame = (int)floor(frame);
	int nextFrame = (int)ceil(frame) % frameNames.size(); // The frame number should never be negative, so we don't reall need to check here.
	float fraction = frame - currentFrame; // The fractional part to interpolate with.

	// Get a buffer
	buffer = _model.GetFrame(frameNames[0]);

	// Now interpolate.
	const VertexBuffer& current = _model.GetFrame(frameNames[currentFrame]);
	const VertexBuffer& next = _model.GetFrame(frameNames[nextFrame]);
	for (unsigned int i = 0; i < current.GetPositions().size(); ++i) {
		buffer.GetPositions()[i] = Lerp(current.GetPositions()[i], next.GetPositions()[i], fraction);
	}
	for (unsigned int i = 0; i < current.GetNormals().size(); ++i) {
		buffer.GetNormals()[i] = Lerp(current.GetNormals()[i], next.GetNormals()[i], fraction);
	}
	for (unsigned int i = 0; i < current.GetFaceNormals().size(); ++i) {
		buffer.GetFaceNormals()[i] = Lerp(current.GetFaceNormals()[i], next.GetFaceNormals()[i], fraction);
	}

	// Cool we should be done.
	return buffer;
}

// The normal lookup table
ModelMD2::Vector ModelMD2::normals[] = {
	#include "anorms.h"
};

ModelMD2::ModelMD2() {
	texture = NULL;
	normalMap = NULL;
}

ModelMD2::ModelMD2(const std::string& _filename) {
	texture = NULL;
	normalMap = NULL;
	Load(_filename);
}

ModelMD2::ModelMD2(std::istream& _in, const std::string& _workDir) {
	texture = NULL;
	normalMap = NULL;
	Load(_in, _workDir);
}

ModelMD2::~ModelMD2() {
	if (texture != NULL)
		delete texture;
}

void ModelMD2::Load(const std::string& _filename) {
	// Calculate the working directory
	int end = _filename.find_last_of("/\\");
	std::string workingDir = _filename.substr(0, end);

	// Get the file as a filestream
	std::ifstream file(_filename.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		return;

	Load(file, workingDir);
	file.close();
}

void ModelMD2::Load(std::istream& _in, const std::string& _workDir) {
	// Read the header
	_in.read((char*)&header, sizeof(Header));

	// Validate the file
	if (header.ident[0] != 'I' || header.ident[1] != 'D' || header.ident[2] != 'P' || header.ident[3] != '2') {
		// Invalid file
		return;
	}
	if (header.version != 8) {
		// Invalid file
		return;
	}

	// Vars
	Triangle* triangles = new Triangle[header.numTris];
	TextureName* names = new TextureName[header.numSkins];
	TextureCoord* texCoords = new TextureCoord[header.numTextureCoords];
	Frame* frames = new Frame[header.numFrames];

	// Continue to load
	// First load the skins
	_in.seekg(header.offsetSkins, std::ios::beg);
	_in.read((char*)names, header.numSkins * sizeof(TextureName));

	// Read the texture coords
	_in.seekg(header.offsetTexture, std::ios::beg);
	_in.read((char*)texCoords, header.numTextureCoords * sizeof(TextureCoord));
	
	// Read the triangles
	_in.seekg(header.offsetTris, std::ios::beg);
	_in.read((char*)triangles, sizeof(Triangle) * header.numTris);

	// Read all the frames
	_in.seekg(header.offsetFrames, std::ios::beg);
	for (int i = 0; i < header.numFrames; ++i) {
		// Read the info of the frame
		_in.read((char*)&frames[i], sizeof(FrameInfo));

		// Allocate memory for the vertex data
		frames[i].vertices = new Vertex[header.numVertices];

		// Read all the vertices
		_in.read((char*)frames[i].vertices, sizeof(Vertex) * header.numVertices);
	}

	// Load the texture
	if (texture != NULL)
		delete texture;
	texture = new Texture2D(_workDir + "/" + names[0].name);
	if (!texture->GetLoaded()) {
		delete texture;
		texture = NULL;
	}

	// Now we have all the data we need
	// Now loop around each frame
	for (int f = 0; f < header.numFrames; ++f) {
		// Let's format it in a way so that we can render it
		VertexBuffer buffer;

		// Dump all of the vertices into the vertex buffer
		for (int i = 0; i < header.numVertices; ++i) {
			buffer.GetPositions().push_back(Vector4(
				frames[f].vertices[i].v[0] * frames[f].info.scale.x + frames[f].info.translate.x,
				frames[f].vertices[i].v[2] * frames[f].info.scale.z + frames[f].info.translate.z,
				frames[f].vertices[i].v[1] * frames[f].info.scale.y + frames[f].info.translate.y,
				1));

                // The normal indices
            buffer.GetNormalsIndex().push_back(i);
		}

		// Dump all of the texture coords
		for (int i = 0; i < header.numTextureCoords; ++i) {
			// Clamp the texture coordinates.
			if (texCoords[i].t < 0)
				texCoords[i].t = 0;
			if (texCoords[i].s < 0)
				texCoords[i].s = 0;
			if (texCoords[i].t > header.textureheight)
				texCoords[i].t = header.textureheight;
			if (texCoords[i].s > header.texturewidth)
				texCoords[i].s = header.texturewidth;

			buffer.GetTextureCoords().push_back(Vector4(texCoords[i].t / (float)header.texturewidth,
														texCoords[i].s / (float)header.textureheight,
														0, 0));
		}

		// Now let's load the triangles...
		for (int i = 0; i < header.numTris; ++i) {
			for (int j = 0; j < 3; ++j) {
				buffer.GetPositionsIndex().push_back(triangles[i].vertices[j]);
				buffer.GetTextureCoordsIndex().push_back(triangles[i].textureCoords[j]);
			}
		}

		// Pre-calculate the face normals
		for (int i = 0; i < header.numTris*3; i+=3) {
			Vector4 normal = (buffer.GetPositions()[buffer.GetPositionsIndex()[i+1]] - 
								buffer.GetPositions()[buffer.GetPositionsIndex()[i]])
									.Cross(buffer.GetPositions()[buffer.GetPositionsIndex()[i+1]] - 
										buffer.GetPositions()[buffer.GetPositionsIndex()[i+2]]);
			normal.w = 0;
			normal.Normalize();
			normal *= -1; // Flip our normal to coincide with the md2 normals. (They were the other way around).
			buffer.GetFaceNormals().push_back(normal);
								
		}

		// Calculate the averages
		for (int i = 0; i < header.numVertices; ++i) {
			buffer.GetNormals().push_back(Vector4(normals[(int)frames[f].vertices[i].normalIndex].x, normals[(int)frames[f].vertices[i].normalIndex].y, normals[(int)frames[f].vertices[i].normalIndex].z, 0));
		}

		// Put this into the frame
		this->frames[frames[f].info.name].Clear();
		this->frames[frames[f].info.name] = buffer;
		this->frames[frames[f].info.name].SetAmbient(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
		this->frames[frames[f].info.name].SetDiffuse(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		this->frames[frames[f].info.name].SetSpecular(Vector4(1.0f, 1.0f, 1.0f, 0.0f));
		this->frames[frames[f].info.name].SetTexture(texture);
	}

	// Delete the memory we allocated
	// First delete the vertices in the frames
	for (int i = 0; i < header.numFrames; ++i) {
		delete [] frames[i].vertices;
	}
	delete [] triangles;
	delete [] names;
	delete [] texCoords;
	delete [] frames;
}

const VertexBuffer& ModelMD2::GetFrame(std::string _ident) const {
	return (frames.find(_ident)->second);
}

const VertexBuffer& ModelMD2::GetFrame() const {
	return frames.begin()->second;
}

const VertexBuffer& ModelMD2::GetAnimatedFrame() {
	return anim.GetFrame(*this);
}

void ModelMD2::SetAnimation(const Animation& _anim) {
	anim = _anim;
}

void ModelMD2::Update(float _time) {
	anim.Update(_time);
}