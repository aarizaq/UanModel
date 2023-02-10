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

#ifndef __UANCSMACAMAC_H
#define __UANCSMACAMAC_H

#include "UanIMac.h"
#include "inet/linklayer/csmaca/CsmaCaMac.h"

namespace inet {
namespace Uan {

class INET_API UanCsmaCaMac : public UanIMac, public CsmaCaMac
{
  protected:
    /**
     * @name Initialization functions
     */
    //@{
    /** @brief Initialization of the module and its variables */
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleUpperPacket(Packet *packet) override;
    virtual void encapsulate(Packet *frame) override;
    virtual void decapsulate(Packet *frame) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;
    //@}
    /**
     * @name Timer functions
     * @brief These functions have the side effect of starting the corresponding timers.
     */
    //@{
  public:
    virtual b getHeaderLength() const override {return headerLength;}
};

}
} // namespace inet

#endif

