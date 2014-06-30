#include "VertexBuffer.hpp"
using namespace dkr;

VertexBuffer::VertexBuffer() {
	// Set default shading
	SetShading(VertexBuffer::SHADE_PHONG);

	// Set default culling
	SetFaceCulling(VertexBuffer::CULL_BACK);

	// Set the draw mode 
	SetDrawMode(VertexBuffer::DRAW_SOLID);

	// Set the texture to null
	SetTextureMode(VertexBuffer::TEXTURE_BASIC);
	SetTexture(NULL);
}

VertexBuffer::~VertexBuffer() {
}

/* Clear the buffer */
void VertexBuffer::Clear() {
	positions.clear();
	positionsIndex.clear();

	colors.clear();
	colorsIndex.clear();

	textureCoords.clear();
	textureIndex.clear();

	normals.clear();
	normalsIndex.clear();

	faceNormals.clear();
	faceNormalsIndex.clear();

	faceColors.clear();
}

/* Allow access to the data */
std::vector<Vector4>& VertexBuffer::GetPositions() const {
	return const_cast<std::vector<Vector4>& >(positions);
}

std::vector<Vector4>& VertexBuffer::GetColors() const {
	return const_cast<std::vector<Vector4>& >(colors);
}

std::vector<Vector4>& VertexBuffer::GetTextureCoords() const {
	return const_cast<std::vector<Vector4>& >(textureCoords);
}

std::vector<Vector4>& VertexBuffer::GetNormals() const {
	return const_cast<std::vector<Vector4>& >(normals);
}

std::vector<Vector4>& VertexBuffer::GetFaceNormals() const {
	return const_cast<std::vector<Vector4>& >(faceNormals);
}

std::vector<int>& VertexBuffer::GetPositionsIndex() const {
	return const_cast<std::vector<int>& >(positionsIndex);
}

std::vector<int>& VertexBuffer::GetColorsIndex() const {
	return const_cast<std::vector<int>& >(colorsIndex);
}

std::vector<int>& VertexBuffer::GetTextureCoordsIndex() const {
	return const_cast<std::vector<int>& >(textureIndex);
}

std::vector<int>& VertexBuffer::GetNormalsIndex() const {
	return const_cast<std::vector<int>& >(normalsIndex);
}

std::vector<int>& VertexBuffer::GetFaceNormalsIndex() const {
	return const_cast<std::vector<int>& >(faceNormalsIndex);
}

std::vector<Vector4>& VertexBuffer::GetFaceColors() const {
	return const_cast<std::vector<Vector4>& >(faceColors);
}

/* The material data */
const Vector4& VertexBuffer::GetAmbient() const {
	return ambient;
}
const Vector4& VertexBuffer::GetDiffuse() const {
	return diffuse;
}
const Vector4& VertexBuffer::GetSpecular() const {
	return specular;
}
void VertexBuffer::SetAmbient(const Vector4& _amb) {
	ambient = _amb;
}
void VertexBuffer::SetDiffuse(const Vector4& _dif) {
	diffuse = _dif;
}
void VertexBuffer::SetSpecular(const Vector4& _spec) {
	specular = _spec;
}

/* Back face culling */
void VertexBuffer::SetFaceCulling(VertexBuffer::CullType _type) {
	cullType = _type;
}
VertexBuffer::CullType VertexBuffer::GetFaceCulling() const {
	return cullType;
}

/* Shading model */
void VertexBuffer::SetShading(VertexBuffer::ShadeType _type) {
	shadeType = _type;
}
VertexBuffer::ShadeType VertexBuffer::GetShading() const {
	return shadeType;
}

/* Draw mode */
void VertexBuffer::SetDrawMode(VertexBuffer::DrawType _type) {
	drawMode = _type;
}
VertexBuffer::DrawType VertexBuffer::GetDrawMode() const {
	return drawMode;
}

/* Texture mode */
void VertexBuffer::SetTextureMode(VertexBuffer::TextureType _type) {
	textureMode = _type;
}
VertexBuffer::TextureType VertexBuffer::GetTextureMode() const {
	return textureMode;
}
void VertexBuffer::SetTexture(Texture2D* _texture) {
	texture = _texture;
}
Texture2D* VertexBuffer::GetTexture() const {
	return texture;
}