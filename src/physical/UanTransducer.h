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

#ifndef _UANTRANSDUCER_H_
#define _UANTRANSDUCER_H_

//#include "inet/physicallayer/wireless/common/base/packetlevel/PhysicalLayerBase.h"
//#include "inet/physicallayer/common/Signal.h"
//#include "inet/physicallayer/wireless/common/contract/packetlevel/IAntenna.h"
//#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
//#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadioMedium.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatRadioBase.h"

namespace inet {
namespace Uan {

using namespace physicallayer;

class INET_API UanTransducer : public FlatRadioBase //: public PhysicalLayerBase, public virtual IRadio
{
public:
  static simsignal_t minSNIRSignal;
  static simsignal_t packetErrorRateSignal;
  static simsignal_t bitErrorRateSignal;
  static simsignal_t symbolErrorRateSignal;
  static simsignal_t droppedPacket;

protected:
  /**
   * An identifier which is globally unique for the whole lifetime of the
   * simulation among all radios.
   */
  double currentTxPower;

private:
  void parseRadioModeSwitchingTimes();
  void startRadioModeSwitch(RadioMode newRadioMode, simtime_t switchingTime);

protected:
  virtual void initialize(int stage) override;

  virtual void handleMessageWhenDown(cMessage *message) override;
  virtual void handleMessageWhenUp(cMessage *message) override;
  virtual void handleSelfMessage(cMessage *message) override;
  virtual void handleTransmissionTimer(cMessage *message) override;
  virtual void handleReceptionTimer(cMessage *message) override;
  virtual void handleUpperCommand(cMessage *command) override;
  virtual void handleLowerCommand(cMessage *command) override;
  virtual void handleUpperPacket(Packet *packet) override;
  virtual void handleSignal(WirelessSignal *signal) override;

  virtual void startTransmission(Packet *macFrame, IRadioSignal::SignalPart part) override;
  virtual void continueTransmission() override;
  virtual void endTransmission() override;
  virtual void abortTransmission() override;

  virtual WirelessSignal *createSignal(Packet *packet) const override;

  virtual void startReception(cMessage *timer, IRadioSignal::SignalPart part) override;
  virtual void continueReception(cMessage *timer) override;
  virtual void endReception(cMessage *timer) override;
  virtual void abortReception(cMessage *timer) override;
  virtual void captureReception(cMessage *timer) override;

  virtual void sendUp(Packet *macFrame) override;

public:
  UanTransducer() { }
  virtual ~UanTransducer();

  double getCurrentTxPower();
  void setCurrentTxPower(double txPower);

  std::list<cMessage *>concurrentReceptions;

  virtual int getId() const override { return id; }

  virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

  virtual const IAntenna *getAntenna() const override { return antenna; }
  virtual const ITransmitter *getTransmitter() const override { return transmitter; }
  virtual const IReceiver *getReceiver() const override { return receiver; }
  virtual const IRadioMedium *getMedium() const override { return medium; }

  virtual const cGate *getRadioGate() const override { return radioIn; }
  virtual RadioMode getRadioMode() const override { return radioMode; }

  virtual ReceptionState getReceptionState() const override { return receptionState; }
  virtual TransmissionState getTransmissionState() const override { return transmissionState; }

  virtual const ITransmission *getTransmissionInProgress() const override;
  virtual const ITransmission *getReceptionInProgress() const override;

  virtual IRadioSignal::SignalPart getTransmittedSignalPart() const override;
  virtual IRadioSignal::SignalPart getReceivedSignalPart() const override;

  virtual void decapsulate(Packet *packet) const override;
};

}
}

#endif /* _UANRADIO_H_ */
