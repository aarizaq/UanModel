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

#ifndef __UAN_SIMPLEAPP_H_
#define __UAN_SIMPLEAPP_H_

#include <map>
#include <vector>
#include <omnetpp.h>
#include "AppPacket_m.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/LifecycleOperation.h"

using namespace omnetpp;

namespace inet {
namespace Uan{

/**
 * TODO - Generated class
 */
class INET_API SimpleApp : public cSimpleModule, public ILifecycle
{
    std::map<MacAddress, uint64_t> received;
    std::map<MacAddress, std::vector<simtime_t> > receivedTime;
    bool recordTime = false;
    protected:
        virtual void initialize(int stage) override;
        void finish() override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void handleMessage(cMessage *msg) override;
        virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;

        void handleMessageFromLowerLayer(cMessage *msg);

        void sendPacket();
        void sendDownMgmtPacket();

        int64_t numberOfPacketsToSend = -1;
        uint64_t sentPackets = 0;
        uint64_t recPackets = 0;
        int lastSentMeasurement;
        simtime_t timeToFirstPacket;
        cMessage *sendMeasurements = nullptr;
        //history of sent packets;
        cOutVector sfVector;
        cOutVector tpVector;

    public:
        SimpleApp() {}
        virtual ~SimpleApp() {
            cancelAndDelete(sendMeasurements);
        }
        simsignal_t UanAppPacketSent;
};

}
}
#endif
