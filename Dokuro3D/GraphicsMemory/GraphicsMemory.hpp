#ifndef GRAPHICS_MEMORY_DEF
#define GRAPHICS_MEMORY_DEF

#include "VertexBuffer.hpp"
#include "DrawBuffer.hpp"

#include <vector>
#include <unordered_map>

namespace dkr {
	// Represents graphics memory.
	class GraphicsMemory {
	public:
		GraphicsMemory();
		virtual ~GraphicsMemory();

		// Add a buffer
		int AddBuffer(const VertexBuffer& _buffer);
		void RecreateBuffer(int _id, const VertexBuffer& _buffer);

		// Remove buffer
		void RemoveBuffer(int _index);

		// Get a buffer from graphics memory.
		const VertexBuffer& GetBuffer(int _index) const;
		VertexBuffer& GetBuffer(int _index);

		// Get a temporary buffer graphics memory.
		DrawBuffer& GetTempBuffer(int _index);

	private:
		std::unordered_map<int, VertexBuffer> graphicsMemory;
		std::unordered_map<int, DrawBuffer> tempMemory;

		// Keep track of what id to give next.
		int nextId;
	};
}

#endif