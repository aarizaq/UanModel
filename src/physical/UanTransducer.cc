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

//#include "UanPreamble_m.h"
#include "UanTagInfo_m.h"
#include "UanTransducer.h"
#include "UanPhyPreamble_m.h"
#include "UanScalarBackgroundNoise.h"
#include "UanReceiver.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/SignalTag_m.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarTransmission.h"
#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"

//#include "LoRaMacFrame_m.h"

namespace inet {
namespace Uan {


Define_Module(UanTransducer);

simsignal_t UanTransducer::minSNIRSignal = cComponent::registerSignal("minSNIR");
simsignal_t UanTransducer::packetErrorRateSignal = cComponent::registerSignal("packetErrorRate");
simsignal_t UanTransducer::bitErrorRateSignal = cComponent::registerSignal("bitErrorRate");
simsignal_t UanTransducer::symbolErrorRateSignal = cComponent::registerSignal("symbolErrorRate");
simsignal_t UanTransducer::droppedPacket = cComponent::registerSignal("droppedPacket");

void UanTransducer::initialize(int stage)
{
    PhysicalLayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        switchTimer = new cMessage("switchTimer");
        transmissionTimer = new cMessage("transmissionTimer");
        antenna = check_and_cast<IAntenna *>(getSubmodule("antenna"));
        transmitter = check_and_cast<ITransmitter *>(getSubmodule("transmitter"));
        receiver = check_and_cast<IReceiver *>(getSubmodule("receiver"));
        medium.reference(this, "soundMediumModule", true);
        mediumModuleId = check_and_cast<cModule *>(medium.get())->getId();
        upperLayerIn = gate("upperLayerIn");
        upperLayerOut = gate("upperLayerOut");
        radioIn = gate("radioIn");
        radioIn->setDeliverImmediately(true);
        sendRawBytes = par("sendRawBytes");
        separateTransmissionParts = par("separateTransmissionParts");
        separateReceptionParts = par("separateReceptionParts");
        WATCH(radioMode);
        WATCH(receptionState);
        WATCH(transmissionState);
        WATCH(receivedSignalPart);
        WATCH(transmittedSignalPart);
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER) {
        medium->addRadio(this);
        if (medium->getCommunicationCache()->getNumTransmissions() == 0 && isListeningPossible()) {
            auto noise = check_and_cast<const UanScalarBackgroundNoise *>(medium->getBackgroundNoise());
            auto rec = check_and_cast<const UanReceiver *>(receiver);
            auto nos = noise->getNoiseReference(rec->getCenterFrequency());
            auto ener = rec->getEnergyDetection();
            throw cRuntimeError("Receiver is busy without any ongoing transmission, probably energy detection level is too low or background noise level is too high Noise: %f Energy detection %f", nos.get(), ener.get());
        }
        initializeRadioMode();
        parseRadioModeSwitchingTimes();
    }
    else if (stage == INITSTAGE_LAST) {
        EV_INFO << "Initialized " << getCompleteStringRepresentation() << endl;
    }
}

void UanTransducer::parseRadioModeSwitchingTimes()
{
    const char *times = par("switchingTimes");

    char prefix[3];
    unsigned int count = sscanf(times, "%s", prefix);

    if (count > 2)
        throw cRuntimeError("Metric prefix should be no more than two characters long");

    double metric = 1;

    if (strcmp("s", prefix) == 0)
        metric = 1;
    else if (strcmp("ms", prefix) == 0)
        metric = 0.001;
    else if (strcmp("ns", prefix) == 0)
        metric = 0.000000001;
    else
        throw cRuntimeError("Undefined or missed metric prefix for switchingTimes parameter");

    cStringTokenizer tok(times + count + 1);
    unsigned int idx = 0;
    while (tok.hasMoreTokens()) {
        switchingTimes[idx / RADIO_MODE_SWITCHING][idx % RADIO_MODE_SWITCHING] = atof(tok.nextToken()) * metric;
        idx++;
    }
    if (idx != RADIO_MODE_SWITCHING * RADIO_MODE_SWITCHING)
        throw cRuntimeError("Check your switchingTimes parameter! Some parameters may be missed");
}


UanTransducer::~UanTransducer() {
}

std::ostream& UanTransducer::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << static_cast<const cSimpleModule *>(this);
    if (level <= PRINT_LEVEL_TRACE)
        stream << ", antenna = " << printFieldToString(antenna, level + 1, evFlags)
               << ", transmitter = " << printFieldToString(transmitter, level + 1, evFlags)
               << ", receiver = " << printFieldToString(receiver, level + 1, evFlags);
    return stream;
}


