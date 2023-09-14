//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "UanTransmission.h"

namespace inet {

namespace physicallayer {

UanTransmission::UanTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, b headerLength, b dataLength, const IModulation *modulation, Hz bandwidth, const simtime_t symbolTime, bps bitrate, double codeRate):
        ApskTransmission(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, packetModel, bitModel, symbolModel, sampleModel, analogModel, headerLength, dataLength, modulation, bandwidth, symbolTime, bitrate, codeRate)

{
}

std::ostream& UanTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UanTransmission";
    return TransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

