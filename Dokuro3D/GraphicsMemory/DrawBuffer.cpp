#include "DrawBuffer.hpp"
using namespace dkr;

#include <algorithm>

// Sort method for triangles
int DrawBuffer::Triangle::Sort(Triangle _i, Triangle _j) {
	return (_i.averageZ > _j.averageZ);
}

DrawBuffer::DrawBuffer() {
}

DrawBuffer::~DrawBuffer() {
}

void DrawBuffer::Allocate(const VertexBuffer& _buffer) {
	// Allocate the positions vector.
	GetPositions().clear();
	GetPositions().resize(_buffer.GetPositions().size(), Vector4());
}

void DrawBuffer::Clear() {
	GetTriangles().clear();
}

std::vector<Vector4>& DrawBuffer::GetPositions() {
	return positions;
}

std::vector<DrawBuffer::Triangle>& DrawBuffer::GetTriangles() {
	return triangles;
}

void DrawBuffer::SortTriangles() {
	// Calculate average z values
	for (unsigned int i = 0; i < GetTriangles().size(); ++i) {
		GetTriangles()[i].averageZ = (GetTriangles()[i].vertices[0].z + GetTriangles()[i].vertices[1].z + GetTriangles()[i].vertices[2].z) / 3;
	}

	// Sort them
	std::sort(GetTriangles().begin(), GetTriangles().end(), Triangle::Sort);
}