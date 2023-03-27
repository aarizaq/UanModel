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

import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;
import inet.physicallayer.wireless.common.contract.packetlevel.IErrorModel;

//
// TODO auto-generated module
//
module UanUniDiskReceiver like IReceiver
{
        parameters:
        @signal[UanReceptionCollision](type=bool); // optional
        @statistic[UanReceptionCollision](source=ReceptionCollision; record=count);
        double energyDetection @unit(dBm); // no signal is detected at all below this reception power threshold (idle state)
        double sensitivity @unit(dBm);     // reception is not possible if the signal power is below sensitivity (idle or busy states)
        double snirThreshold @unit(dB);    // reception is not successful if the SNIR is below this threshold (unsuccessful reception)
        string snirThresholdMode = default("min");
        double centerFrequency @unit(Hz); // center frequency of the band where this receiver listens on the medium
        double bandwidth @unit(Hz);        // bandwidth of the band where this receiver listens on the medium
        string errorModelType = default("");   // NED type of the error model
        string modulation = default("QAM-4");
        @class(inet::Uan::UanUniDiskReceiver);
        @display("i=block/wrx");

    submodules:
        errorModel: <errorModelType> like IErrorModel if errorModelType != "";
}
