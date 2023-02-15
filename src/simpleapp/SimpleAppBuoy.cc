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
#include "inet/mobility/contract/IMobility.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/flora/lorabase/LoRaTagInfo_m.h"

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
        radioGateOut = gate("appOutRadio");
        transducerGateOut = gate("appOut");
        radioGateIn = gate("appInRadio");
        transducerGateIn = gate("appIn");

        mob = check_and_cast<IMobility *>(findContainingNode(this)->getSubmodule("mobility"));
        radio = check_and_cast<physicallayer::IRadio *>(radioGateOut->getPathEndGate()->getOwnerModule()->getParentModule()->getSubmodule("radio"));
        transducer = check_and_cast<physicallayer::IRadio *>(transducerGateOut->getPathEndGate()->getOwnerModule()->getParentModule()->getSubmodule("radio"));
        uanMacModule =  check_and_cast<UanIMac *>(transducerGateOut->getPathEndGate()->getOwnerModule()->getParentModule()->getSubmodule("mac"));
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
        uanTranmitter = check_and_cast<const UanTransmitter*>(transducer->getTransmitter());
        dataBitrate = uanTranmitter->getBitrate();
        cModule *radioModule = check_and_cast<cModule *>(radio);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);

        WATCH(loRaTP);
        WATCH(loRaCF);
        WATCH(loRaSF);
        WATCH(loRaBW);
        WATCH(loRaCR);
    }
}


void SimpleAppBuoy::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));
    if (signalID == IRadio::receptionStateChangedSignal) {
        if (radio->getReceptionState() == IRadio::RECEPTION_STATE_RECEIVING) {
            // Stop timers
            if (backoffTimer && backoffTimer->isScheduled()) {
                Timers t;
                t.timer = backoffTimer;
                t.remain = backoffTimer->getArrivalTime() - simTime();
                t.arrival = backoffTimer->getArrivalTime();
                cancelEvent(t.timer);
                pendingTimers.push_back(t);
            }
            if (configureLoRaParameters && configureLoRaParameters->isScheduled()){
                Timers t;
                t.timer = configureLoRaParameters;
                t.remain = configureLoRaParameters->getArrivalTime() - simTime();
                t.arrival = backoffTimer->getArrivalTime();
                cancelEvent(t.timer);
                pendingTimers.push_back(t);
            }
            if (sendMeasurements && sendMeasurements->isScheduled()) {
                Timers t;
                t.timer = sendMeasurements;
                t.remain = sendMeasurements->getArrivalTime() - simTime();
                t.arrival = backoffTimer->getArrivalTime();
                cancelEvent(t.timer);
                pendingTimers.push_back(t);
            }
        }
        else if (radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE) {
            // Restart timers
            for (const auto &elem : pendingTimers) {
                scheduleAfter(elem.remain, elem.timer);
            }
            pendingTimers.clear();
        }
    }
}


void SimpleAppBuoy::finish()
{
    //cModule *host = getContainingNode(this);
    //auto mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
    //Coord coord = mobility->getCurrentPosition();
    recordScalar("sentPackets", sentPackets);
    recordScalar("recPackets", recPackets);
}

void SimpleAppBuoy::sendInfoFrame(const int &type,const simtime_t &time, const simtime_t &transmissionTime, const b &size)
{
    auto infoHeader = makeShared<AppPacketInformation>();
    infoHeader->setPosition(mob->getCurrentPosition());
    infoHeader->setTransmissionTime(transmissionTime);
    infoHeader->setTime(time);
    infoHeader->setChunkLength(size); // set the length
    infoHeader->setType(type);

    auto pktInfo = new Packet("InfoFrame");
    pktInfo->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::lora);
    pktInfo->addTagIfAbsent<MacAddressReq>()->setDestAddress(MacAddress::BROADCAST_ADDRESS);
    auto loraTag = pktInfo->addTagIfAbsent<flora::LoRaTag>();
    loraTag->setBandwidth(loRaBW);
    loraTag->setCenterFrequency(loRaCF);
    loraTag->setSpreadFactor(loRaSF);
    loraTag->setCodeRendundance(loRaCR);
    loraTag->setPower(mW(math::dBmW2mW(loRaTP)));
    pktInfo->setKind(DATA);
    pktInfo->insertAtBack(infoHeader);
    send(pktInfo, radioGateOut); // send the advertisement packet
}


