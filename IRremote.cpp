/*
 * IRremote
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz> 
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 * Modified by Gaspard van Koningsveld to trim out IRrecv, not using PWM anymore, allow setting of IR LED pin, and make it compatible with the Spark Core v1.0 (STM32F103CB based)
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

// #include "IRremote.h"

IRsend::IRsend(int irPin) : irPin(irPin) {};


// From https://github.com/zoellner/IRLib/blob/master/IRLib.cpp and also 
// based on https://gist.github.com/technobly/8313449
uint16_t TIM_ARR = (uint16_t)(24000000 / 38000) - 1; // 38 KHz Init
 
// User defined analogWrite() to gain control of PWM initialization
void IRsend::analogWrite2(uint16_t pin, uint8_t value) {
  TIM_OCInitTypeDef TIM_OCInitStructure;
 
  if (pin >= TOTAL_PINS || PIN_MAP[pin].timer_peripheral == NULL) {
    return;
  }
  // SPI safety check
  if (SPI.isEnabled() == true && (pin == SCK || pin == MOSI || pin == MISO)) {
    return;
  }
  // I2C safety check
  if (Wire.isEnabled() == true && (pin == SCL || pin == SDA)) {
    return;
  }
  // Serial1 safety check
  if (Serial1.isEnabled() == true && (pin == RX || pin == TX)) {
    return;
  }
  if (PIN_MAP[pin].pin_mode != OUTPUT && PIN_MAP[pin].pin_mode != AF_OUTPUT_PUSHPULL) {
    return;
  }
  // Don't re-init PWM and cause a glitch if already setup, just update duty cycle and return.
  if (PIN_MAP[pin].pin_mode == AF_OUTPUT_PUSHPULL) {
    TIM_OCInitStructure.TIM_Pulse = (uint16_t)(value * (TIM_ARR + 1) / 255);
    if (PIN_MAP[pin].timer_ch == TIM_Channel_1) {
      PIN_MAP[pin].timer_peripheral-> CCR1 = TIM_OCInitStructure.TIM_Pulse;
    } else if (PIN_MAP[pin].timer_ch == TIM_Channel_2) {
      PIN_MAP[pin].timer_peripheral-> CCR2 = TIM_OCInitStructure.TIM_Pulse;
    } else if (PIN_MAP[pin].timer_ch == TIM_Channel_3) {
      PIN_MAP[pin].timer_peripheral-> CCR3 = TIM_OCInitStructure.TIM_Pulse;
    } else if (PIN_MAP[pin].timer_ch == TIM_Channel_4) {
      PIN_MAP[pin].timer_peripheral-> CCR4 = TIM_OCInitStructure.TIM_Pulse;
    }
    return;
  }
 
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
 
  //PWM Frequency : PWM_FREQ (Hz)
  uint16_t TIM_Prescaler = (uint16_t)(SystemCoreClock / 24000000) - 1; //TIM Counter clock = 24MHz
 
  // TIM Channel Duty Cycle(%) = (TIM_CCR / TIM_ARR + 1) * 100
  uint16_t TIM_CCR = (uint16_t)(value * (TIM_ARR + 1) / 255);
 
  // AFIO clock enable
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
 
  //pinMode(pin, AF_OUTPUT_PUSHPULL); // we need to do this manually else we get a glitch
 
  // TIM clock enable
  if (PIN_MAP[pin].timer_peripheral == TIM2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  else if (PIN_MAP[pin].timer_peripheral == TIM3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  else if (PIN_MAP[pin].timer_peripheral == TIM4)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
 
  // Time base configuration
  TIM_TimeBaseStructure.TIM_Period = TIM_ARR;
  TIM_TimeBaseStructure.TIM_Prescaler = TIM_Prescaler;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
 
  TIM_TimeBaseInit(PIN_MAP[pin].timer_peripheral, & TIM_TimeBaseStructure);
 
  // PWM1 Mode configuration
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = TIM_CCR;
 
  if (PIN_MAP[pin].timer_ch == TIM_Channel_1) {
    // PWM1 Mode configuration: Channel1
    TIM_OC1Init(PIN_MAP[pin].timer_peripheral, & TIM_OCInitStructure);
    TIM_OC1PreloadConfig(PIN_MAP[pin].timer_peripheral, TIM_OCPreload_Enable);
  } else if (PIN_MAP[pin].timer_ch == TIM_Channel_2) {
    // PWM1 Mode configuration: Channel2
    TIM_OC2Init(PIN_MAP[pin].timer_peripheral, & TIM_OCInitStructure);
    TIM_OC2PreloadConfig(PIN_MAP[pin].timer_peripheral, TIM_OCPreload_Enable);
  } else if (PIN_MAP[pin].timer_ch == TIM_Channel_3) {
    // PWM1 Mode configuration: Channel3
    TIM_OC3Init(PIN_MAP[pin].timer_peripheral, & TIM_OCInitStructure);
    TIM_OC3PreloadConfig(PIN_MAP[pin].timer_peripheral, TIM_OCPreload_Enable);
  } else if (PIN_MAP[pin].timer_ch == TIM_Channel_4) {
    // PWM1 Mode configuration: Channel4
    TIM_OC4Init(PIN_MAP[pin].timer_peripheral, & TIM_OCInitStructure);
    TIM_OC4PreloadConfig(PIN_MAP[pin].timer_peripheral, TIM_OCPreload_Enable);
  }
 
  TIM_ARRPreloadConfig(PIN_MAP[pin].timer_peripheral, ENABLE);
 
  // TIM enable counter
  TIM_Cmd(PIN_MAP[pin].timer_peripheral, ENABLE);
}

void IRsend::sendNEC(unsigned long data, int nbits)
{
  enableIROut(38);
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(NEC_BIT_MARK);
      space(NEC_ONE_SPACE);
    } 
    else {
      mark(NEC_BIT_MARK);
      space(NEC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(NEC_BIT_MARK);
  space(0);
}

void IRsend::sendSony(unsigned long data, int nbits) {
  enableIROut(40);
  mark(SONY_HDR_MARK);
  space(SONY_HDR_SPACE);
  data = data << (32 - nbits);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(SONY_ONE_MARK);
      space(SONY_HDR_SPACE);
    } 
    else {
      mark(SONY_ZERO_MARK);
      space(SONY_HDR_SPACE);
    }
    data <<= 1;
  }
}

void IRsend::sendRaw(const uint16_t *buf, int len, int hz)
{
  enableIROut(hz);
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(buf[i]);
    } 
    else {
      mark(buf[i]);
    }
  }
  space(0); // Just to be sure
}

// Note: first bit must be a one (start bit)
void IRsend::sendRC5(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC5_T1); // First start bit
  space(RC5_T1); // Second start bit
  mark(RC5_T1); // Second start bit
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      space(RC5_T1); // 1 is space, then mark
      mark(RC5_T1);
    } 
    else {
      mark(RC5_T1);
      space(RC5_T1);
    }
    data <<= 1;
  }
  space(0); // Turn off at end
}

// Caller needs to take care of flipping the toggle bit
void IRsend::sendRC6(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC6_HDR_MARK);
  space(RC6_HDR_SPACE);
  mark(RC6_T1); // start bit
  space(RC6_T1);
  int t;
  for (int i = 0; i < nbits; i++) {
    if (i == 3) {
      // double-wide trailer bit
      t = 2 * RC6_T1;
    } 
    else {
      t = RC6_T1;
    }
    if (data & TOPBIT) {
      mark(t);
      space(t);
    } 
    else {
      space(t);
      mark(t);
    }

    data <<= 1;
  }
  space(0); // Turn off at end
}

/* Sharp and DISH support by Todd Treece ( http://unionbridge.org/design/ircommand )

The Dish send function needs to be repeated 4 times, and the Sharp function
has the necessary repeat built in because of the need to invert the signal.

Sharp protocol documentation:
http://www.sbprojects.com/knowledge/ir/sharp.htm

Here are the LIRC files that I found that seem to match the remote codes
from the oscilloscope:

Sharp LCD TV:
http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

DISH NETWORK (echostar 301):
http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx

For the DISH codes, only send the last for characters of the hex.
i.e. use 0x1C10 instead of 0x0000000000001C10 which is listed in the
linked LIRC file.
*/
void IRsend::sendSharp(unsigned long data, int nbits) {
  unsigned long invertdata = data ^ SHARP_TOGGLE_MASK;
  enableIROut(38);
  for (int i = 0; i < nbits; i++) {
    if (data & 0x4000) {
      mark(SHARP_BIT_MARK);
      space(SHARP_ONE_SPACE);
    }
    else {
      mark(SHARP_BIT_MARK);
      space(SHARP_ZERO_SPACE);
    }
    data <<= 1;
  }
  
  mark(SHARP_BIT_MARK);
  space(SHARP_ZERO_SPACE);
  delay(46);
  for (int i = 0; i < nbits; i++) {
    if (invertdata & 0x4000) {
      mark(SHARP_BIT_MARK);
      space(SHARP_ONE_SPACE);
    }
    else {
      mark(SHARP_BIT_MARK);
      space(SHARP_ZERO_SPACE);
    }
    invertdata <<= 1;
  }
  mark(SHARP_BIT_MARK);
  space(SHARP_ZERO_SPACE);
  delay(46);
}

