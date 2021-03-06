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

#ifndef _UANPATHLOSSTHORP_H_
#define _UANPATHLOSSTHORP_H_

#include "inet/physicallayer/wireless/common/base/packetlevel/PathLossBase.h"


namespace inet{
namespace Uan {

using namespace physicallayer;

/**
 * This class implements the log normal shadowing model.
 */
class INET_API UanPathLossThorp : public PathLossBase
{
  protected:
    double m_SpreadCoef;
    double GetAttenDbKm (double freqKhz) const;
  protected:

    virtual void initialize(int stage) override;

  public:
    UanPathLossThorp();
    virtual double computePathLoss(mps propagationSpeed, Hz frequency, m distance) const override;
    virtual m computeRange(mps propagationSpeed, Hz frequency, double loss) const override;
};

} // namespace physicallayer
}

#endif /* LORAPHY_LORAPATHLOSSOULU_H_ */