const ITransmission *UanTransducer::getTransmissionInProgress() const
{
    if (!transmissionTimer->isScheduled())
        return nullptr;
    else
        return static_cast<WirelessSignal *>(transmissionTimer->getContextPointer())->getTransmission();
}

const ITransmission *UanTransducer::getReceptionInProgress() const
{
    if (receptionTimer == nullptr)
        return nullptr;
    else
        return static_cast<WirelessSignal *>(receptionTimer->getControlInfo())->getTransmission();
}

IRadioSignal::SignalPart UanTransducer::getTransmittedSignalPart() const
{
    return transmittedSignalPart;
}

IRadioSignal::SignalPart UanTransducer::getReceivedSignalPart() const
{
    return receivedSignalPart;
}

void UanTransducer::handleMessageWhenDown(cMessage *message)
{
    if (message->getArrivalGate() == radioIn || isReceptionTimer(message))
        delete message;
    else
        OperationalBase::handleMessageWhenDown(message);
}

void UanTransducer::handleMessageWhenUp(cMessage *message)
{
    if (message->isSelfMessage())
        handleSelfMessage(message);
    else if (message->getArrivalGate() == upperLayerIn) {
        if (!message->isPacket()) {
            handleUpperCommand(message);
            delete message;
        }
        else
            handleUpperPacket(check_and_cast<Packet *>(message));
    }
    else if (message->getArrivalGate() == radioIn) {
        if (!message->isPacket()) {
            handleLowerCommand(message);
            delete message;
        }
        else
            handleSignal(check_and_cast<WirelessSignal *>(message));
    }
    else
        throw cRuntimeError("Unknown arrival gate '%s'.", message->getArrivalGate()->getFullName());
}

void UanTransducer::handleSelfMessage(cMessage *message)
{
    FlatRadioBase::handleSelfMessage(message);
}

void UanTransducer::handleTransmissionTimer(cMessage *message)
{
    if (message->getKind() == IRadioSignal::SIGNAL_PART_WHOLE)
        endTransmission();
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_PREAMBLE)
        continueTransmission();
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_HEADER)
        continueTransmission();
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_DATA)
        endTransmission();
    else
        throw cRuntimeError("Unknown self message");
}

void UanTransducer::handleReceptionTimer(cMessage *message)
{
    if (message->getKind() == IRadioSignal::SIGNAL_PART_WHOLE)
        endReception(message);
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_PREAMBLE)
        continueReception(message);
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_HEADER)
        continueReception(message);
    else if (message->getKind() == IRadioSignal::SIGNAL_PART_DATA)
        endReception(message);
    else
        throw cRuntimeError("Unknown self message");
}

void UanTransducer::handleUpperCommand(cMessage *message)
{
    if (message->getKind() == RADIO_C_CONFIGURE) {
        ConfigureRadioCommand *configureCommand = check_and_cast<ConfigureRadioCommand *>(message->getControlInfo());
        if (configureCommand->getRadioMode() != -1)
            setRadioMode((RadioMode)configureCommand->getRadioMode());
    }
    else
        throw cRuntimeError("Unsupported command");
}

void UanTransducer::handleLowerCommand(cMessage *message)
{
    throw cRuntimeError("Unsupported command");
}

