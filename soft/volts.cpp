#include <avr/io.h>
#include <avr/pgmspace.h>

#include <Arduino.h>
#include <math.h>

#include "voltsLUT.h"

#define AVGEXP 6 // max
#define AVGS (int)pow((double)2,(double)AVGEXP)
#define LSBMASK 0xFF>>(16-AVGEXP)
 

int volts(const unsigned char adc_pin) {      
  
  unsigned int adc,adcavg;
  unsigned int Volts;
  unsigned int index;
  unsigned int avgs;

// set our own prescaler to 128 = 125kHz @ 16MHz CPU clock, enable ADC, >200 kHz to get max. resolution
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
 
// reading ADC 
//  Vntcadc = analogRead(VNTC_PIN); // first measurement needs to be dumped according to data sheet, only required if internal bandgap (reference) is used
  adc = 0;
  for (avgs=0;avgs<AVGS;avgs++) {adc += analogRead(adc_pin); delay(1);} // conversion takes about 100 us (13 * 1/128 kHz) + 1 ms delay (for averaging)

  ADCSRA = ~_BV(ADEN) & (_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0)); // disable ADC, but keep prescaler set
 
  adcavg=adc>>AVGEXP;
  if ((adc&LSBMASK)>(AVGS/2-1)) adcavg+=1;

// read from LUT     
  if ((adcavg >= VOLTSADC_MIN) && (adcavg <= VOLTSADC_MAX)) {
    index = adcavg-VOLTSADC_MIN;
    Volts = pgm_read_word_near(VOLTS_LUT + index); 
  } else if (adcavg < VOLTSADC_MIN) { // above max, 
    Volts = VOLTS_MIN;
  } else { // below min
    Volts = VOLTS_MAX;
  }
 

 return Volts;
  
}
