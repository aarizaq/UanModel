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

#include "UanPathLossThorp.h"

namespace inet{
namespace Uan {

Define_Module(UanPathLossThorp);

UanPathLossThorp::UanPathLossThorp():
m_SpreadCoef(1.5)
{
}

void UanPathLossThorp::initialize(int stage)
{
    PathLossBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        m_SpreadCoef = par("spreadCoef");
    }
}

double UanPathLossThorp::GetAttenDbKm (double freqKhz) const {
  double fsq = freqKhz * freqKhz;
  double atten;
  if (freqKhz >= 0.4) {
      atten = 0.11 * fsq / (1 + fsq) + 44 * fsq / (4100 + fsq)
              + 2.75 * 0.0001 * fsq + 0.003;
  }
  else {
      atten = 0.002 + 0.11 * (freqKhz / (1 + freqKhz)) + 0.011 * freqKhz;
  }
  return atten;
}

double UanPathLossThorp::computePathLoss(mps propagationSpeed, Hz frequency, m distance) const
{
    double attenuationDb = m_SpreadCoef * 10.0 * log10 (distance.get())
           + (distance.get() / 1000.0) * GetAttenDbKm (frequency.get() / 1000.0);
    return math::dB2fraction(attenuationDb);
}

m UanPathLossThorp::computeRange(mps propagationSpeed, Hz frequency, double loss) const
{
    // distance = (waveLength ^ 2 / (16 * PI ^ 2 * systemLoss * loss)) ^ (1 / alpha)
    double att = math::dB2fraction(GetAttenDbKm (frequency.get() / 1000.0));
    double dist = 1000 * loss/att;

    return m(dist);
}

}
}
