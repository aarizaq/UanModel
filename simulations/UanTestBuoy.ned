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
import Uan.physical.UanSoundMedium;
import inet.flora.loraphy.LoRaMedium;
import inet.visualizer.contract.IIntegratedVisualizer;
@license(LGPL);
network UanTestBuoy
{
    parameters:
        int numberOfNodes = default(1);
        int numberBuoys = default(1);
        @display("bgb=355.824,356.832");
    submodules:
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=550,50");
            networkRouteVisualizer.displayRoutes = true;
        }
        soundMedium: UanSoundMedium {
            @display("p=109,22");
        }
        radioMedium: LoRaMedium {
            pathLoss.d0 = m(1000);
            pathLoss.PL_d0_db = 126.43;
            pathLoss.gamma = 1.76;
            pathLoss.sigma = 8;
            @display("p=34,22");
        }
        uanNode[numberOfNodes]: UanNode {
            @display("p=176,178");
        }
        uanBuoy[numberBuoys]: UanBuoy {
            @display("p=118,225;is=vs");
        }
    connections:

}
