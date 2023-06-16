/*
 * LoRaReception.h
 *
 *  Created on: Feb 17, 2017
 *      Author: slabicm1
 */

#ifndef UANPHY_UANRECEPTION_H_
#define UANPHY_UANRECEPTION_H_

#include <complex>
#include "inet/physicallayer/wireless/common/base/packetlevel/ReceptionBase.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalBitModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSampleModel.h"
#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalSymbolModel.h"


namespace inet {
namespace Uan {


using namespace physicallayer;

class Tap
{
public:
  /**
   * Default constructor.  Creates Tap with delay=0, amp=0
   */
  Tap();
  /**
   * Constructor
   *
   * \param delay Time delay (usually from first arrival) of signal
   * \param amp Complex amplitude of arrival
   */
  Tap (omnetpp::simtime_t _delay, std::complex<double> amp):delay(_delay),  amplitude(amp){}
  Tap (omnetpp::simtime_t _delay, double amp) : delay(_delay), amplitude(std::complex<double>(amp, 0)){}
  Tap (std::complex<double> amp): Tap (omnetpp::SimTime(), amp){}
  Tap (double amp) : Tap (omnetpp::SimTime(), amp){}

  /**
   * Get the complex amplitude of arrival.
   *
   * \return The amplitude.
   */
  std::complex<double> getAmp (void) const {return amplitude;};
  /**
   * Get the delay time, usually from first arrival of signal.
   * \return The time delay.
   */
  omnetpp::simtime_t getDelay (void) const {return delay;};

private:
  omnetpp::simtime_t delay;        //!< The time delay.
  std::complex<double> amplitude;  //!< The amplitude.
};  // class Tap

typedef std::vector<Tap> TapVector;

class INET_API UanReception : public ReceptionBase
{
protected:
    TapVector tapVector;
    omnetpp::simtime_t resolution;
  public:
    UanReception(const IRadio *radio, const ITransmission *transmission, const simtime_t startTime, const simtime_t endTime, const Coord& startPosition, const Coord& endPosition, const Quaternion& startOrientation, const Quaternion& endOrientation, const IReceptionAnalogModel *analogModel, TapVector arrivals, omnetpp::simtime_t resolution);
    virtual W getPower() const;
    virtual W computeMinPower(simtime_t startTime, simtime_t endTime) const;
    virtual const TapVector &getTaps() const {return tapVector;}
    virtual const omnetpp::simtime_t &getResolution() const {return resolution;}
    virtual TapVector &getTapsForUpdate() {return const_cast<TapVector&>(getTaps());}
    virtual omnetpp::simtime_t &getResolutionForUpdate() {return const_cast<omnetpp::simtime_t &>(getResolution());}
};

}
}

#endif
