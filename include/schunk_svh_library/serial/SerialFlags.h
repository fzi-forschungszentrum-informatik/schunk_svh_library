////////////////////////////////////////////////////////////////////////////////
//
// © Copyright 2022 SCHUNK Mobile Greifsysteme GmbH, Lauffen/Neckar Germany
// © Copyright 2022 FZI Forschungszentrum Informatik, Karlsruhe, Germany
//
// This file is part of the Schunk SVH Library.
//
// The Schunk SVH Library is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// The Schunk SVH Library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// the Schunk SVH Library. If not, see <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
/*!\file
 *
 * \author  Lars Pfotzer <pfotzer@fzi.de>
 * \date    2013-03-08
 *
 * \author  Klaus Uhl
 * \date    2007-11-06
 *
 * \brief   Contains tSerialFlags
 *
 */
//----------------------------------------------------------------------
#ifndef DRIVER_SVH_SERIAL_SERIALFLAGS_H_INCLUDED
#define DRIVER_SVH_SERIAL_SERIALFLAGS_H_INCLUDED

#ifdef _SYSTEM_WIN32_
#  include <Windows.h>
#endif

#include "schunk_svh_library/ImportExport.h"

namespace driver_svh {
namespace serial {

//! Short description of tSerialFlags
/*! A more detailed description of tSerialFlags, which
    Klaus Uhl hasn't done till now!
*/
class DRIVER_SVH_IMPORT_EXPORT SerialFlags
{
public:
  enum DataBits
  {
    DB_5 = 5,
    DB_6 = 6,
    DB_7 = 7,
    DB_8 = 8
  };

  enum StopBits
  {
    SB_1,
    SB_1_P5,
    SB_2
  };

  enum Parity
  {
    P_NONE,
    P_EVEN,
    P_ODD,
    P_MARK,
    P_SPACE
  };

  enum BaudRate
  {
    BR_0      = 0,
    BR_50     = 50,
    BR_75     = 75,
    BR_110    = 110,
    BR_134    = 134,
    BR_150    = 150,
    BR_200    = 200,
    BR_300    = 300,
    BR_600    = 600,
    BR_1200   = 1200,
    BR_1800   = 1800,
    BR_2400   = 2400,
    BR_4800   = 4800,
    BR_9600   = 9600,
    BR_19200  = 19200,
    BR_38400  = 38400,
    BR_57600  = 57600,
    BR_115200 = 115200,
    BR_230400 = 230400,
    BR_500000 = 500000,
    BR_921600 = 921600
  };

  enum FlowControl
  {
    FC_FLOW,
    FC_HAND_SHAKE
  };

  enum ModemControlFlags
  {
    MCF_NULL      = 0x000,
    MCF_DTR       = 0x002,
    MCF_RTS       = 0x004,
    MCF_UNDEFINED = 0x800
  };

  SerialFlags()
    : m_data_bits(DB_8)
    , m_stop_bits(SB_1)
    , m_parity(P_NONE)
    , m_baud_rate(BR_9600)
    , m_flow_control(FC_HAND_SHAKE)
    , m_use_modem_control(true)
    , m_modem_control_flags(MCF_UNDEFINED)
    , m_enable_receiver(false)
    , m_enable_stop_on_receive(false)
  {
  }

  SerialFlags(DataBits data_bits,
              Parity parity                         = P_NONE,
              bool use_modem_control                = false,
              bool enable_receiver                  = false,
              bool enable_stop_on_receive           = false,
              ModemControlFlags modem_control_flags = MCF_UNDEFINED)
    : m_data_bits(data_bits)
    , m_stop_bits(SB_1)
    , m_parity(parity)
    , m_baud_rate(BR_9600)
    , m_flow_control(FC_HAND_SHAKE)
    , m_use_modem_control(use_modem_control)
    , m_modem_control_flags(modem_control_flags)
    , m_enable_receiver(enable_receiver)
    , m_enable_stop_on_receive(enable_stop_on_receive)
  {
  }

  SerialFlags(BaudRate baud_rate,
              DataBits data_bits,
              Parity parity                         = P_NONE,
              bool use_modem_control                = false,
              bool enable_receiver                  = false,
              bool enable_stop_on_receive           = false,
              ModemControlFlags modem_control_flags = MCF_UNDEFINED)
    : m_data_bits(data_bits)
    , m_stop_bits(SB_1)
    , m_parity(parity)
    , m_baud_rate(baud_rate)
    , m_flow_control(FC_HAND_SHAKE)
    , m_use_modem_control(use_modem_control)
    , m_modem_control_flags(modem_control_flags)
    , m_enable_receiver(enable_receiver)
    , m_enable_stop_on_receive(enable_stop_on_receive)
  {
  }

  SerialFlags(BaudRate baud_rate,
              Parity parity,
              DataBits data_bits,
              StopBits stop_bits,
              bool use_modem_control                = false,
              bool enable_receiver                  = false,
              bool enable_stop_on_receive           = false,
              ModemControlFlags modem_control_flags = MCF_UNDEFINED)
    : m_data_bits(data_bits)
    , m_stop_bits(stop_bits)
    , m_parity(parity)
    , m_baud_rate(baud_rate)
    , m_flow_control(FC_HAND_SHAKE)
    , m_use_modem_control(use_modem_control)
    , m_modem_control_flags(modem_control_flags)
    , m_enable_receiver(enable_receiver)
    , m_enable_stop_on_receive(enable_stop_on_receive)
  {
  }

  SerialFlags(const SerialFlags& flags)
    : m_data_bits(flags.m_data_bits)
    , m_stop_bits(flags.m_stop_bits)
    , m_parity(flags.m_parity)
    , m_baud_rate(flags.m_baud_rate)
    , m_flow_control(flags.m_flow_control)
    , m_use_modem_control(flags.m_use_modem_control)
    , m_modem_control_flags(flags.m_modem_control_flags)
    , m_enable_receiver(flags.m_enable_receiver)
    , m_enable_stop_on_receive(false)
  {
  }

  BaudRate getBaudRate() const { return m_baud_rate; }
  DataBits getDataBits() const { return m_data_bits; }
  bool enableStopOnReceive() const { return m_enable_stop_on_receive; }
  bool enableReceiver() const { return m_enable_receiver; }
  FlowControl getFlowControl() const { return m_flow_control; }
  Parity getParity() const { return m_parity; }
  StopBits getStopBits() const { return m_stop_bits; }
  bool useModemControl() const { return m_use_modem_control; }

  void setBaudRate(BaudRate baud_rate) { m_baud_rate = baud_rate; }

  ModemControlFlags getModemControlFlags() const { return m_modem_control_flags; }

#ifdef _SYSTEM_POSIX_
  unsigned long cFlags() const;
  static unsigned long cFlags(BaudRate baud_rate);
#endif
#ifdef _SYSTEM_WIN32_
  void GetDCB(LPDCB dcb) const;
#endif

private:
  DataBits m_data_bits;
  StopBits m_stop_bits;
  Parity m_parity;
  BaudRate m_baud_rate;
  FlowControl m_flow_control;
  bool m_use_modem_control;
  ModemControlFlags m_modem_control_flags;
  bool m_enable_receiver;
  bool m_enable_stop_on_receive;
};

} // namespace serial
} // namespace driver_svh

#endif
