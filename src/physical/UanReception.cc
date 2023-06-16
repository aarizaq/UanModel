/*
 */

#include "UanReception.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarReceptionAnalogModel.h"
namespace inet {
namespace Uan {

UanReception::UanReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const IReceptionAnalogModel *analogModel, TapVector arrivals, omnetpp::simtime_t resolution) :
        ReceptionBase(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, analogModel),        tapVector(arrivals),
        resolution(resolution)
{
}

W UanReception::getPower() const
{
    auto scalarRec = check_and_cast<const ScalarReceptionAnalogModel*>(this->getAnalogModel());
    return scalarRec->getPower();
}



W UanReception::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    auto scalarRec = check_and_cast<const ScalarReceptionAnalogModel*>(this->getAnalogModel());
    return scalarRec->getPower();
}

}
}
