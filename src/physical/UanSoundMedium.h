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
#ifndef __UANMEDIUM_H_
#define __UANMEDIUM_H_
#include <algorithm>
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReceptionResult.h"
#include "inet/physicallayer/wireless/common/medium/RadioMedium.h"

#include "UanTransducer.h"

namespace inet{
namespace Uan {

using namespace physicallayer;

class INET_API UanSoundMedium : public RadioMedium
{
    friend class UanTransducer;
protected:
   // virtual bool matchesMacAddressFilter(const IRadio *radio, const Packet *packet) const override;
        //@}
    public:
    UanSoundMedium();
      virtual ~UanSoundMedium();
      //virtual const IReceptionDecision *getReceptionDecision(const IRadio *receiver, const IListening *listening, const ITransmission *transmission, IRadioSignal::SignalPart part) const override;
      virtual const IReceptionResult *getReceptionResult(const IRadio *receiver, const IListening *listening, const ITransmission *transmission) const override;
};
}
}

#endif
