/**
 * @file Cosa/Watchdog.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012, Mikael Patel
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
 * Watchdog abstraction. Low power timer.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_WATCHDOG_H__
#define __COSA_WATCHDOG_H__

#include <avr/wdt.h>
#include <avr/sleep.h>

#include "Cosa/Types.h"
#include "Cosa/Bits.h"
#include "Cosa/Event.h"
#include "Cosa/Thing.h"

class Watchdog {

public:
  /**
   * Callback function prototype for watchdog timeout.
   */
  typedef void (*InterruptHandler)(void* env);

private:
  // FIX: Allow multiple callbacks with any interval of delay (tick count)
  static volatile uint16_t _ticks;
  static InterruptHandler _handler;
  static uint8_t _prescale;
  static uint8_t _mode;
  static void* _env;
  
public:
  /**
   * Set watchdog timeout interrupt handler.
   * @param[in] fn interrupt handler.
   * @param[in] env environment pointer.
   */
  static void set(InterruptHandler fn, void* env = 0) 
  { 
    _handler = fn; 
    _env = env; 
  }

  /**
   * Get number of watchdog cycles.
   * @return number of ticks.
   */
  static uint16_t get_ticks() 
  { 
    return (_ticks); 
  }

  /**
   * Get number of milli-seconds per tick.
   * @return milli-seconds.
   */
  static uint16_t ms_per_tick() 
  { 
    return (16 << _prescale); 
  }

  /**
   * Start watchdog with given period (milli-seconds), sleep mode and 
   * timeout interrupt handler. The timeout period is mapped to 
   * 16 milli-seconds and double periods (32, 64, 128, etc to approx 
   * 8 seconds).
   * @param[in] ms timeout period in milli-seconds.
   * @param[in] fn function.
   * @param[in] mode sleep mode.
   */
  static void begin(uint16_t ms, 
		    InterruptHandler fn = 0, 
		    uint8_t mode = SLEEP_MODE_IDLE);

  /**
   * Await watchdogs next tick. Put into sleep mode according to 
   * begin setup.
   */
  static void await();

  /**
   * Delay using watchdog timeouts and sleep mode.
   * @param[in] ms sleep period in milli-seconds.
   */
  static void delay(uint16_t ms);
  
  /**
   * Stop watchdog. Turn off timout callback.
   */
  static void end() 
  { 
    wdt_disable(); 
  }

  /**
   * Trampoline function for interrupt service on watchdog timeout.
   */
  static void on_timeout()
  { 
    _ticks++; 
    if (_handler != 0) _handler(_env); 
  }

  /**
   * Callback function to push event for watchdog timeout.
   */
  static void push_event(void* env) 
  { 
    Event::push(Event::TIMEOUT_TYPE, 0, env);
  }
};

#endif
