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

import inet.physicallayer.wireless.common.contract.packetlevel.IBackgroundNoise;

//
// This background noise model describes noise that does not change over space,
// time and frequency. It produces scalar noise signals that can be further used
// in scalar computations.
//
module UanScalarBackgroundNoise like IBackgroundNoise
{
    parameters:
        double shipping = default(0); // must be in the range 0 to 1;
        double windSpeed @unit(mps)= default(1mps);
        @class(inet::Uan::UanScalarBackgroundNoise);
        @display("i=block/mac");
}

