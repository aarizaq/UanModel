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

#ifndef __UAN_SIMPLEAPPBUOY_H_
#define __UAN_SIMPLEAPPBUOY_H_

#include <omnetpp.h>
#include "linklayer/UanIMac.h"
#include "physical/UanTransmitter.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/lifecycle/LifecycleOperation.h"
#include "simpleapp/AppPacketBuoy_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/flora/loraapp/SimpleLoRaApp.h"
using namespace omnetpp;

namespace inet {
namespace Uan{

#ifndef INET_WITH_FLORACODE
#  error SimpleAppBuoy uses Flora/lora model
#endif

class INET_API SimpleAppBuoy : public flora::SimpleLoRaApp, public cListener
{
    struct Timers {
        cMessage *timer = nullptr;
        simtime_t remain;
    };
    std::vector<Timers> pendingTimers;

    protected:
        virtual void initialize(int stage) override;
        virtual void finish() override;
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void handleMessage(cMessage *msg) override;
        virtual bool handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback) override;

        virtual void handleMessageFromLowerLayer(cMessage *msg) override;
        virtual void handleMessageFromLowerLayerRadio(cMessage *msg);
        virtual simtime_t sendPacket();

        virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

        physicallayer::IRadio *radio = nullptr;
        physicallayer::IRadio *transducer = nullptr;
        simtime_t timeToFirstPacket;
        BackoffMsgTimer *backoffTimer = nullptr;
        IMobility *mob = nullptr;
        m minimumDistance = m(1000); // minimum distance that allow simultaneous communication
        cPar *backoffDelay = nullptr;  // used to delay the transmission
        bps dataBitrate;
        cGate *radioGateOut = nullptr;
        cGate *transducerGateOut = nullptr;
        cGate *radioGateIn = nullptr;
        cGate *transducerGateIn = nullptr;
        UanIMac *uanMacModule = nullptr;
        const UanTransmitter * uanTranmitter = nullptr;

    public:
        SimpleAppBuoy() {}
        virtual ~SimpleAppBuoy() {
            cancelAndDelete(sendMeasurements);
            cancelAndDelete(backoffTimer);
        }
        simsignal_t UanAppPacketSent;
        //LoRa physical layer parameters
};

}
}
#endif
