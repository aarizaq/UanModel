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

#include "simpleapp/SimpleAppBuoy.h"
#include "physical/UanTransmitter.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"

namespace inet {
namespace Uan {

Define_Module(SimpleAppBuoy);

void SimpleAppBuoy::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // Read configuration parameters
        loRaTP = par("initialLoRaTP").doubleValue();
        loRaCF = units::values::Hz(par("initialLoRaCF").doubleValue());
        loRaSF = par("initialLoRaSF");
        loRaBW = inet::units::values::Hz(par("initialLoRaBW").doubleValue());
        loRaCR = par("initialLoRaCR");
        loRaUseHeader = par("initialUseHeader");
        evaluateADRinNode = par("evaluateADRinNode");
        numberOfPacketsToSend = par("numberOfPacketsToSend");
        radioGate = gate("appOutRadio");
        transducerGate = gate("appOut");

        mob = check_and_cast<IMobility *>(findContainingNode(this)->getSubmodule("mobility"));
        radio = check_and_cast<physicallayer::IRadio *>(radioGate->getPathEndGate()->getOwnerModule()->getParentModule()->getSubmodule("radio"));
        transducer = check_and_cast<physicallayer::IRadio *>(transducerGate->getPathEndGate()->getOwnerModule()->getParentModule()->getSubmodule("radio"));
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
        unsigned int maxTry = 100;
        do {
            timeToFirstPacket = par("timeToFirstPacket");
            EV << "Wylosowalem czas :" << timeToFirstPacket << endl;
            maxTry--;
            if (maxTry == 0)
                throw cRuntimeError("time to first packet packet must be grater than %f", par("maxTimeToFirstPacket").doubleValue());
            //if(timeToNextPacket < 5) error("Time to next packet must be grater than 3");
        } while(timeToFirstPacket < par("maxTimeToFirstPacket").doubleValue());

        backoffTimer = new BackoffMsgTimer("BackoffTimer");
        numberOfPacketsToSend = par("numberOfPacketsToSend");
        backoffDelay = &par("backoffDelay");
        minimumDistance = m(par("minimumDistance").doubleValue());
        sendMeasurements = new cMessage("sendMeasurements");
        if(numberOfPacketsToSend == -1 || sentPackets < numberOfPacketsToSend)
            scheduleAt(simTime() + timeToFirstPacket, sendMeasurements);
        UanAppPacketSent = registerSignal("UanAppPacketSent");
        auto transmitter = check_and_cast<const UanTransmitter*>(transducer->getTransmitter());
        dataBitrate = transmitter->getBitrate();
    }
}


void SimpleAppBuoy::finish()
{
    cModule *host = getContainingNode(this);
    auto mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
    Coord coord = mobility->getCurrentPosition();
    recordScalar("sentPackets", sentPackets);
    recordScalar("receivedADRCommands", receivedADRCommands);
}

void SimpleAppBuoy::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == sendMeasurements) {
            auto tPacket = sendPacket();
            if (simTime() >= getSimulation()->getWarmupPeriod())
                sentPackets++;
            if(numberOfPacketsToSend == -1 || sentPackets < numberOfPacketsToSend)
            {
                scheduleAt(simTime() + tPacket + backoffDelay->doubleValue(), sendMeasurements);
            }
            else
                delete msg;
        }
        else if (msg == backoffTimer) {

        }
        else {

        }
    }
    else if (msg->getArrivalGate() == radioGate) {
        handleMessageFromLowerLayerRadio(msg);
    }
    else {
        handleMessageFromLowerLayer(msg);
        delete msg;
        //cancelAndDelete(sendMeasurements);
        //sendMeasurements = new cMessage("sendMeasurements");
        //scheduleAt(simTime(), sendMeasurements);
    }
}

void SimpleAppBuoy::handleMessageFromLowerLayer(cMessage *msg)
{
    auto pkt = check_and_cast<Packet *>(msg);
    const auto & packet = pkt->peekAtFront<AppPacket>();
    if (packet == nullptr)
        throw cRuntimeError("No AppPAcket header found");
    if (simTime() >= getSimulation()->getWarmupPeriod())
        receivedADRCommands++;
}

void SimpleAppBuoy::handleMessageFromLowerLayerRadio(cMessage *msg)
{
    auto pkt = check_and_cast<Packet *>(msg);
    const auto & header = pkt->peekAtFront<AppPacketInformation>();
    if (header == nullptr)
        throw cRuntimeError("No AppPacketInformation header found");
    auto distance = mob->getCurrentPosition().distance(header->getPosition());
    if (distance < minimumDistance.get()) {
        // compute time
        simtime_t t = distance/1500 + header->getTransmissionTime() + header->getTime();
        // create backoff time
        if (backoffTimer->isScheduled()) {
            // check if it is necessary to reschedule the timer.
            if (backoffTimer->getArrivalTime() < t) {
                cancelEvent(backoffTimer);
                scheduleAt(t, backoffTimer);
            }
            return;
        }
        // schedule backoff
        if (!sendMeasurements->isScheduled())
            throw cRuntimeError("Error, sendMeasurements is not schedule and should be ");
        simtime_t remain = sendMeasurements->getArrivalTime() - simTime(); // remain time
        backoffTimer->setRemainTime(remain);
        scheduleAt(t, backoffTimer);
        cancelEvent(sendMeasurements);
    }
}

bool SimpleAppBuoy::handleOperationStage(LifecycleOperation *operation, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();

    throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

simtime_t SimpleAppBuoy::sendPacket()
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
    //pktRequest->addTagIfAbsent<SignalBitrateReq>()->setDataBitrate(dataBitrate);
    simtime_t transmissionTime = (double) pktRequest->getBitLength()/dataBitrate.get();
    auto infoHeader = makeShared<AppPacketInformation>();
    infoHeader->setPosition(mob->getCurrentPosition());
    infoHeader->setTransmissionTime(transmissionTime);
    infoHeader->setTime(simTime());
    infoHeader->setChunkLength(b(400)); // set the length
    auto pktInfo = new Packet("InfoFrame");
    pktInfo->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::lora);
    pktInfo->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);

    pktInfo->setKind(DATA);
    pktInfo->insertAtBack(infoHeader);

    send(pktInfo, radioGate); // send the advertisement packet
    send(pktRequest, transducerGate);
    return transmissionTime;
}

} //end namespace inet
}
