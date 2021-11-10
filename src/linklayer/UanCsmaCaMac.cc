//
// Copyright (C) 2016 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "UanCsmaCaMac.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

namespace inet {
namespace Uan {



Define_Module(UanCsmaCaMac);

void UanCsmaCaMac::finish()
{
    CsmaCaMac::finish();
}

void UanCsmaCaMac::receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details)
{
    CsmaCaMac::receiveSignal(source,signalID,value,details);
}

/****************************************************************
 * Initialization functions.
 */
void UanCsmaCaMac::initialize(int stage)
{
    MacProtocolBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        EV << "Initializing stage 0\n";
        fcsMode = parseFcsMode(par("fcsMode"));
        useAck = par("useAck");
        bitrate = par("bitrate");
        headerLength = B(par("headerLength"));
        if (headerLength > B(255))
            throw cRuntimeError("The specified headerLength is too large");

        ackLength = B(par("ackLength"));
        if (ackLength > B(255))
            throw cRuntimeError("The specified ackLength is too large");

        ackTimeout = par("ackTimeout");
        slotTime = par("slotTime");
        sifsTime = par("sifsTime");
        difsTime = par("difsTime");
        cwMin = par("cwMin");
        cwMax = par("cwMax");
        cwMulticast = par("cwMulticast");
        retryLimit = par("retryLimit");

        // initialize self messages
        endSifs = new cMessage("SIFS");
        endDifs = new cMessage("DIFS");
        endBackoff = new cMessage("Backoff");
        endAckTimeout = new cMessage("AckTimeout");
        endData = new cMessage("Data");
        mediumStateChange = new cMessage("MediumStateChange");

        // set up internal queue
        txQueue = check_and_cast<queueing::IPacketQueue *>(getSubmodule("queue"));

        // state variables
        fsm.setName("CsmaCaMac State Machine");
        backoffPeriod = -1;
        retryCounter = 0;

        // statistics
        numRetry = 0;
        numSentWithoutRetry = 0;
        numGivenUp = 0;
        numCollision = 0;
        numSent = 0;
        numReceived = 0;
        numSentBroadcast = 0;
        numReceivedBroadcast = 0;

        // initialize watches
        WATCH(fsm);
        WATCH(backoffPeriod);
        WATCH(retryCounter);
        WATCH(numRetry);
        WATCH(numSentWithoutRetry);
        WATCH(numGivenUp);
        WATCH(numCollision);
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numSentBroadcast);
        WATCH(numReceivedBroadcast);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {
        // subscribe for the information of the carrier sense
        radio.reference(this, "radioModule", true);
        cModule *radioModule = check_and_cast<cModule *>(radio.get());
        radioModule->subscribe(physicallayer::IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(physicallayer::IRadio::transmissionStateChangedSignal, this);
        radio->setRadioMode(physicallayer::IRadio::RADIO_MODE_RECEIVER);
    }
}


void UanCsmaCaMac::handleUpperPacket(Packet *packet)
{
    auto frame = check_and_cast<Packet *>(packet);
    auto dest = packet->getTag<MacAddressReq>()->getDestAddress();
    if (dest == networkInterface->getMacAddress()) {
        delete packet;
        return;
    }

    encapsulate(frame);
    const auto& macHeader = frame->peekAtFront<CsmaCaMacHeader>();
    EV << "frame " << frame << " received from higher layer, receiver = " << macHeader->getReceiverAddress() << endl;
    ASSERT(!macHeader->getReceiverAddress().isUnspecified());
    txQueue->enqueuePacket(frame);
    if (fsm.getState() != IDLE)
        EV << "deferring upper message transmission in " << fsm.getStateName() << " state\n";
    else if (!txQueue->isEmpty()) {
        popTxQueue();
        handleWithFsm(currentTxFrame);
    }
}

}

} // namespace inet

