
#ifndef __UAN_UANQAMMODULATION_H
#define __UAN_UANQAMMODULATION_H

#include "inet/physicallayer/wireless/common/modulation/MqamModulation.h"

namespace inet {
namespace Uan {

using namespace physicallayer;


class INET_API UanQamModulation : public MqamModulation
{

  public:
    UanQamModulation(int constellationSize);
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "UanQamModulation"; }
};

class INET_API UanQam4Modulation : public UanQamModulation
{
  protected:
     static const std::vector<ApskSymbol> constellation;
  public:
    static const UanQam4Modulation singleton;
    UanQam4Modulation():UanQamModulation(0){};
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "UanQam4Modulation"; }
};

class INET_API UanQam8Modulation : public UanQamModulation
{
protected:
  static const std::vector<ApskSymbol> constellation;
  public:
    static const UanQam8Modulation singleton;
    UanQam8Modulation():UanQamModulation(0){};
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override { return stream << "UanQam8Modulation"; }
};

}
}
#endif
