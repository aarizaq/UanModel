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

package Uan.physical;

import inet.physicallayer.wireless.common.base.packetlevel.PropagationBase;

//
// This propagation model computes the propagation time to be proportional to
// the traveled distance, the ratio is determined by the constant propagation
// speed parameter.
//
module UanSpeedPropagation extends PropagationBase
{
    parameters:
        double temperature @unit(K)= default(293K); // 20 C
        double salinity = default(35); // ppt (parts per thousand)
        double deep @unit(m) = default(10m);
        bool useMeanDeep = default(true); // use the mean deep of source and destination to compute the speed.
        bool ignoreMovementDuringTransmission = default(true); // true means that the movement of the transmitter and the receiver during the signal transmission is ignored
        bool ignoreMovementDuringPropagation = default(true);  // true means that the movement of the transmitter and the receiver during the signal propagation is ignored
        bool ignoreMovementDuringReception = default(true);    // true means that the movement of the transmitter and the receiver during the signal reception is ignored
        @class(UanSpeedPropagation);
}

