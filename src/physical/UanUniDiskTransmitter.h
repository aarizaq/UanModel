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

#ifndef UANPHY_UANUNIDISKTRANSMITTER_H_
#define UANPHY_UANUNIDISKTRANSMITTER_H_
#include "UanTransmitter.h"


namespace inet {
namespace Uan {

using namespace physicallayer;

class UanUniDiskTransmitter : public UanTransmitter {
protected:
    m communicationRange = m(NaN);
    m interferenceRange = m(NaN);
    m detectionRange = m(NaN);
    public:
        UanUniDiskTransmitter();
        virtual void initialize(int stage) override;
        virtual m getMaxCommunicationRange() const override { return communicationRange; }
        virtual m getMaxInterferenceRange() const override { return interferenceRange; }
        virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, const simtime_t startTime) const override;
        virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
};

}
}

#endif
