#include <avr/io.h>
#include <avr/pgmspace.h>

#include <Arduino.h>

#include "tempLUT.h"

//int temp(unsigned char ntc_pin) {                

int temp(const unsigned char adc_pin) {      
  
  unsigned int adc;
  unsigned int Temp;
  unsigned int index;
  
// set our own prescaler to 128 = 125kHz @ 16MHz CPU clock, enable ADC, >200 kHz to get max. resolution
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
 
// reading ADC 
  adc = analogRead(adc_pin); // conversion takes about 100 us (13 * 1/128 kHz)
 
  ADCSRA = ~_BV(ADEN) & (_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0)); // disable ADC, but keep prescaler set
 

// read from LUT     
  if ((adc >= NTCADC_MIN) && (adc <= NTCADC_MAX)) {
    index = adc-NTCADC_MIN;
    Temp = pgm_read_word_near(NTC_LUT + index); 
  } else if (adc < NTCADC_MIN) { // above max, ADC voltage is invers to temperature
    Temp = TEMP_MAX;
  } else { // below min
    Temp = TEMP_MIN;
  }
 

 return Temp;
  
}
