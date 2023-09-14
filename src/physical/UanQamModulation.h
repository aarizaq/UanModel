
#ifndef __UAN_UANQAMMODULATION_H
#define __UAN_UANQAMMODULATION_H

#include "inet/physicallayer/wireless/common/modulation/MqamModulation.h"

namespace inet {
namespace Uan {

using namespace physicallayer;


class INET_API UanQamModulation : public MqamModulation
{
protected:
      const std::vector<ApskSymbol> *constellation;
  public:
    UanQamModulation(int constellationSize);
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "UanQamModulation"; }

    virtual const std::vector<ApskSymbol> *getConstellation() const override { return constellation; }
    virtual unsigned int getConstellationSize() const override { return constellation->size(); }
    virtual unsigned int getCodeWordSize() const override { return log2(constellation->size()); }

    virtual const ApskSymbol *mapToConstellationDiagram(const ShortBitVector& symbol) const override;
    virtual ShortBitVector demapToBitRepresentation(const ApskSymbol *symbol) const override;
    virtual double calculateSER(double snir, Hz bandwidth, bps bitrate) const override;
    virtual double calculateBER(double snir, Hz bandwidth, bps bitrate) const override;

};

class INET_API UanQam4Modulation : public UanQamModulation
{
  protected:
      static std::vector<ApskSymbol> constellationAux;
  public:
    static const UanQam4Modulation singleton;
    UanQam4Modulation():UanQamModulation(0) { constellation = &constellationAux;}
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "UanQam4Modulation"; }
};

class INET_API UanQam8Modulation : public UanQamModulation
{
protected:
     static std::vector<ApskSymbol> constellationAux;
  public:
    static const UanQam8Modulation singleton;
    UanQam8Modulation():UanQamModulation(0) { constellation = const_cast<std::vector<ApskSymbol> *> (&constellationAux);}
};

}
}
#endif
