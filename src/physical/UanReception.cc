/*
 */

#include "UanReception.h"

namespace inet {
namespace Uan {

UanReception::UanReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, Hz centerFrequency, Hz bandwidth, W receivedPower, TapVector arrivals, omnetpp::simtime_t resolution) :
        ScalarReception(radio, transmission, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation, centerFrequency, bandwidth, receivedPower),
        receivedPower(receivedPower),
        tapVector(arrivals),
        resolution(resolution)
{
}


W UanReception::computeMinPower(simtime_t startTime, simtime_t endTime) const
{
    return receivedPower;
}

}
}
