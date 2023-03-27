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

#ifndef UANPHY__UANUNIDISKRECEIVER_H_
#define UANPHY__UANUNIDISKRECEIVER_H_

#include "inet/physicallayer/wireless/unitdisk/UnitDiskReceiver.h"

namespace inet {

namespace Uan {

using namespace physicallayer;

class INET_API UanUniDiskReceiver : public UnitDiskReceiver
{

  public:
    UanUniDiskReceiver();

    void initialize(int stage) override;

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

};

} // namespace physicallayer

} // namespace inet

#endif

