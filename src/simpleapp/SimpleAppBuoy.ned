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

package Uan.simpleapp;

//
// TODO auto-generated module
//
simple SimpleAppBuoy
{
    parameters:
        @signal[UanAppPacketSent](type=long); // optional
        @statistic[UanAppPacketSent](source=UanAppPacketSent; record=count);
        
        
        volatile double timeToFirstPacket @unit(s) = default(10s);
        volatile double timeToNextPacket @unit(s) = default(10s);
        double initialLoRaTP @unit(dBm) = default(30dBm);
        double initialLoRaCF @unit(Hz) = default(868MHz);
        int initialLoRaSF = default(12);
        double initialLoRaBW @unit(Hz) = default(125kHz);
        int initialLoRaCR = default(4);
        bool initialUseHeader = default(true);
        bool evaluateADRinNode = default(false);
        int dataSize @unit(B) = default(10B);
        double maxTimeToFirstPacket @unit(s)= default(0s);
        
        double guardTime @unit(s) = default(5ms); // small guard time used to compute the sound transmission 
                
        double minimumDistance @unit(m) = default(1000m);
        volatile double backoffDelay @unit(s) = default(uniform(2s,4s));
        int numberOfPacketsToSend = default(-1); // -1 always sent
        int maxBackoffFailures = default(4);
        volatile string destAddress = default("00:00:00:00:00:01");
    	@class(inet::Uan::SimpleAppBuoy);
    gates:
        output appOut @labels(AppPacket/down);
        input appIn @labels(AppPacket/up);
        output appOutRadio @labels(AppPacket/down);
        input appInRadio @labels(AppPacket/up);
}
