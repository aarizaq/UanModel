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


package Uan.node;

import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import Uan.simpleapp.SimpleAppBuoy;
import Uan.linklayer.UanCsmaCaInterface;
import inet.flora.lorabase.LoRaNic;

module UanBuoy
{
    parameters:
        @networkNode();
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.energySourceModule = default(exists(energyStorage) ? absPath(".energyStorage") : "");
        *.mobilityModule = default(exists(mobility) ? absPath(".mobility") : "");
        uanNic.radio.antenna.mobilityModule = default("^.^.^.mobility");
        LoRaNic.mac.typename ="CsmaCaMacLora";
        LoRaNic.radio.transmitter.power = 1W;
        @display("bgb=297,313;i=device/accesspoint;is=vs");
    gates:
        input radioIn[2] @directIn;
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=30,47");
        }
        mobility: <default("StationaryMobility")> like IMobility {

            @display("p=30,133");
        }
        uanNic: UanCsmaCaInterface {
            @display("p=98,239");
        }
        simpleApp: SimpleAppBuoy {
            @display("p=137,32");
        }
        LoRaNic: LoRaNic {
            @display("p=210,239");
        }
    connections allowunconnected:
        simpleApp.appOut --> uanNic.upperLayerIn;
        uanNic.upperLayerOut --> simpleApp.appIn;
        simpleApp.appOutRadio --> LoRaNic.upperLayerIn;
        LoRaNic.upperLayerOut --> simpleApp.appInRadio;
        
        radioIn[0] --> uanNic.radioIn;
        radioIn[1] --> LoRaNic.radioIn;
}
