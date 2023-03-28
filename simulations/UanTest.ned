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

package Uan.simulations;

import Uan.node.UanNode;
import Uan.node.UanBuoy;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
@license(LGPL);
network UanTest
{
    parameters:
        int numberOfNodes = default(1);
        @display("bgb=355.824,356.832");
    submodules:
         visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=550,50");
            mediumVisualizer.mediumModule = "soundMedium";
            mediumVisualizer.displayCommunicationRanges = true;
            networkRouteVisualizer.displayRoutes = true;
            
        }
        soundMedium: <default("UanSoundMedium")> like IRadioMedium if typename != "" 
        {
            @display("p=109,22");
        }

        uanNode1: UanNode {
            @display("p=176,178");
        }
        uanNode2: UanNode {
            @display("p=52,189");
        }

    connections:

}