void UanTransducer::handleUpperPacket(Packet *packet)
{
    emit(packetReceivedFromUpperSignal, packet);
    if (isTransmitterMode(radioMode)) {
        auto tag = packet->removeTagIfPresent<UanTag>();


        if (transmissionTimer->isScheduled())
            throw cRuntimeError("Received frame from upper layer while already transmitting.");
        if (separateTransmissionParts)
            startTransmission(packet, IRadioSignal::SIGNAL_PART_PREAMBLE);
        else
            startTransmission(packet, IRadioSignal::SIGNAL_PART_WHOLE);
    }
    else {
        EV_ERROR << "Radio is not in transmitter or transceiver mode, dropping frame." << endl;
        delete packet;
    }
}

void UanTransducer::handleSignal(WirelessSignal *radioFrame)
{
    auto receptionTimer = createReceptionTimer(radioFrame);
    if (separateReceptionParts)
        startReception(receptionTimer, IRadioSignal::SIGNAL_PART_PREAMBLE);
    else
        startReception(receptionTimer, IRadioSignal::SIGNAL_PART_WHOLE);
}

/*
bool UanTransducer::handleNodeStart(IDoneCallback *doneCallback)
{
    // NOTE: we ignore radio mode switching during start
    completeRadioModeSwitch(RADIO_MODE_OFF);
    return PhysicalLayerBase::handleNodeStart(doneCallback);
}
bool UanTransducer::handleNodeShutdown(IDoneCallback *doneCallback)
{
    // NOTE: we ignore radio mode switching and ongoing transmission during shutdown
    cancelEvent(switchTimer);
    if (transmissionTimer->isScheduled())
        abortTransmission();
    completeRadioModeSwitch(RADIO_MODE_OFF);
    return PhysicalLayerBase::handleNodeShutdown(doneCallback);
}
void UanTransducer::handleNodeCrash()
{
    cancelEvent(switchTimer);
    if (transmissionTimer->isScheduled())
        abortTransmission();
    completeRadioModeSwitch(RADIO_MODE_OFF);
    PhysicalLayerBase::handleNodeCrash();
}
*/

void UanTransducer::startTransmission(Packet *macFrame, IRadioSignal::SignalPart part)
{
    FlatRadioBase::startTransmission(macFrame, part);
   /* auto radioFrame = createSignal(macFrame);
    auto transmission = radioFrame->getTransmission();
    transmissionTimer->setKind(part);
    transmissionTimer->setContextPointer(const_cast<Signal *>(radioFrame));
    scheduleAt(transmission->getEndTime(part), transmissionTimer);
    EV_INFO << "Transmission started: " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(part) << " as " << transmission << endl;
    updateTransceiverState();
    updateTransceiverPart();
    emit(transmissionStartedSignal, check_and_cast<const cObject *>(transmission));
    //check_and_cast<LoRaMedium *>(medium)->fireTransmissionStarted(transmission);
    //check_and_cast<LoRaMedium *>(medium)->emit(IRadioMedium::transmissionStartedSignal, check_and_cast<const cObject *>(transmission));
    //check_and_cast<RadioMedium *>(medium)->emit(transmissionStartedSignal, check_and_cast<const cObject *>(transmission));
    check_and_cast<RadioMedium *>(medium)->emit(IRadioMedium::signalDepartureStartedSignal, check_and_cast<const cObject *>(transmission));
    */
}

void UanTransducer::continueTransmission()
{
    FlatRadioBase::continueTransmission();
    /*
    auto previousPart = (IRadioSignal::SignalPart)transmissionTimer->getKind();
    auto nextPart = (IRadioSignal::SignalPart)(previousPart + 1);
    auto radioFrame = static_cast<Signal *>(transmissionTimer->getContextPointer());
    auto transmission = radioFrame->getTransmission();
    EV_INFO << "Transmission ended: " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(previousPart) << " as " << radioFrame->getTransmission() << endl;
    transmissionTimer->setKind(nextPart);
    scheduleAt(transmission->getEndTime(nextPart), transmissionTimer);
    EV_INFO << "Transmission started: " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(nextPart) << " as " << transmission << endl;
    updateTransceiverState();
    updateTransceiverPart();
    */
}

