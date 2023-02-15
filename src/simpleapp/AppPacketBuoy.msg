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

//
// TODO generated message class
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.common.geometry.Geometry;
import simpleapp.AppPacket;

namespace inet::Uan;

enum BouyPacketType
{
    INFORMATIOM = 2;
    REQUEST = 3;
}

class AppPacketPosition extends FieldsChunk {
    int msgType @enum(AppPacketType);
    Coord position;
    short type @enum(BouyPacketType);
    simtime_t time;
}

class AppPacketInformation extends AppPacketPosition {
    int destination;
    int origin;
    simtime_t transmissionTime;
}

message BackoffMsgTimer
{
   simtime_t remainTime;
   uint64_t failures = 0;
} 
