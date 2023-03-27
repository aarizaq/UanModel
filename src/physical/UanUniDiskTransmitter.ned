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

package Uan.physical;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandTransmitterBase;

module UanUniDiskTransmitter extends NarrowbandTransmitterBase
{
    parameters:
        headerLength = 1b; //
        double powerTransmissionInDb @unit(dBmW) = default(190dBm);  // It should be pascals.
        double communicationRange @unit(m);                              // the range where the reception of transmissions produced by this transmitter is possible
        double interferenceRange @unit(m) = default(communicationRange); // the range where transmissions produced by this transmitter interferes with other transmissions
        double detectionRange @unit(m) = default(interferenceRange);     // the range where transmissions produced by this transmitter can be detected at all
        modulation = default("BPSK"); // TODO this is simply wrong
        @signal[UanTransmissionCreated](type=bool); // optional
        @statistic[UanTransmissionCreated](source=UanTransmissionCreated; record=count);        
        @class(inet::Uan::UanUniDiskTransmitter);
}