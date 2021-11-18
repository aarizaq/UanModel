//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "UanSpeedPropagation.h"

#include "inet/physicallayer/wireless/common/signal/Arrival.h"

namespace inet {

namespace Uan {

Define_Module(UanSpeedPropagation);

UanSpeedPropagation::UanSpeedPropagation() :
    PropagationBase(),
    ignoreMovementDuringTransmission(false),
    ignoreMovementDuringPropagation(false),
    ignoreMovementDuringReception(false)
{
}

void UanSpeedPropagation::initialize(int stage)
{
    PropagationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        ignoreMovementDuringTransmission = par("ignoreMovementDuringTransmission");
        ignoreMovementDuringPropagation = par("ignoreMovementDuringPropagation");
        ignoreMovementDuringReception = par("ignoreMovementDuringReception");
        useMeanDeep = par("useMeanDeep");
        salinity = par("salinity");
        temperature = par("temperature").doubleValue() - 273.0;
        if (temperature < 4.0)
            throw cRuntimeError("Check temperature value,  in Kelvin %f, in Celsius %f, the sea temperature must be higher than 4 C", par("temperature").doubleValue(), temperature);

        // TODO
        if (!ignoreMovementDuringTransmission)
            throw cRuntimeError("ignoreMovementDuringTransmission is yet not implemented");
    }
}

const Coord UanSpeedPropagation::computeArrivalPosition(const simtime_t time, const Coord& position, IMobility *mobility) const
{
    // TODO return mobility->getPosition(time);
    throw cRuntimeError("Movement approximation is not implemented");
}

std::ostream& UanSpeedPropagation::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UanSpeedPropagation";
    if (level <= PRINT_LEVEL_TRACE)
        stream << EV_FIELD(ignoreMovementDuringTransmission)
               << EV_FIELD(ignoreMovementDuringPropagation)
               << EV_FIELD(ignoreMovementDuringReception);
    return PropagationBase::printToStream(stream, level);
}

const IArrival *UanSpeedPropagation::computeArrival(const ITransmission *transmission, IMobility *mobility) const
{
    arrivalComputationCount++;

    // https://sites.dartmouth.edu/dujs/2012/03/11/the-underwater-propagation-of-sound-and-its-applications/
    // C(T,P,S) = 1449.2 + 4.6T – 0.055T^2 + 0.00029T^3 + (1.34 – 0.01 T)(S – 35) + 0.16z,
    // T temperature, celsius, P salinity in PPT, z deep in meters.

    double deep = par("deep").doubleValue();

    const simtime_t startTime = transmission->getStartTime();
    const simtime_t endTime = transmission->getEndTime();
    const Coord& startPosition = transmission->getStartPosition();
    const Coord& endPosition = transmission->getEndPosition();
    const Coord& startArrivalPosition = ignoreMovementDuringPropagation ? mobility->getCurrentPosition() : computeArrivalPosition(startTime, startPosition, mobility);

    if (par("useMeanDeep"))
        deep = std::abs((startPosition.z - startArrivalPosition.z)/2+startArrivalPosition.z);
    double speed = 1449.2 + 4.6 * temperature - 0.055 * std::pow(temperature,2) + 0.00029 * std::pow(temperature,3) +
                (1.34 - 0.01 * temperature)*(salinity - 35.0) + 0.16  * deep;

    const simtime_t startPropagationTime = startPosition.distance(startArrivalPosition) / speed;
    const simtime_t startArrivalTime = startTime + startPropagationTime;
    const Quaternion& startArrivalOrientation = mobility->getCurrentAngularPosition();
    if (ignoreMovementDuringReception) {
        const Coord& endArrivalPosition = startArrivalPosition;
        const simtime_t endPropagationTime = startPropagationTime;
        const simtime_t endArrivalTime = endTime + startPropagationTime;
        const simtime_t preambleDuration = transmission->getPreambleDuration();
        const simtime_t headerDuration = transmission->getHeaderDuration();
        const simtime_t dataDuration = transmission->getDataDuration();
        const Quaternion& endArrivalOrientation = mobility->getCurrentAngularPosition();
        return new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, preambleDuration, headerDuration, dataDuration, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
    }
    else {
        const Coord& endArrivalPosition = computeArrivalPosition(endTime, endPosition, mobility);
        const simtime_t endPropagationTime = endPosition.distance(endArrivalPosition) / propagationSpeed.get();
        const simtime_t endArrivalTime = endTime + endPropagationTime;
        const simtime_t preambleDuration = transmission->getPreambleDuration();
        const simtime_t headerDuration = transmission->getHeaderDuration();
        const simtime_t dataDuration = transmission->getDataDuration();
        const Quaternion& endArrivalOrientation = mobility->getCurrentAngularPosition();
        return new Arrival(startPropagationTime, endPropagationTime, startArrivalTime, endArrivalTime, preambleDuration, headerDuration, dataDuration, startArrivalPosition, endArrivalPosition, startArrivalOrientation, endArrivalOrientation);
    }
}

} // namespace physicallayer

} // namespace inet

