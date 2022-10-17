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

#include "UanAnalogModel.h"

#include "UanReception.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandTransmissionBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarAnalogModel.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/packetlevel/ScalarReception.h"

namespace inet {
namespace Uan {

Define_Module(UanAnalogModel);

std::ostream& UanAnalogModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    return stream << "UanAnalogModel";
}


W UanAnalogModel::computeReceptionPower(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const IRadioMedium *radioMedium = receiverRadio->getMedium();
//    const IRadio *transmitterRadio = transmission->getTransmitter();
//    const IAntenna *receiverAntenna = receiverRadio->getAntenna();
//    const IAntenna *transmitterAntenna = transmitterRadio->getAntenna();
    const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(transmission->getAnalogModel());
    const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(transmission->getAnalogModel());
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
//    const Quaternion transmissionDirection = computeTransmissionDirection(transmission, arrival);
//    const Quaternion transmissionAntennaDirection = transmission->getStartOrientation() - transmissionDirection;
//    const Quaternion receptionAntennaDirection = transmissionDirection - arrival->getStartOrientation();
    double transmitterAntennaGain = computeAntennaGain(transmission->getTransmitterAntennaGain(), transmission->getStartPosition(), arrival->getStartPosition(), transmission->getStartOrientation());
    double receiverAntennaGain = computeAntennaGain(receiverRadio->getAntenna()->getGain().get(), arrival->getStartPosition(), transmission->getStartPosition(), arrival->getStartOrientation());
    double pathLoss = radioMedium->getPathLoss()->computePathLoss(transmission, arrival);
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(narrowbandSignalAnalogModel->getCenterFrequency(), transmission->getStartPosition(), receptionStartPosition) : 1;
    W transmissionPower = scalarSignalAnalogModel->getPower();
    return transmissionPower * std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
}

const IReception *UanAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const auto * narrowTransmission = check_and_cast<const NarrowbandTransmissionBase *>(transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion receptionEndOrientation = arrival->getEndOrientation();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    W receivedPower = computeReceptionPower(receiverRadio, transmission, arrival);
    auto centerFrequency = narrowTransmission->getCenterFrequency();
    auto bandwith = narrowTransmission->getBandwidth();

    TapVector tapVec;
    Tap tap(SimTime(), 1.0);
    tapVec.push_back(tap);

    return new UanReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, centerFrequency, bandwith, receivedPower, tapVec, SimTime());
}

const INoise *UanAnalogModel::computeNoise(const IListening *listening, const IInterference *interference) const
{

    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz commonCenterFrequency = bandListening->getCenterFrequency();
    Hz commonBandwidth = bandListening->getBandwidth();

    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto reception : *interferingReceptions) {
        const ISignalAnalogModel *signalAnalogModel = reception->getAnalogModel();
        const INarrowbandSignal *narrowbandSignalAnalogModel = check_and_cast<const INarrowbandSignal *>(signalAnalogModel);
        Hz signalCenterFrequency = narrowbandSignalAnalogModel->getCenterFrequency();
        Hz signalBandwidth = narrowbandSignalAnalogModel->getBandwidth();


        if((commonCenterFrequency == signalCenterFrequency && commonBandwidth >= signalBandwidth))
        {
            const IScalarSignal *scalarSignalAnalogModel = check_and_cast<const IScalarSignal *>(signalAnalogModel);
            W power = scalarSignalAnalogModel->getPower();
            simtime_t startTime = reception->getStartTime();
            simtime_t endTime = reception->getEndTime();
            if (startTime < noiseStartTime)
                noiseStartTime = startTime;
            if (endTime > noiseEndTime)
                noiseEndTime = endTime;
            std::map<simtime_t, W>::iterator itStartTime = powerChanges->find(startTime);
            if (itStartTime != powerChanges->end())
                itStartTime->second += power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
            std::map<simtime_t, W>::iterator itEndTime = powerChanges->find(endTime);
            if (itEndTime != powerChanges->end())
                itEndTime->second -= power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
        }
        else if (areOverlappingBands(commonCenterFrequency, commonBandwidth, narrowbandSignalAnalogModel->getCenterFrequency(), narrowbandSignalAnalogModel->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }

    const ScalarNoise *backgroundNoisePowerChanges = dynamic_cast<const ScalarNoise *>(interference->getBackgroundNoise());
    if (backgroundNoisePowerChanges) {
        if (commonCenterFrequency == backgroundNoisePowerChanges->getCenterFrequency() && commonBandwidth >= backgroundNoisePowerChanges->getBandwidth())
            addNoise(backgroundNoisePowerChanges, noiseStartTime, noiseEndTime, powerChanges);
        else if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, backgroundNoisePowerChanges->getCenterFrequency(), backgroundNoisePowerChanges->getBandwidth()))
            throw cRuntimeError("Partially interfering background noise is not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");
    }


    EV_TRACE << "Noise power begin " << endl;
    W noise = W(0);
    for (std::map<simtime_t, W>::const_iterator it = powerChanges->begin(); it != powerChanges->end(); it++) {
        noise += it->second;
        EV_TRACE << "Noise at " << it->first << " = " << noise << endl;
    }
    EV_TRACE << "Noise power end" << endl;
    return new ScalarNoise(noiseStartTime, noiseEndTime, commonCenterFrequency, commonBandwidth, powerChanges);
}

const ISnir *UanAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new ScalarSnir(reception, noise);
}

} 

}

