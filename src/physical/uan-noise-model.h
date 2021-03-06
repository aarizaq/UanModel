/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Leonard Tracy <lentracy@gmail.com>
 */

#ifndef UAN_NOISE_MODEL_H
#define UAN_NOISE_MODEL_H

#include <omnetpp.h>


namespace Uan {
using namespace omnetpp;

/**
 * \ingroup uan
 *
 * UAN Noise Model base class.
 */
class UanNoiseModel : public cObject
{
public:
  /**
   * Compute the noise power at a given frequency.
   *
   * \param fKhz Frequency in kHz.
   * \return Noise power in dB re 1uPa/Hz.
   */
  virtual double GetNoiseDbHz (double fKhz) const = 0;
};  // class UanNoiseModel

} // namespace ns3

#endif /* UAN_NOISE_MODEL_H */
