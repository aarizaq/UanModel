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

#include "UanUniDiskTransmitter.h"
#include <string.h>
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmission.h"
#include "inet/physicallayer/wireless/unitdisk/UnitDiskTransmission.h"

namespace inet {
namespace Uan {

Define_Module(UanUniDiskTransmitter);

UanUniDiskTransmitter::UanUniDiskTransmitter() :
        UanTransmitter()
{
}

void UanUniDiskTransmitter::initialize(int stage)
{

    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        preambleDuration = par("preambleDuration");
        headerLength = b(par("headerLength"));
        bitrate = bps(par("bitrate"));
        communicationRange = m(par("communicationRange"));
        interferenceRange = m(par("interferenceRange"));
        detectionRange = m(par("detectionRange"));
        UanTransmissionCreated = registerSignal("UanTransmissionCreated");
    }
}

std::ostream& UanUniDiskTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UanTransmitter";
    return FlatTransmitterBase::printToStream(stream, level, evFlags);
}

const ITransmission *UanUniDiskTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    const_cast<UanUniDiskTransmitter* >(this)->emit(UanTransmissionCreated, true);
    bps transmissionBitrate = computeTransmissionDataBitrate(packet);
    const simtime_t headerDuration = b(headerLength).get() / bps(transmissionBitrate).get();
    const simtime_t dataDuration = b(packet->getTotalLength()).get() / bps(transmissionBitrate).get();
    const simtime_t duration = preambleDuration + headerDuration + dataDuration;
    const simtime_t endTime = startTime + duration;
    // TODO: Is it necessary to include this preamble? the information can be extract for the transmission data
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    const Coord& startPosition = mobility->getCurrentPosition();
    const Coord& endPosition = mobility->getCurrentPosition();
    const Quaternion& startOrientation = mobility->getCurrentAngularPosition();
    const Quaternion& endOrientation = mobility->getCurrentAngularPosition();
    return new UnitDiskTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, communicationRange, interferenceRange, detectionRange);
}

}

}
