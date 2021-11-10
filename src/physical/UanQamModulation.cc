
#include "UanQamModulation.h"

namespace inet {
namespace Uan {

const UanQam4Modulation UanQam4Modulation::singleton;
const UanQam8Modulation UanQam8Modulation::singleton;

const double k = sqrt(3);

const std::vector<ApskSymbol> UanQam8Modulation::constellation = {
    ApskSymbol(-1-k, 0), ApskSymbol(0, 1+k), ApskSymbol(1+k, 0),
    ApskSymbol(0, -1-k), ApskSymbol(-1, 1), ApskSymbol(1, 1),
    ApskSymbol(1, -1), ApskSymbol(-1, -1)
};

const std::vector<ApskSymbol> UanQam4Modulation::constellation = {
    ApskSymbol(-1, 1), ApskSymbol(1, 1),
    ApskSymbol(1, -1), ApskSymbol(-1, -1)
};


UanQamModulation::UanQamModulation(int constellationSize) : MqamModulation(std::log2(constellationSize))
{
    if(constellationSize == 0)
        return;
    if ((constellationSize &(constellationSize - 1)) != 0) {
        double k = std::log2(constellationSize);
        throw cRuntimeError("Constellation size not a power of 2, code word size %f", k);
    }
}





}
}
