/**
 * @file Cosa/LCD/Driver/HD44780_IO_SR3W.cpp
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2013, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/LCD/Driver/HD44780.hh"

void 
HD44780::SR3W::setup()
{
  m_port.as_uint8 = 0;
}

void 
HD44780::SR3W::write4b(uint8_t data)
{
  m_port.data = data;
  uint8_t value = m_port.as_uint8;
  m_sda.write(value & 0x20);
  m_scl.toggle();
  m_scl.toggle();
  m_sda.write(value & 0x10);
  m_scl.toggle();
  m_scl.toggle();
  m_sda.write(value & 0x08);
  m_scl.toggle();
  m_scl.toggle();
  m_sda.write(value & 0x04);
  m_scl.toggle();
  m_scl.toggle();
  m_sda.write(value & 0x02);
  m_scl.toggle();
  m_scl.toggle();
  m_sda.write(value & 0x01);
  m_scl.toggle();
  m_scl.toggle();
  m_en.toggle();
  m_en.toggle();
}

void 
HD44780::SR3W::write8b(uint8_t data)
{
  m_port.data = data >> 4;
  uint8_t value = m_port.as_uint8;
  for (uint8_t i = 0; i < 2; i++) {
    m_sda.write(value & 0x20);
    m_scl.toggle();
    m_scl.toggle();
    m_sda.write(value & 0x10);
    m_scl.toggle();
    m_scl.toggle();
    m_sda.write(value & 0x08);
    m_scl.toggle();
    m_scl.toggle();
    m_sda.write(value & 0x04);
    m_scl.toggle();
    m_scl.toggle();
    m_sda.write(value & 0x02);
    m_scl.toggle();
    m_scl.toggle();
    m_sda.write(value & 0x01);
    m_scl.toggle();
    m_scl.toggle();
    m_en.toggle();
    m_en.toggle();
    m_port.data = data;
    value = m_port.as_uint8;
  }
#if (I_CPU >= 16)
  DELAY(SHORT_EXEC_TIME);
#endif
}

void 
HD44780::SR3W::set_mode(uint8_t flag)
{
  m_port.rs = flag;
}

void 
HD44780::SR3W::set_backlight(uint8_t flag)
{
  m_port.bt = flag;
}
