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
#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/NarrowbandReceiverBase.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarSnir.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarReceptionAnalogModel.h"


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
    const  auto scalarSignalAnalogModel = check_and_cast<const ScalarSignalAnalogModel *>(transmission->getAnalogModel());
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
//    const Quaternion transmissionDirection = computeTransmissionDirection(transmission, arrival);
//    const Quaternion transmissionAntennaDirection = transmission->getStartOrientation() - transmissionDirection;
//    const Quaternion receptionAntennaDirection = transmissionDirection - arrival->getStartOrientation();
    double transmitterAntennaGain = computeAntennaGain(transmission->getTransmitterAntennaGain(), transmission->getStartPosition(), arrival->getStartPosition(), transmission->getStartOrientation());
    double receiverAntennaGain = computeAntennaGain(receiverRadio->getAntenna()->getGain().get(), arrival->getStartPosition(), transmission->getStartPosition(), arrival->getStartOrientation());
    double pathLoss = radioMedium->getPathLoss()->computePathLoss(transmission, arrival);
    double obstacleLoss = radioMedium->getObstacleLoss() ? radioMedium->getObstacleLoss()->computeObstacleLoss(scalarSignalAnalogModel->getCenterFrequency(), transmission->getStartPosition(), receptionStartPosition) : 1;
    W transmissionPower = scalarSignalAnalogModel->getPower();
    return transmissionPower * std::min(1.0, transmitterAntennaGain * receiverAntennaGain * pathLoss * obstacleLoss);
}

const IReception *UanAnalogModel::computeReception(const IRadio *receiverRadio, const ITransmission *transmission, const IArrival *arrival) const
{
    const ScalarSignalAnalogModel *transmissionAnalogModel = check_and_cast<const ScalarSignalAnalogModel *>(transmission->getAnalogModel());
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Quaternion receptionStartOrientation = arrival->getStartOrientation();
    const Quaternion receptionEndOrientation = arrival->getEndOrientation();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    W receivedPower = computeReceptionPower(receiverRadio, transmission, arrival);

    TapVector tapVec;
    Tap tap(SimTime(), 1.0);
    tapVec.push_back(tap);
    auto receptionAnalogModel = new ScalarReceptionAnalogModel(transmissionAnalogModel->getPreambleDuration(), transmissionAnalogModel->getHeaderDuration(), transmissionAnalogModel->getDataDuration(), transmissionAnalogModel->getCenterFrequency(), transmissionAnalogModel->getBandwidth(), receivedPower);
    return new UanReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionStartOrientation, receptionEndOrientation, receptionAnalogModel, tapVec, SimTime());
}

const INoise *UanAnalogModel::computeNoise(const IListening *listening, const IInterference *interference) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    Hz commonCenterFrequency = bandListening->getCenterFrequency();
    Hz commonBandwidth = bandListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> powerChanges;
    powerChanges[math::getLowerBound<simtime_t>()] = W(0);
    powerChanges[math::getUpperBound<simtime_t>()] = W(0);
    const std::vector<const IReception *> *interferingReceptions = interference->getInterferingReceptions();
    for (auto reception : *interferingReceptions) {
        auto signalAnalogModel = reception->getAnalogModel();
        auto receptionAnalogModel = check_and_cast<const ScalarReceptionAnalogModel *>(signalAnalogModel);
        Hz signalCenterFrequency = receptionAnalogModel->getCenterFrequency();
        Hz signalBandwidth = receptionAnalogModel->getBandwidth();
        if (commonCenterFrequency == signalCenterFrequency && commonBandwidth >= signalBandwidth)
            addReception(reception, noiseStartTime, noiseEndTime, powerChanges);
        else if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, receptionAnalogModel->getCenterFrequency(), receptionAnalogModel->getBandwidth()))
            throw cRuntimeError("Partially interfering signals are not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");
    }
    const ScalarNoise *scalarBackgroundNoise = dynamic_cast<const ScalarNoise *>(interference->getBackgroundNoise());
    if (scalarBackgroundNoise) {
        if (commonCenterFrequency == scalarBackgroundNoise->getCenterFrequency() && commonBandwidth >= scalarBackgroundNoise->getBandwidth())
            addNoise(scalarBackgroundNoise, noiseStartTime, noiseEndTime, powerChanges);
        else if (!ignorePartialInterference && areOverlappingBands(commonCenterFrequency, commonBandwidth, scalarBackgroundNoise->getCenterFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Partially interfering background noise is not supported by ScalarAnalogModel, enable ignorePartialInterference to avoid this error!");
    }
    EV_TRACE << "Noise power begin " << endl;
    W power = W(0);
    for (auto & it : powerChanges) {
        power += it.second;
        it.second = power;
        EV_TRACE << "Noise at " << it.first << " = " << power << endl;
    }
    EV_TRACE << "Noise power end" << endl;

    const auto& powerFunction = makeShared<math::Interpolated1DFunction<W, simtime_t>>(powerChanges, &math::LeftInterpolator<simtime_t, W>::singleton);
    return new ScalarNoise(noiseStartTime, noiseEndTime, commonCenterFrequency, commonBandwidth, powerFunction);
}

const ISnir *UanAnalogModel::computeSNIR(const IReception *reception, const INoise *noise) const
{
    return new ScalarSnir(reception, noise);
}

} 

}

