#include "FactoryProductionException.hpp"
using namespace dkr;

FactoryProductionException::FactoryProductionException()
	: DokuroException("FactoryProductionException: ") {
}

FactoryProductionException::FactoryProductionException(const std::string& _msg) 
	: DokuroException("FactoryProductionException: " + _msg) {
}