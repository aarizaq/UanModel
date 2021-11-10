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
import Uan.simpleapp.SimpleApp;
import Uan.linklayer.UanCsmaCaInterface;


module UanNode
{
    parameters:
        @networkNode();
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.energySourceModule = default(exists(energyStorage) ? absPath(".energyStorage") : "");
        *.mobilityModule = default(exists(mobility) ? absPath(".mobility") : "");
        uanNic.radio.antenna.mobilityModule = default("^.^.^.mobility");
        @display("bgb=297,313;i=device/accesspoint;is=vs");
    submodules:
        interfaceTable: InterfaceTable {
            @display("p=30,26");
        }
        mobility: <default("StationaryMobility")> like IMobility {

            @display("p=24,88");
        }
        uanNic: UanCsmaCaInterface {
            @display("p=137,239");
        }
        simpleApp: SimpleApp {
            @display("p=137,32");
        }
    connections allowunconnected:

        simpleApp.appOut --> uanNic.upperLayerIn;
        uanNic.upperLayerOut --> simpleApp.appIn;
}