void IRsend::sendDISH(unsigned long data, int nbits)
{
  enableIROut(56);
  mark(DISH_HDR_MARK);
  space(DISH_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & DISH_TOP_BIT) {
      mark(DISH_BIT_MARK);
      space(DISH_ONE_SPACE);
    }
    else {
      mark(DISH_BIT_MARK);
      space(DISH_ZERO_SPACE);
    }
    data <<= 1;
  }
}

void IRsend::sendPanasonic(unsigned int address, unsigned long data) {
    enableIROut(35);
    mark(PANASONIC_HDR_MARK);
    space(PANASONIC_HDR_SPACE);
    
    for(int i=0;i<16;i++)
    {
        mark(PANASONIC_BIT_MARK);
        if (address & 0x8000) {
            space(PANASONIC_ONE_SPACE);
        } else {
            space(PANASONIC_ZERO_SPACE);
        }
        address <<= 1;        
    }    
    for (int i=0; i < 32; i++) {
        mark(PANASONIC_BIT_MARK);
        if (data & TOPBIT) {
            space(PANASONIC_ONE_SPACE);
        } else {
            space(PANASONIC_ZERO_SPACE);
        }
        data <<= 1;
    }
    mark(PANASONIC_BIT_MARK);
    space(0);
}

void IRsend::sendJVC(unsigned long data, int nbits, int repeat)
{
    enableIROut(38);
    data = data << (32 - nbits);
    if (!repeat){
        mark(JVC_HDR_MARK);
        space(JVC_HDR_SPACE); 
    }
    for (int i = 0; i < nbits; i++) {
        if (data & TOPBIT) {
            mark(JVC_BIT_MARK);
            space(JVC_ONE_SPACE); 
        } 
        else {
            mark(JVC_BIT_MARK);
            space(JVC_ZERO_SPACE); 
        }
        data <<= 1;
    }
    mark(JVC_BIT_MARK);
    space(0);
}


void  IRsend::My_delay_uSecs(unsigned int T) {
    if(T){if(T>16000) {delayMicroseconds(T % 1000); delay(T/1000); } else delayMicroseconds(T);};
}



void IRsend::mark(int time) {
  // Sends an IR mark (frequency burst output) for the specified number of microseconds.
    analogWrite2 (irPin,128);
    My_delay_uSecs(time);

}

void IRsend::space(int time) {
  // Sends an IR space (no output) for the specified number of microseconds.
    analogWrite2 (irPin,0);
    My_delay_uSecs(time);
}

void IRsend::enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // MAX frequency is 166khz.
    pinMode (irPin, OUTPUT);
    TIM_ARR = (uint16_t)(24000000 / (khz*1000)) - 1; 
    analogWrite2 (irPin,128); // 50% Duty Cycle
    pinMode(irPin, AF_OUTPUT_PUSHPULL);
    space(0); // And turn off the PWM output until we need it
    delay(100);

}