void UanTransducer::endTransmission()
{
    FlatRadioBase::endTransmission();
    /*
    auto part = (IRadioSignal::SignalPart)transmissionTimer->getKind();
    auto radioFrame = static_cast<Signal *>(transmissionTimer->getContextPointer());
    auto transmission = radioFrame->getTransmission();
    transmissionTimer->setContextPointer(nullptr);
    EV_INFO << "Transmission ended: " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(part) << " as " << transmission << endl;
    updateTransceiverState();
    updateTransceiverPart();
    //check_and_cast<LoRaMedium *>(medium)->fireTransmissionEnded(transmission);
    //check_and_cast<RadioMedium *>(medium)->emit(transmissionEndedSignal, check_and_cast<const cObject *>(transmission));
    emit(transmissionEndedSignal, check_and_cast<const cObject *>(transmission));
    // TODO: move to radio medium
    check_and_cast<LoRaMedium *>(medium)->emit(IRadioMedium::signalDepartureEndedSignal, check_and_cast<const cObject *>(transmission));
    */

}

void UanTransducer::abortTransmission()
{
    FlatRadioBase::abortTransmission();
 /*   auto part = (IRadioSignal::SignalPart)transmissionTimer->getKind();
    auto radioFrame = static_cast<Signal *>(transmissionTimer->getContextPointer());
    auto transmission = radioFrame->getTransmission();
    transmissionTimer->setContextPointer(nullptr);
    EV_INFO << "Transmission aborted: " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(part) << " as " << transmission << endl;
    EV_WARN << "Aborting ongoing transmissions is not supported" << endl;
    cancelEvent(transmissionTimer);
    updateTransceiverState();
    updateTransceiverPart();*/
}

WirelessSignal *UanTransducer::createSignal(Packet *packet) const
{
    return FlatRadioBase::createSignal(packet);
    /*
    Signal *radioFrame = check_and_cast<Signal *>(medium->transmitPacket(this, packet));
    ASSERT(radioFrame->getDuration() != 0);
    return radioFrame;
    */
}

void UanTransducer::startReception(cMessage *timer, IRadioSignal::SignalPart part)
{
    FlatRadioBase::startReception(timer, part);
/*    auto signal = static_cast<Signal *>(timer->getControlInfo());
    auto arrival = signal->getArrival();
    auto reception = signal->getReception();
// TODO: should be this, but it breaks fingerprints: if (receptionTimer == nullptr && isReceiverMode(radioMode) && arrival->getStartTime(part) == simTime()) {
    if (isReceiverMode(radioMode) && arrival->getStartTime(part) == simTime()) {
        auto transmission = signal->getTransmission();
        auto isReceptionAttempted = medium->isReceptionAttempted(this, transmission, part);
        EV_INFO << "Reception started: " << (isReceptionAttempted ? "attempting" : "not attempting") << " " << (ISignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
        if (isReceptionAttempted)
        {
            receptionTimer = timer;
            emit(receptionStartedSignal, check_and_cast<const cObject *>(reception));
        }
    }
    else
        EV_INFO << "Reception started: ignoring " << (ISignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
    timer->setKind(part);
    scheduleAt(arrival->getEndTime(part), timer);
    updateTransceiverState();
    updateTransceiverPart();
    //check_and_cast<LoRaMedium *>(medium)->fireReceptionStarted(reception);
    //check_and_cast<RadioMedium *>(medium)->emit(receptionStartedSignal, check_and_cast<const cObject *>(reception));
    check_and_cast<LoRaMedium *>(medium)->emit(IRadioMedium::signalArrivalStartedSignal, check_and_cast<const cObject *>(reception));
    */
}

