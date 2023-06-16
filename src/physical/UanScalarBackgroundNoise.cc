//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "UanScalarBackgroundNoise.h"
#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/analogmodel/scalar/ScalarNoise.h"
#include "inet/physicallayer/wireless/common/radio/packetlevel/BandListening.h"

namespace inet {
namespace Uan {

Define_Module(UanScalarBackgroundNoise);

void UanScalarBackgroundNoise::initialize(int stage)
{
    cModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        shipping = par("shipping");
        if (shipping < 0 || shipping > 0)
            throw cRuntimeError("shipping must be in the range 0-1, actual value %f", shipping);
        windSpeed = mps(par("windSpeed"));
    }
}

std::ostream& UanScalarBackgroundNoise::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UanScalarBackgroundNoise";
    if (level <= PRINT_LEVEL_DETAIL) {
        stream << EV_FIELD(shipping);
        stream << EV_FIELD(windSpeed);
    }
    return stream;
}


// Common acoustic noise formulas from "Principles of Underwater Sound" by Robert J. Urick

const INoise *UanScalarBackgroundNoise::computeNoise(const IListening *listening) const
{
    const BandListening *bandListening = check_and_cast<const BandListening *>(listening);
    simtime_t startTime = listening->getStartTime();
    simtime_t endTime = listening->getEndTime();
    Hz centerFrequency = bandListening->getCenterFrequency();
    Hz listeningBandwidth = bandListening->getBandwidth();

    double fKhz = centerFrequency.get()/1000;

    double turbDb = 17.0 - 30.0 * std::log10 (fKhz);
    double turb = math::dBmW2mW(turbDb);
    double shipDb = 40.0 + 20.0 * (shipping - 0.5) + 26.0 * std::log10 (fKhz) - 60.0 * std::log10 (fKhz + 0.03);
    double ship = math::dBmW2mW(shipDb);
    double windDb = 50.0 + 7.5 * std::pow (windSpeed.get(), 0.5) + 20.0 * std::log10 (fKhz) - 40.0 * std::log10 (fKhz + 0.4);
    double wind = math::dBmW2mW(windDb);
    double thermalDb = -15 + 20 * std::log10 (fKhz);
    double thermal = math::dBmW2mW(thermalDb);
    double power = (turb + ship + wind + thermal)/1000.0; // in pascals

    //double turb1 = std::pow (10.0, turbDb * 0.1);
    //double ship1 = std::pow (10.0, (shipDb * 0.1));
    //double wind1 = std::pow (10.0, windDb * 0.1);
    //double thermal1 = std::pow (10, thermalDb * 0.1);
    //double powerDb = 10 * std::log10 (turb1 + ship1 + wind1 + thermal1);

    const auto& powerFunction = makeShared<math::Boxcar1DFunction<W, simtime_t>>(startTime, endTime, W(power));
    return new ScalarNoise(startTime, endTime, centerFrequency, listeningBandwidth, powerFunction);
}


const W UanScalarBackgroundNoise::getNoiseReference(const Hz &centerFrequency) const
{
    double fKhz = centerFrequency.get()/1000;
    double turbDb = 17.0 - 30.0 * std::log10 (fKhz);
    double turb = math::dBmW2mW(turbDb);
    double shipDb = 40.0 + 20.0 * (shipping - 0.5) + 26.0 * std::log10 (fKhz) - 60.0 * std::log10 (fKhz + 0.03);
    double ship = math::dBmW2mW(shipDb);
    double windDb = 50.0 + 7.5 * std::pow (windSpeed.get(), 0.5) + 20.0 * std::log10 (fKhz) - 40.0 * std::log10 (fKhz + 0.4);
    double wind = math::dBmW2mW(windDb);
    double thermalDb = -15 + 20 * std::log10 (fKhz);
    double thermal = math::dBmW2mW(thermalDb);
    double power = (turb + ship + wind + thermal)/1000.0; // in pascals
    return W(power); // Should be in Pascal, but
}


} // Uan
}


