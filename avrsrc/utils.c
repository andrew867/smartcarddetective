/**
 * \file
 * \brief	utils.c source file
 *
 * This file implements some utility functions used in several parts
 * of the code for this project
 *
 * These functions are not microcontroller dependent but they are intended
 * for the AVR 8-bit architecture
 *
 * Copyright (C) 2013 Omar Choudary (omar.choudary@cl.cam.ac.uk)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "utils.h"
#include "scd_io.h"
#include "scd_hal.h"
#include "scd_logger.h"
#include "scd_values.h"

/**
 * Write a 16 bit value using an atomic operation
 */
void Write16bitRegister(volatile uint16_t *reg, uint16_t value)
{
  uint8_t sreg;

  sreg = SREG;
  cli();
  *reg = value;
  SREG = sreg;	
}

/**
 * Read a 16 bit value using an atomic operation
 */
uint16_t Read16bitRegister(volatile uint16_t *reg)
{
  uint16_t i;
  uint8_t sreg;

  sreg = SREG;
  cli();
  i = *reg;
  SREG = sreg;

  return i;
}

/**
 * This function puts the SCD to sleep (including all peripherials), until
 * there is clock received from terminal
 */
void SleepUntilTerminalClock()
{
  uint8_t sreg, lcdstate;

  Write16bitRegister(&OCR3A, 100);
  Write16bitRegister(&TCNT3, 1);
  TCCR3A = 0;
  TIMSK3 = 0x02;  //Interrupt on Timer3 compare A match
  TCCR3B = 0x0F;  // CTC, timer external source
  sreg = SREG;

  // stop LCD and LEDs before going to sleep
  lcdstate = GetLCDState();
  if(lcdAvailable && lcdstate != 0) LCDOff();
  Led1Off();
  Led2Off();
  Led3Off();
  Led4Off();

  // go to sleep
  set_sleep_mode(SLEEP_MODE_IDLE); // it is also possible to use sleep_mode() below
  cli();
  sleep_enable();
  sei();
  sleep_cpu();

  // back from sleep
  sleep_disable();
  SREG = sreg;
  TIMSK3 = 0; // disable interrupts on Timer3
  TCCR3B = 0; // stop timer   
  Led4On();
}

/**
 * This function puts the SCD to sleep (including all peripherials), until
 * the card is inserted or removed
 */
void SleepUntilCardInserted()
{
  uint8_t sreg, lcdstate;

  // stop LCD and LEDs before going to sleep
  lcdstate = GetLCDState();
  if(lcdAvailable && lcdstate != 0) LCDOff();
  Led1Off();
  Led2Off();
  Led3Off();
  Led4Off();

  // go to sleep
  sreg = SREG;
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  sei();
  sleep_cpu();

  // back from sleep
  sleep_disable();
  SREG = sreg;
  Led4On();
}


/**
 * Retrieve relative time value and write it to log
 *
 * @param logger the logger struct used for logging the time. This should not be
 * NULL.
 * @return zero if success, non-zero otherwise.
 */
uint8_t LogCurrentTime(log_struct_t *logger)
{
  uint32_t time;

  if(logger == NULL)
    return RET_ERR_PARAM;

  time = GetCounter();
  LogByte4(
      logger,
      LOG_TIME_GENERAL,
      (time & 0xFF),
      ((time >> 8) & 0xFF),
      ((time >> 16) & 0xFF),
      ((time >> 24) & 0xFF));

  return 0;
}


