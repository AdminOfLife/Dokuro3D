#include "GraphicsMemory.hpp"
using namespace dkr;

#include "../Exceptions/DokuroException.hpp"

GraphicsMemory::GraphicsMemory() {
	nextId = 0;
}

GraphicsMemory::~GraphicsMemory() {
}

int GraphicsMemory::AddBuffer(const VertexBuffer& _buffer) {
	// Put the buffer into the graphics memory.
	graphicsMemory.insert(std::pair<int, VertexBuffer>(nextId, _buffer));

	// Put a temporary buffer into graphics memory
	DrawBuffer temp;
	temp.Allocate(_buffer);
	tempMemory.insert(std::pair<int, DrawBuffer>(nextId, temp));

	// Increment the nextId
	nextId++;

	// Return the index to this buffer.
	return nextId - 1;
}

void GraphicsMemory::RecreateBuffer(int _id, const VertexBuffer& _buffer) {
	// Put the buffer into the graphics memory.
	graphicsMemory.erase(graphicsMemory.find(_id));
	graphicsMemory.insert(std::pair<int, VertexBuffer>(_id, _buffer));

	// Put a temporary buffer into graphics memory
	DrawBuffer temp;
	temp.Allocate(_buffer);
	tempMemory.erase(tempMemory.find(_id));
	tempMemory.insert(std::pair<int, DrawBuffer>(_id, temp));
}

void GraphicsMemory::RemoveBuffer(int _index) {
	if (graphicsMemory.find(_index) != graphicsMemory.end())
		graphicsMemory.erase(graphicsMemory.find(_index));
	if (tempMemory.find(_index) != tempMemory.end())
		tempMemory.erase(tempMemory.find(_index));
}

// Get a buffer
const VertexBuffer& GraphicsMemory::GetBuffer(int _index) const {
	if (graphicsMemory.find(_index) != graphicsMemory.end())
		return graphicsMemory.find(_index)->second;
	throw new DokuroException("There is no buffer at that index!");
}
VertexBuffer& GraphicsMemory::GetBuffer(int _index) {
	if (graphicsMemory.find(_index) != graphicsMemory.end())
		return graphicsMemory.find(_index)->second;
	throw new DokuroException("There is no buffer at that index!");
}

// Get a temporary buffer
DrawBuffer& GraphicsMemory::GetTempBuffer(int _index) {
	if (tempMemory.find(_index) != tempMemory.end())
		return tempMemory.find(_index)->second;
	throw new DokuroException("There is no buffer at that index!");
}