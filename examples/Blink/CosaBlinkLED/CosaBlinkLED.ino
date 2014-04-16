/**
 * @file CosaLED.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014, Mikael Patel
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
 * @section Description
 * Cosa demonstration of LED class and static instance.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/LED.hh"
#include "Cosa/Power.hh"
#include "Cosa/Event.hh"
#include "Cosa/Watchdog.hh"

void setup()
{
  Power::set(SLEEP_MODE_PWR_DOWN);
  Watchdog::begin(128, Watchdog::push_timeout_events);
  static LED builtin;
  builtin.alert_mode();
}

void loop()
{
  Event event;
  Event::queue.await(&event);
  event.dispatch();
}