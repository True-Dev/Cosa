/**
 * @file Cosa/IOStream/Driver/ST7565P.cpp
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

#include "Cosa/IOStream/Driver/ST7565P.hh"
#include "Cosa/Watchdog.hh"

// Enable to allow reversed display type
#define MIRRORED

// Mark block as data write operations
#define data							\
  for (uint8_t i = (m_cs.clear(), 1); i != 0; i--, m_cs.set())

// Mark block as command write operations
#define commands							\
  for (uint8_t i = (m_cs.clear(), m_dc.clear(), 1);			\
       i != 0;								\
       i--, m_dc.set(), m_cs.set())

// Initialization script
const uint8_t ST7565P::script[] PROGMEM = {
#if defined(MIRRORED)
  LCD_BIAS_9,
  ADC_REVERSE,
#else
  LCD_BIAS_7,
  ADC_NORMAL,
#endif
  COM_OUTPUT_NORMAL,
  SET_DISPLAY_START 	| 0,
  SET_POWER_CONTROL 	| 0x04,
  SCRIPT_PAUSE		, 50,
  SET_POWER_CONTROL 	| 0x06,
  SCRIPT_PAUSE		, 50,
  SET_POWER_CONTROL 	| 0x07,
  SCRIPT_PAUSE		, 10,
  SET_RESISTOR_RATIO 	| 0x06,
  DISPLAY_ON,
  DISPLAY_NORMAL,
  DISPLAY_NORMAL_POINTS,
  SCRIPT_END
};

void 
ST7565P::set(uint8_t x, uint8_t y)
{
#if defined(MIRRORED)
  x += 132 - WIDTH;
#endif
  commands {
    write(SET_X_ADDR | ((x >> 4) & X_ADDR_MASK));
    write(x & X_ADDR_MASK);
    write(SET_Y_ADDR | (y & Y_ADDR_MASK));
  }
}

bool 
ST7565P::begin(uint8_t level)
{
  const uint8_t* bp = script;
  uint8_t cmd;
  commands {
    while ((cmd = pgm_read_byte(bp++)) != SCRIPT_END) {
      if (cmd == SCRIPT_PAUSE) {
	uint8_t ms = pgm_read_byte(bp++);
	Watchdog::delay(ms);
      }
      else write(cmd);
    }
  }
  set_display_contrast(level);
  m_x = 0;
  m_y = 0;
  set(m_x, m_y);
  return (true);
}

bool 
ST7565P::end()
{
  commands {
    write(DISPLAY_OFF);
  }
  return (true);
}

void 
ST7565P::set_display_mode(DisplayMode mode)
{
  commands {
    write(DISPLAY_NORMAL | mode);
  }
}

void 
ST7565P::set_display_contrast(uint8_t level)
{
  commands {
    write(SET_CONSTRAST);
    write(CONSTRAST_MASK & level);
  }
}

void 
ST7565P::set_cursor(uint8_t x, uint8_t y)
{
  set(x, y);
  m_x = (x & (WIDTH - 1));
  m_y = (y & (LINES - 1));
}

void 
ST7565P::draw_icon(const uint8_t* bp)
{
  uint8_t width = pgm_read_byte(bp++);
  uint8_t height = pgm_read_byte(bp++);
  uint8_t lines = (height >> 3);
  for (uint8_t y = 0; y < lines; y++) {
    data {
      for (uint8_t x = 0; x < width; x++) {
	write(m_mode ^ pgm_read_byte(bp++));
      }
    }
    set_cursor(m_x, m_y + 1);
  }
  set_cursor(m_x, m_y + 1);
}

void 
ST7565P::draw_bitmap(uint8_t* bp, uint8_t width, uint8_t height)
{
  uint8_t lines = (height >> 3);
  for (uint8_t y = 0; y < lines; y++) {
    data {
      for (uint8_t x = 0; x < width; x++) {
	write(m_mode ^ (*bp++));
      }
    }
    set_cursor(m_x, m_y + 1);
  }
  set_cursor(m_x, m_y + 1);
}

void 
ST7565P::draw_bar(uint8_t procent, uint8_t width, uint8_t pattern)
{
  if (procent > 100) procent = 100;
  uint8_t filled = (procent * (width - 2U)) / 100;
  uint8_t boarder = (m_y == 0 ? 0x81 : 0x80);
  width -= (filled + 1);
  data {
    write(m_mode ^ 0xff);
    while (filled--) {
      write(m_mode ^ (pattern | boarder));
      pattern = ~pattern;
    }
    write(m_mode ^ 0xff);
    width -= 1;
    if (width > 0) {
      while (width--)
	write(m_mode ^ boarder);
    }
    write(m_mode ^ 0xff);
  }
}

int 
ST7565P::putchar(char c)
{
  // Check for special characters; carriage-return-line-feed
  if (c == '\n') {
    set_cursor(0, m_y + 1);
    fill(m_mode, WIDTH);
    set(m_x, m_y);
    return (c);
  }
  
  // Check for special character: form-feed
  if (c == '\f') {
    for (uint8_t y = 0; y < LINES; y++) {
      set(0, y);
      fill(m_mode, WIDTH);
    }
    set_cursor(0, 0);
    return (c);
  }

  // Check for special character: back-space
  if (c == '\b') {
    uint8_t width = m_font->get_width(' ');
    if (m_x < width) width = m_x;
    set_cursor(m_x - width, m_y);
    return (c);
  }

  // Access font for character width and bitmap
  uint8_t width = m_font->get_width(c);
  const uint8_t* bp = m_font->get_bitmap(c);
  m_x += width;

  // Check that the character is not clipped
  if (m_x > WIDTH) {
    putchar('\n');
    m_x = width;
  }

  // Write character to the display memory and an extra byte
  data {
    while (--width) 
      write(m_mode ^ pgm_read_byte(bp++));
    write(m_mode);
  }

  return (c);
}
