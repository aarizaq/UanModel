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

#ifndef __UAN_UANSCALARBACKGROUNDNOISE_H
#define __UAN_UANSCALARBACKGROUNDNOISE_H


#include "inet/physicallayer/wireless/common/contract/packetlevel/IBackgroundNoise.h"
namespace inet {
namespace Uan {


using namespace physicallayer;


class INET_API UanScalarBackgroundNoise : public cModule, public IBackgroundNoise
{
protected:
     double shipping = NaN;
     mps windSpeed = mps(NaN);
  protected:
    virtual void initialize(int stage) override;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const INoise *computeNoise(const IListening *listening) const override;
};

} // namespace physicallayer
}

#endif

