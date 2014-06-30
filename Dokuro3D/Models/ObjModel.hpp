#ifndef OBJMODEL_DEF
#define OBJMODEL_DEF

#include <string>
#include <ifstream>

#include "../Transformer/Transformer.hpp"
#include "../Renderer/IRenderer.hpp"

namespace dkr {

	class ObjModel {
	public:
		ObjModel();
		ObjModel(std::ostream& _in);
		ObjModel(const std::string& _filename);
		~ObjModel();

		/* Load the model */
		void Load(const std::string& _filename);
		void Load(std::ostream& _in);

		/* Draw the model */
		void Draw(Transformer& _trans, IRenderer& _renderer);

	private:

	};

}

#endif