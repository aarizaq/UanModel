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

import Uan.physical.UanTransmitter;
import Uan.physical.UanReceiver;
import inet.physicallayer.wireless.common.base.packetlevel.FlatRadioBase;
import inet.power.storage.IdealEpEnergyStorage;
module UanTransducer extends FlatRadioBase
{
    parameters:
        antenna.typename = default("IsotropicAntenna");
        transmitter.typename = default("UanTransmitter");
        receiver.typename = default("UanReceiver");
        string soundMediumModule = default("soundMedium"); // The idea is to have different mediums for radio and sound.

        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[radioModeChanged](type=long);
        @signal[listeningChanged];
        @signal[receptionStateChanged](type=long);
        @signal[transmissionStateChanged](type=long);
        @signal[receivedSignalPartChanged](type=long);
        @signal[transmittedSignalPartChanged](type=long);
        @signal[transmissionStarted];
        @signal[transmissionEnded];
        @signal[receptionStarted];
        @signal[receptionEnded];
        @signal[minSNIR];
        @signal[packetErrorRate];
        @signal[bitErrorRate];
        @signal[symbolErrorRate];
        @signal[droppedPacket](type=long);

        @statistic[radioMode](title="Radio mode"; source=radioModeChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[receptionState](title="Radio reception state"; source=receptionStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[transmissionState](title="Radio transmission state"; source=transmissionStateChanged; record=count,vector; interpolationmode=sample-hold);


        centerFrequency = default(1000 Hz);
        bandwidth = default(250 Hz);

        // 802.15.4-2006 (page 28)
        *.bitrate = default(50 bps);

        // 4 octets Preamble
        // 1 octet SFD
        // 7 bit Frame length
        // 1 bit Reserved
        *.headerBitLength = 1 b;

        receiver.energyDetection = default(60dBm); // it should be pascals
        receiver.sensitivity = default(60dBm); // it should be pascals

        // There is no fixed boundary, because of the
        // DSSS and the capture effect. Taking the sensitivity minus some
        // arbitrary value as an approximate guess.
        receiver.minInterferencePower = default(0dBm);

        // Minimum SNIR
        receiver.snirThreshold = default(-8 dB);

        //Default transmitter power set to maximum for cache limits
        transmitter.powerTransmissionInDb = default(190dBm);
        transmitter.preambleDuration = 0.001s;
        //*.energySourceModule = default(absPath(energySourceModule));
        //*.energySourceModule = default(absPath(energySourceModule));
        @class(inet::Uan::UanTransducer); //originally it was @class(Radio);
        @display("bgb=215,413");
    submodules:
        IdealEpEnergyStorage: IdealEpEnergyStorage {
            @display("p=175.974,237.69");
        }

}
