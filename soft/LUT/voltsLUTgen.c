// this program generates a LUT for translation of ADC values into temperature in 0.25K increments
// needs to be compiled and run on the system

#include <stdio.h>

// temperature range, TEMP_MIN can be increased to save memory
#define VOLTS_MIN 9 // Volts
#define VOLTS_MAX 16 // Volts

// constant parameters 
#define R1 43.2  // R2 [kOhm]
#define R2 8.25  // R2 [kOhm]
#define VREF 2.51 // reference voltage in volts (measured)
        
#define VOLTS_MULTIPLIER 10 // factor for temp value (to increase resolution with integer values)

double VoltsCalc(unsigned int adc) {

   double Volts;

//  VBAT >---[ R1 ]--x--[ R2 ] ---|
//                   |
//                   | VADC
//                   V
//  VREF >--------[ ADC ]--/--> 10 bit value 
//
//VADC/R2 = VBAT/(R1+R2) --> VADC/R2*(R1+R2) = VBAT
//
//ADC/1024 = VADC/VREF --> ADC/1024*VREF = VADC
// 
//
  return (double)adc/1024.0*VREF/R2*(R1+R2);

}

int main (void) {

  unsigned int adc;
  double Volts;
  unsigned int adcmin =0;
  unsigned int adcmax =0;	

  printf("#ifndef VOLTSLUT_H\n");  
  printf("#define VOLTSLUT_H\n");
  printf("const int16_t VOLTS_LUT[] PROGMEM = {");
  for (adc=0;adc<1024;adc++) {
    
// calculating temperature 
    Volts = VoltsCalc(adc);
 
    if ((Volts > (double)VOLTS_MIN ) && (Volts < (double)VOLTS_MAX)) { // limit temperature range
      if (!adcmin) {
        adcmin = adc;
      } else { 
        printf(",");
      }; 
        
      printf("%.0f",Volts*(double)VOLTS_MULTIPLIER);
      adcmax = adc;	
    }		
  } 
  printf("};\n");
  printf("#define VOLTSADC_MIN %d\n",adcmin);	
  printf("#define VOLTSADC_MAX %d\n",adcmax);
  printf("#define VOLTS_MIN %d\n",VOLTS_MIN);
  printf("#define VOLTS_MAX %d\n",VOLTS_MAX);
  printf("#define VOLTS_MULTIPLIER %d\n",VOLTS_MULTIPLIER);

	
  printf("#endif\n");  
 
  return(0);
}
