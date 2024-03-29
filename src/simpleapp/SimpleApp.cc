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

#include "simpleapp/SimpleApp.h"
#include "simpleapp/AppPacket_m.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

namespace inet {
namespace Uan {

Define_Module(SimpleApp);

void SimpleApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
       // std::pair<double,double> coordsValues = std::make_pair(-1, -1);
       // cModule *host = getContainingNode(this);
        // Generate random location for nodes if circle deployment type
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        do {
            timeToFirstPacket = par("timeToFirstPacket");
            EV << "Wylosowalem czas :" << timeToFirstPacket << endl;
            //if(timeToNextPacket < 5) error("Time to next packet must be grater than 3");
        } while(timeToFirstPacket <= 5);

        //timeToFirstPacket = par("timeToFirstPacket");
        sendMeasurements = new cMessage("sendMeasurements");
        numberOfPacketsToSend = par("numberOfPacketsToSend");

        sentPackets = 0;
        receivedADRCommands = 0;
        if(numberOfPacketsToSend == -1 || sentPackets < numberOfPacketsToSend)
            scheduleAt(simTime()+timeToFirstPacket, sendMeasurements);
        UanAppPacketSent = registerSignal("UanAppPacketSent");
    }
}


void SimpleApp::finish()
{
    cModule *host = getContainingNode(this);
    auto mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
    Coord coord = mobility->getCurrentPosition();
    recordScalar("sentPackets", sentPackets);
    recordScalar("receivedADRCommands", receivedADRCommands);
}

void SimpleApp::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == sendMeasurements)
        {
            sendPacket();
            if (simTime() >= getSimulation()->getWarmupPeriod())
                sentPackets++;
            if(numberOfPacketsToSend == -1 || sentPackets < numberOfPacketsToSend)
            {
                scheduleAt(simTime() + par("timeToNextPacket"), sendMeasurements);
            }
            else
                delete msg;
        }
    }
    else
    {
        handleMessageFromLowerLayer(msg);
        delete msg;
        //cancelAndDelete(sendMeasurements);
        //sendMeasurements = new cMessage("sendMeasurements");
        //scheduleAt(simTime(), sendMeasurements);
    }
}

void SimpleApp::handleMessageFromLowerLayer(cMessage *msg)
{
    auto pkt = check_and_cast<Packet *>(msg);
    const auto & packet = pkt->peekAtFront<AppPacket>();
    if (packet == nullptr)
        throw cRuntimeError("No AppPAcket header found");
    if (simTime() >= getSimulation()->getWarmupPeriod())
        receivedADRCommands++;
}

bool SimpleApp::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void SimpleApp::sendPacket()
{
    auto pktRequest = new Packet("DataFrame");
    pktRequest->setKind(DATA);

    auto payload = makeShared<AppPacket>();

    pktRequest->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::unknown);
    pktRequest->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress(par("destAddress").stringValue()));


    payload->setChunkLength(B(par("dataSize").intValue()));

    lastSentMeasurement = rand();
    payload->setSampleMeasurement(lastSentMeasurement);


    pktRequest->insertAtBack(payload);
    send(pktRequest, "appOut");
}

} //end namespace inet
}