void UanTransducer::continueReception(cMessage *timer)
{
    FlatRadioBase::continueReception(timer);
 /*   auto previousPart = (IRadioSignal::SignalPart)timer->getKind();
    auto nextPart = (IRadioSignal::SignalPart)(previousPart + 1);
    auto radioFrame = static_cast<Signal *>(timer->getControlInfo());
    auto arrival = radioFrame->getArrival();
    auto reception = radioFrame->getReception();
    if (timer == receptionTimer && isReceiverMode(radioMode) && arrival->getEndTime(previousPart) == simTime()) {
        auto transmission = radioFrame->getTransmission();
        bool isReceptionSuccessful = medium->isReceptionSuccessful(this, transmission, previousPart);
        EV_INFO << "Reception ended: " << (isReceptionSuccessful ? "successfully" : "unsuccessfully") << " for " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(previousPart) << " as " << reception << endl;
        if (!isReceptionSuccessful)
            receptionTimer = nullptr;
        auto isReceptionAttempted = medium->isReceptionAttempted(this, transmission, nextPart);
        EV_INFO << "Reception started: " << (isReceptionAttempted ? "attempting" : "not attempting") << " " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(nextPart) << " as " << reception << endl;
        if (!isReceptionAttempted)
            receptionTimer = nullptr;
    }
    else {
        EV_INFO << "Reception ended: ignoring " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(previousPart) << " as " << reception << endl;
        EV_INFO << "Reception started: ignoring " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(nextPart) << " as " << reception << endl;
    }
    timer->setKind(nextPart);
    scheduleAt(arrival->getEndTime(nextPart), timer);
    updateTransceiverState();
    updateTransceiverPart();
    */
}

void UanTransducer::decapsulate(Packet *packet) const
{
    auto chunk = packet->peekAtFront<Chunk>();
    auto preamble = dynamicPtrCast<const UanPhyPreamble>(chunk);
    if (preamble != nullptr) {
        packet->popAtFront<UanPhyPreamble>();
    }
    return;
}

