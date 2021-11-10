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
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "UanTransmitter.h"
#include "UanQamModulation.h"
#include <string.h>
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmission.h"

namespace inet {
namespace Uan {


Define_Module(UanTransmitter);

UanTransmitter::UanTransmitter() :
    FlatTransmitterBase()
{
}

void UanTransmitter::initialize(int stage)
{

    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {

        if (!strcmp("QAM-4", par("modulation").stringValue()))
            modulation = &UanQam4Modulation::singleton;
        else if (!strcmp("QAM-8", par("modulation").stringValue()))
            modulation = &UanQam8Modulation::singleton;
        else
            modulation = ApskModulationBase::findModulation(par("modulation").stringValue());

        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));

        preambleDuration = par("preambleDuration");
        headerLength = b(par("headerLength"));
        bitrate = bps(par("bitrate"));
        power = W(par("power"));
        UanTransmissionCreated = registerSignal("UanTransmissionCreated");
    }
}

std::ostream& UanTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UanTransmitter";
    return FlatTransmitterBase::printToStream(stream, level, evFlags);
}

const ITransmission *UanTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    const_cast<UanTransmitter* >(this)->emit(UanTransmissionCreated, true);

    W transmissionPower = computeTransmissionPower(packet);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = b(headerLength).get() / bps(transmissionBitrate).get();
    const simtime_t dataDuration = b(packet->getTotalLength()).get() / bps(transmissionBitrate).get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    return new ScalarTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, modulation, headerLength, packet->getTotalLength(), centerFrequency, bandwidth, transmissionBitrate, transmissionPower);
}

}

}
