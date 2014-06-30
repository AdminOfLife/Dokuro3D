#include "ObjModel.hpp"
using namespace dkr;

ObjModel::ObjModel() {
}

ObjModel::ObjModel(std::ostream& _in) {
	Load(_in);
}

ObjModel::ObjModel(const std::string& _filename) {
	Load(_filename);
}

/* ---------------------------------------- Loading ------------------------------------ */