void UanTransducer::endReception(cMessage *timer)
{

    auto part = (IRadioSignal::SignalPart)timer->getKind();
    auto signal = static_cast<WirelessSignal *>(timer->getControlInfo());
    auto arrival = signal->getArrival();
    auto reception = signal->getReception();
    if (timer == receptionTimer && isReceiverMode(radioMode) && arrival->getEndTime() == simTime()) {
        auto transmission = check_and_cast<const ScalarTransmission *> (signal->getTransmission());

        // TODO: this would draw twice from the random number generator in isReceptionSuccessful: auto isReceptionSuccessful = medium->isReceptionSuccessful(this, transmission, part);
        auto isReceptionSuccessful = medium->getReceptionDecision(this, signal->getListening(), transmission, part)->isReceptionSuccessful();
        EV_INFO << "Reception ended: " << (isReceptionSuccessful ? "\x1b[1msuccessfully\x1b[0m" : "\x1b[1munsuccessfully\x1b[0m") << " for " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
        auto macFrame = medium->receivePacket(this, signal);
        take(macFrame);
        if (isReceptionSuccessful) {
            decapsulate(macFrame);
            auto tag = macFrame->addTag<UanTag>();
            //auto preamble = packet->popAtFront<LoRaPhyPreamble>();
            tag->setBandwidth(transmission->getBandwidth());
            tag->setCenterFrequency(transmission->getCenterFrequency());
            tag->setPower(transmission->getPower());
            sendUp(macFrame);
        }
        else {
            emit(UanTransducer::droppedPacket, 0);
            delete macFrame;
        }
        receptionTimer = nullptr;
        emit(receptionEndedSignal, check_and_cast<const cObject *>(reception));
    }
    else
        EV_INFO << "Reception ended: \x1b[1mignoring\x1b[0m " << (IWirelessSignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
    updateTransceiverState();
    updateTransceiverPart();
    delete timer;
    // TODO: move to radio medium
    check_and_cast<RadioMedium *>(medium.get())->emit(IRadioMedium::signalArrivalEndedSignal, check_and_cast<const cObject *>(reception));
/*
    auto part = (IRadioSignal::SignalPart)timer->getKind();
    auto signal = static_cast<Signal *>(timer->getControlInfo());
    auto arrival = signal->getArrival();
    auto reception = signal->getReception();
    if (timer == receptionTimer && isReceiverMode(radioMode) && arrival->getEndTime() == simTime()) {
    //if (isReceiverMode(radioMode) && arrival->getEndTime() == simTime()) {
        auto transmission = signal->getTransmission();
// TODO: this would draw twice from the random number generator in isReceptionSuccessful: auto isReceptionSuccessful = medium->isReceptionSuccessful(this, transmission, part);
        auto isReceptionSuccessful = medium->getReceptionDecision(this, signal->getListening(), transmission, part)->isReceptionSuccessful();
        EV_INFO << "Reception ended: " << (isReceptionSuccessful ? "successfully" : "unsuccessfully") << " for " << (ISignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
        auto macFrame = medium->receivePacket(this, signal);
        auto tag = macFrame->addTag<flora::LoRaTag>();
        auto preamble = macFrame->popAtFront<LoRaPhyPreamble>();
        tag->setBandwidth(preamble->getBandwidth());
        tag->setCenterFrequency(preamble->getCenterFrequency());
        tag->setCodeRendundance(preamble->getCodeRendundance());
        tag->setPower(preamble->getPower());
        tag->setSpreadFactor(preamble->getSpreadFactor());
        tag->setUseHeader(preamble->getUseHeader());
        if(isReceptionSuccessful) {
            emit(packetSentToUpperSignal, macFrame);
            sendUp(macFrame);
        }
        else {
            emit(UanTransducer::droppedPacket, 0);
            delete macFrame;
        }
        emit(receptionEndedSignal, check_and_cast<const cObject *>(reception));
        receptionTimer = nullptr;
    }
    else
        EV_INFO << "Reception ended: ignoring " << (ISignal *)signal << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
    updateTransceiverState();
    updateTransceiverPart();
    //check_and_cast<LoRaMedium *>(medium)->fireReceptionEnded(reception);
    //check_and_cast<RadioMedium *>(medium)->emit(receptionEndedSignal, check_and_cast<const cObject *>(reception));
    check_and_cast<LoRaMedium *>(medium)->emit(IRadioMedium::signalArrivalEndedSignal, check_and_cast<const cObject *>(reception));
    delete timer;
    */
}

void UanTransducer::abortReception(cMessage *timer)
{
    FlatRadioBase::abortReception(timer);
  /*  auto radioFrame = static_cast<Signal *>(timer->getControlInfo());
    auto part = (IRadioSignal::SignalPart)timer->getKind();
    auto reception = radioFrame->getReception();
    EV_INFO << "Reception aborted: for " << (ISignal *)radioFrame << " " << IRadioSignal::getSignalPartName(part) << " as " << reception << endl;
    if (timer == receptionTimer)
    {
        receptionTimer = nullptr;
    }
    updateTransceiverState();
    updateTransceiverPart();
    */
}

void UanTransducer::captureReception(cMessage *timer)
{
    // TODO: this would be called when the receiver switches to a stronger signal while receiving a weaker one
    throw cRuntimeError("Not yet implemented");
}

void UanTransducer::sendUp(Packet *macFrame)
{
    auto signalPowerInd = macFrame->findTag<SignalPowerInd>();
    if (signalPowerInd == nullptr)
        throw cRuntimeError("signal Power indication not present");
    auto snirInd =  macFrame->findTag<SnirInd>();
    if (snirInd == nullptr)
        throw cRuntimeError("snir indication not present");

    auto errorTag = macFrame->findTag<ErrorRateInd>();

    emit(minSNIRSignal, snirInd->getMinimumSnir());
    if (errorTag && !std::isnan(errorTag->getPacketErrorRate()))
        emit(packetErrorRateSignal, errorTag->getPacketErrorRate());
    if (errorTag && !std::isnan(errorTag->getBitErrorRate()))
        emit(bitErrorRateSignal, errorTag->getBitErrorRate());
    if (errorTag && !std::isnan(errorTag->getSymbolErrorRate()))
        emit(symbolErrorRateSignal, errorTag->getSymbolErrorRate());
    EV_INFO << "Sending up " << macFrame << endl;
    FlatRadioBase::sendUp(macFrame);
    //send(macFrame, upperLayerOut);
}

double UanTransducer::getCurrentTxPower()
{
    return currentTxPower;
}

void UanTransducer::setCurrentTxPower(double txPower)
{
    currentTxPower = txPower;
}

} // namespace uan
}

