//
// Copyright (C) 2017 OpenSim Ltd.
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

package Uan.linklayer;

import inet.linklayer.common.WirelessInterface;

//
// Module interface for CSMA/MA network interfaces.
//
module UanCsmaCaInterface extends WirelessInterface
{
    parameters:
        double ifaceBitrate @unit(bps) = default(100bps);  
        radio.typename = default("UanTransducer");
        mac.typename = default("UanCsmaCaMac");
        mac.bitrate = ifaceBitrate;
        radio.bitrate = ifaceBitrate;        
        mac.useAck = default(false);
        mac.sifsTime =  default(1ms);
        mac.slotTime =  default(2ms);
        mac.headerLength = default(8B);         
}

