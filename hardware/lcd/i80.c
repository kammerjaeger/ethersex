/*
*
* Copyright (c) 2009 by DEIN_NAME <DAVOR@DANACH.net>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#include <avr/io.h>
#include <util/delay.h>

#include "i80.h"
#include "config.h"
#include "core/debug.h"

/* global variables */
FILE *lcd;

/* macros for defining the data pins as input or output */
#define _DATA_INPUT(a)  PIN_CLEAR(I80_D ## a); \
						DDR_CONFIG_IN(I80_D ## a);

#define DATA_INPUT() do { \
              _DATA_INPUT(0); \
						  _DATA_INPUT(1); \
						  _DATA_INPUT(2); \
						  _DATA_INPUT(3); \
              _DATA_INPUT(4); \
						  _DATA_INPUT(5); \
						  _DATA_INPUT(6); \
						  _DATA_INPUT(7); \
						} while (0)

#define DATA_OUTPUT() do { \
              DDR_CONFIG_OUT(I80_D0); \
						  DDR_CONFIG_OUT(I80_D1); \
						  DDR_CONFIG_OUT(I80_D2); \
						  DDR_CONFIG_OUT(I80_D3); \
						  DDR_CONFIG_OUT(I80_D4); \
						  DDR_CONFIG_OUT(I80_D5); \
						  DDR_CONFIG_OUT(I80_D6); \
						  DDR_CONFIG_OUT(I80_D7); \
						 } while (0);

inline void CTRL_OUTPUT(){
#ifdef HAVE_I80_A0
  DDR_CONFIG_OUT(I80_A0);
#endif
#ifdef HAVE_I80_NRD
  DDR_CONFIG_OUT(I80_NRD);
#endif
#ifdef HAVE_I80_NCS
  DDR_CONFIG_OUT(I80_NCS);
#endif
  DDR_CONFIG_OUT(I80_NWR);
  PIN_CLEAR(I80_BUSY);
  DDR_CONFIG_IN(I80_BUSY);
}

void noinline output_byte(uint8_t A0, uint8_t data)
{
#ifdef HAVE_I80_NRD
    PIN_SET(I80_NRD);
#endif
#ifdef HAVE_I80_NCS
    PIN_CLEAR(I80_NCS);
#endif

#ifdef HAVE_I80_A0
    PIN_CLEAR(I80_A0);
    if (A0)
        PIN_SET(I80_A0);
#endif
    DATA_OUTPUT();
    
    /* compute data bits */
    PIN_CLEAR(I80_D0);
    PIN_CLEAR(I80_D1);
    PIN_CLEAR(I80_D2);
    PIN_CLEAR(I80_D3);
    PIN_CLEAR(I80_D4);
    PIN_CLEAR(I80_D5);
    PIN_CLEAR(I80_D6);
    PIN_CLEAR(I80_D7);
    if (data & _BV(0))
        PIN_SET(I80_D0);
    if (data & _BV(1))
        PIN_SET(I80_D1);
    if (data & _BV(2))
        PIN_SET(I80_D2);
    if (data & _BV(3))
        PIN_SET(I80_D3);
    if (data & _BV(4))
        PIN_SET(I80_D4);
    if (data & _BV(5))
        PIN_SET(I80_D5);
    if (data & _BV(6))
        PIN_SET(I80_D6);
    if (data & _BV(7))
        PIN_SET(I80_D7);

    PIN_CLEAR(I80_NWR);
    PIN_SET(I80_NWR);

#ifdef HAVE_I80_NCS
    PIN_SET(I80_NCS);
#endif

    /* wait until command is executed by checking busy flag, with timeout */

    /* max execution time is for return home command,
     * which takes at most 1.52ms = 1520us */
    uint8_t busy, timeout = 20;
    do {
        busy = !PIN_HIGH(I80_BUSY);
        _delay_us(1);
        timeout--;
    } while (busy && timeout > 0);
    #ifdef DEBUG
    if (timeout == 0)
        debug_printf("lcd timeout busy high!\n");
    #endif
    
    timeout = 200;
    do {
        busy = PIN_HIGH(I80_BUSY);
        _delay_us(10);
        timeout--;
    } while (busy && timeout > 0);
    #ifdef DEBUG
    if (timeout == 0)
        debug_printf("lcd timeout busy low!\n");
    #endif

  _delay_us(5);
}

void I80_config(uint8_t cursor, uint8_t blink) 
{
    if (cursor == 1 && blink == 1){
      output_byte(0,0x15);
    } else if (cursor == 1){
      output_byte(0,0x14);
    } else {
      output_byte(0,0x16);
    }
     
}

void I80_clear()
{
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, ' ');
    output_byte(0, '\r');
    output_byte(0, '\n');
}

void I80_goto(uint8_t line, uint8_t pos)
{
    uint8_t current_pos = ((line * LCD_CHAR_PER_LINE) + pos) % (LCD_CHAR_PER_LINE * LCD_LINES);
    output_byte(0, 0x1b);
    output_byte(0, 0x48);
    output_byte(0, current_pos);
    _delay_us(5);
}

inline void I80_home(void)
{
    I80_goto(0,0);
}

void I80_define_char(uint8_t n_char, uint8_t *data)
{

  output_byte(0, 0x1b);
  output_byte(0, 0x43);
  output_byte(0, n_char);
  
  /* clear junk */
  n_char = 0;
  while (n_char < 8) {
      data[n_char] = (0x1f & data[n_char]);
      n_char++;
  }
  
  data[0] = data[0] << 3;
  data[0] = data[0] | (data[1] >> 2);
  data[1] = data[1] << 6;
  data[1] = data[1] | (data[2] << 1) | (data[3] >>4);
  data[2] = (data[3] << 4) | (data[4] >>1);
  data[3] = (data[4] << 7) | (data[5] <<2) | (data[6] >>3);
  data[4] = (data[6] << 5) | data[7];
  n_char = 0;
  
  while (n_char < 5) {
    /* send the data to lcd into ram */
    output_byte(0, *(data + n_char));
    n_char++;
  }
}

void I80_backlight(uint8_t state)
{
    output_byte(0, 0x1b);
    output_byte(0, 0x4c);
    output_byte(0, state);
}

void I80_init()
{
  I80_hw_init();
  output_byte(0,0x1b);
  output_byte(0,0x49);
  _delay_us(2000);
  I80_config(0,0);
  
  /* open file descriptor */
  lcd = fdevopen(I80_put, NULL);
}

void noinline I80_hw_init(void)
{


#ifdef HAVE_I80_NRD
    PIN_SET(I80_NRD);
#endif
#ifdef HAVE_I80_NCS
    PIN_SET(I80_NCS);
#endif

#ifdef HAVE_I80_A0
    PIN_CLEAR(I80_A0);
#endif
    PIN_SET(I80_NWR);
    
    /* init io pins */
    CTRL_OUTPUT();
    
    PIN_CLEAR(I80_D0);
    PIN_CLEAR(I80_D1);
    PIN_CLEAR(I80_D2);
    PIN_CLEAR(I80_D3);
    PIN_CLEAR(I80_D4);
    PIN_CLEAR(I80_D5);
    PIN_CLEAR(I80_D6);
    PIN_CLEAR(I80_D7);
    DATA_OUTPUT();
    
}

int I80_put(char d, FILE *stream)
{
    output_byte(0, d);

    return 0;
}

/*
  -- Ethersex META --
  header(hardware/lcd/i80.h)
  init(I80_init)
*/