
#include "UanQamModulation.h"

namespace inet {
namespace Uan {

const UanQam4Modulation UanQam4Modulation::singleton;
const UanQam8Modulation UanQam8Modulation::singleton;

const double k = sqrt(3);

std::vector<ApskSymbol> UanQam8Modulation::constellationAux = {
    ApskSymbol(-1-k, 0), ApskSymbol(0, 1+k), ApskSymbol(1+k, 0),
    ApskSymbol(0, -1-k), ApskSymbol(-1, 1), ApskSymbol(1, 1),
    ApskSymbol(1, -1), ApskSymbol(-1, -1)
};

std::vector<ApskSymbol> UanQam4Modulation::constellationAux = {
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

double UanQamModulation::calculateSER(double snir, Hz bandwidth, bps bitrate) const
{
    // http://en.wikipedia.org/wiki/Eb/N0
    double EbN0 = snir * bandwidth.get() / bitrate.get();
    double EsN0 = EbN0 * log2(constellation->size());
    // http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation#Rectangular_QAM
    double Psc = 2 * (1 - 1 / sqrt(constellation->size())) * 0.5 * erfc(1 / sqrt(2) * sqrt(3.0 / (constellation->size() - 1) * EsN0));
    double ser = 1 - (1 - Psc) * (1 - Psc);
    ASSERT(0.0 <= ser && ser <= 1.0);
    return ser;
}

double UanQamModulation::calculateBER(double snir, Hz bandwidth, bps bitrate) const
{
    double EbN0 = snir * bandwidth.get() / bitrate.get();
    double constellationSize = constellation->size();
    double codeWordSize = log2(constellationSize);
    // http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation#Rectangular_QAM
    // http://en.wikipedia.org/wiki/Quadrature_amplitude_modulation#Rectangular_QAM
    double ber = 4.0 / codeWordSize * (1 - 1 / sqrt(constellationSize)) * 0.5 * erfc(1 / sqrt(2) * sqrt(3.0 * codeWordSize / (constellationSize - 1) * EbN0));
    ASSERT(0.0 <= ber && ber <= 1.0);
    return ber;
}


const ApskSymbol *UanQamModulation::mapToConstellationDiagram(const ShortBitVector& symbol) const
{
    unsigned int decimalSymbol = symbol.toDecimal();

    if (decimalSymbol >= constellation->size())
        throw cRuntimeError("Unknown input: %d", decimalSymbol);
    return &constellation->at(decimalSymbol);
}

ShortBitVector UanQamModulation::demapToBitRepresentation(const ApskSymbol *symbol) const
{
    // TODO Complete implementation: http://eprints.soton.ac.uk/354719/1/tvt-hanzo-2272640-proof.pdf
    double symbolQ = symbol->real();
    double symbolI = symbol->imag();
    double minDist = DBL_MAX;
    int nearestNeighborIndex = -1;
    for (unsigned int i = 0; i < constellationSize; i++) {
        const ApskSymbol *constellationSymbol = &constellation->at(i);
        double cQ = constellationSymbol->real();
        double cI = constellationSymbol->imag();
        double dist = (symbolQ - cQ) * (symbolQ - cQ) + (symbolI - cI) * (symbolI - cI);
        if (dist < minDist) {
            minDist = dist;
            nearestNeighborIndex = i;
        }
    }
    ASSERT(nearestNeighborIndex != -1);
    return ShortBitVector(nearestNeighborIndex, codeWordSize);
}




}
}
