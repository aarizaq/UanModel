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

import inet.physicallayer.wireless.common.medium.RadioMedium;

module UanUniDiskSoundMedium extends RadioMedium
{
    parameters:
        propagation.typename = default("ConstantSpeedPropagation");
        propagation.propagationSpeed = default(1500mps);
        
        analogModel.typename = default("UnitDiskMediumAnalogModel");
        signalAnalogRepresentation = default("unitDisk");
        mediumLimitCache.minInterferenceTime = default(1ns);       // minimum time interval to consider two overlapping signals interfering
        mediumLimitCache.maxTransmissionDuration = default(5s);  // maximum duration of a transmission on the medium        
}