void SimpleAppBuoy::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        if (msg == sendMeasurements) {
            auto tPacket = sendPacket() + minimumDistance.get()/1500; // transmission time more maximum propagation time
            if (simTime() >= getSimulation()->getWarmupPeriod())
                sentPackets++;
            auto timeToNextPacket = tPacket + backoffDelay->doubleValue() + 2 * par("guardTime").doubleValue();
            if(numberOfPacketsToSend == -1 || sentPackets < numberOfPacketsToSend)
            {
                // after send a packet, the node must sleep
                scheduleAfter(timeToNextPacket, sendMeasurements);
            }
            else
                delete msg;
        }
        else if (msg == backoffTimer) {
            // TODO: Check radio and transceiver state if they are in receiving state, the radio should delay the transmission
            backoffTimer->setFailures(0);
            scheduleAt(simTime(), sendMeasurements); // it is no very efficient but it is simple
        }
        else {
            throw cRuntimeError("Unknown timer");
        }
    }
    else {
        auto arrivalGate = msg->getArrivalGate();
        if (arrivalGate == radioGateIn) {
            handleMessageFromLowerLayerRadio(msg);
        }
        else if (arrivalGate == transducerGateIn){
            handleMessageFromLowerLayer(msg);
            //cancelAndDelete(sendMeasurements);
            //sendMeasurements = new cMessage("sendMeasurements");
            //scheduleAt(simTime(), sendMeasurements);
            }
        else
            throw cRuntimeError("Unknown gate");
        delete msg;
    }
}

void SimpleAppBuoy::handleMessageFromLowerLayer(cMessage *msg)
{
    auto pkt = check_and_cast<Packet *>(msg);
    const auto & packet = pkt->peekAtFront<AppPacket>();
    if (packet == nullptr)
        throw cRuntimeError("No AppPAcket header found");
    if (simTime() >= getSimulation()->getWarmupPeriod())
        recPackets++;
}

void SimpleAppBuoy::handleMessageFromLowerLayerRadio(cMessage *msg)
{
    auto pkt = check_and_cast<Packet *>(msg);
    const auto & header = pkt->peekAtFront<AppPacketInformation>();
    if (header == nullptr)
        throw cRuntimeError("No AppPacketInformation header found");
    auto distance = mob->getCurrentPosition().distance(header->getPosition());
    if (distance < minimumDistance.get()) {
        if (header->getType() == INFORMATIOM) {
            // compute time
            simtime_t propagation = distance / 1500.0;
            simtime_t delay = propagation + header->getTransmissionTime() + par("guardTime");
            simtime_t t = header->getTime() + delay;
            // create backoff time
            if (!pendingTimers.empty())
                throw cRuntimeError("Why are pending");
            if (backoffTimer->isScheduled()) {
                // check if it is necessary to reschedule the timer.
                if (backoffTimer->getArrivalTime() < t) {
                    cancelEvent(backoffTimer);
                    backoffTimer->setFailures(backoffTimer->getFailures() + 1);
                    if (backoffTimer->getFailures() > par("maxBackoffFailures").intValue()) {
                        sendInfoFrame(REQUEST, simTime(), SimTime::ZERO, b(100));
                    }
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
        else if (header->getType() == REQUEST) {
            // A node request access, it is necessary delay the access to the channel
            if (sendMeasurements->isScheduled()) {
                simtime_t propagation = minimumDistance.get() / 1500.0;
                simtime_t newArrival = sendMeasurements->getArrivalTime() + backoffDelay->doubleValue() + propagation;
                cancelEvent(sendMeasurements);
                backoffTimer->setFailures(backoffTimer->getFailures() + 1);
                scheduleAt(newArrival, sendMeasurements);
            }
            else if (backoffTimer->isScheduled()) {
                simtime_t propagation = minimumDistance.get() / 1500.0;
                simtime_t newArrival = backoffTimer->getArrivalTime() + backoffDelay->doubleValue() + propagation;
                cancelEvent(backoffTimer);
                backoffTimer->setFailures(backoffTimer->getFailures() + 1);
                scheduleAt(newArrival, backoffTimer);
            }
        }
        else
            throw cRuntimeError("Not allowed type");
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
    // Total size of the physical packet.
    b size = uanMacModule->getHeaderLength() + b(pktRequest->getBitLength()) + uanTranmitter->getHeaderLength();
    simtime_t transmissionTime =  b(size).get()/bps(dataBitrate).get() + uanTranmitter->getPreambleDuration() + par("guardTime"); // small guard time

    sendInfoFrame(INFORMATIOM, simTime() + par("guardTime"), transmissionTime, b(100));
    sendDelayed(pktRequest,0.0000001, transducerGateOut);
    return transmissionTime;
}

} //end namespace inet
}
