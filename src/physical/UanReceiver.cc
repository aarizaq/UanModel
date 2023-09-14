//
// Copyright (C) 2014 Florian Meier
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "UanReceiver.h"
#include "UanQamModulation.h"
namespace inet {

namespace Uan {

Define_Module(UanReceiver);

UanReceiver::UanReceiver() :
        FlatReceiverBase()
{
}

void UanReceiver::initialize(int stage)
{

    SnirReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // from FlatReceiverBase
        errorModel = dynamic_cast<IErrorModel *>(getSubmodule("errorModel"));
        energyDetection = mW(math::dBmW2mW(par("energyDetection")));
        sensitivity = mW(math::dBmW2mW(par("sensitivity")));

        // from NarrowbandReceiverBase
        // we are using non standard modulation.
        // modulation = ApskModulationBase::findModulation(par("modulation"));
        centerFrequency = Hz(par("centerFrequency"));
        bandwidth = Hz(par("bandwidth"));

        if (!strcmp("QAM-4", par("modulation").stringValue()))
             modulation = &UanQam4Modulation::singleton;
        else if (!strcmp("QAM-8", par("modulation").stringValue()))
             modulation = &UanQam8Modulation::singleton;
        else
             modulation = ApskModulationBase::findModulation(par("modulation").stringValue());
    }
}

std::ostream& UanReceiver::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UanReceiver";
    return FlatReceiverBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

