#ifndef FACTORYPRODUCTIONEXCEPTION_DEF
#define FACTORYPRODUCTIONEXCEPTION_DEF

#include "DokuroException.hpp"

namespace dkr {
	
	class FactoryProductionException : public DokuroException {
	public:
		FactoryProductionException();
		FactoryProductionException(const std::string& _msg);

	private:


	};
}

#endif