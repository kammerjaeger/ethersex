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

#ifndef _I80_H
#define _I80_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "config.h"

#ifdef I80_LCD_SUPPORT

#define I80_CU20026MCPB 1
#define I80_TYPE I80_CU20026MCPB
/*Definition for different display types
  Add some new Displays here*/
#if I80_TYPE == I80_CU20026MCPB
    #define LCD_CHAR_PER_LINE 20 	
    #define LCD_LINES 2
#else
#error "unknown I80 compatible controller type!"

#endif

/* lcd stream */
extern FILE *lcd;
extern uint8_t current_pos;
extern uint8_t back_light;

#define clock_write(en) clock_rw(0,en)
#define clock_read(en) clock_rw(1,en)

#define noinline __attribute__((noinline))

/* prototypes */
void I80_init(void);
void I80_config(uint8_t cursor, uint8_t blink);
void I80_define_char(uint8_t n_char, uint8_t *data);
void I80_backlight(uint8_t state);
void noinline I80_clear();
inline void I80_home(void);
void noinline I80_goto(uint8_t line, uint8_t pos);
int noinline I80_put(char d, FILE *stream);
void noinline I80_hw_init(void);

#endif /* I80_LCD_SUPPORT */

#endif /* _I80_H */