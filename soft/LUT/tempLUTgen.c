// this program generates a LUT for translation of ADC values into temperature in 0.25K increments
// needs to be compiled and run on the system

#include <stdio.h>
#include <math.h>

// temperature range, TEMP_MIN can be increased to save memory
#define TEMP_MIN -50 // Celsius
#define TEMP_MAX 125 // Celsius

// constant parameters 
#define NTC_B 3380.0 // B-value NTC
#define NTC_R25K 10.0 // R25 value NTC [kOhm]
#define RREF 2.4  // reference resistor [kOhm]

#define TEMP_MULTIPLIER 1 // factor for temp value (to increase resolution with integer values)


#define ZEROC_K 273.15 // 0C in Kelvin

double TempCalc(unsigned int adc) {

   double Temp;
   double Vntc;
   double Rntc;

// calculating temperature in Celsius in 2 steps  
    Rntc = RREF*(double)adc/(1024.0-(double)adc);
    Temp = 1.0/(1.0/(25.0+ZEROC_K)-log(NTC_R25K/Rntc)/NTC_B)-ZEROC_K;
    return Temp;
}

int main (void) {

  unsigned int Vntcadc;
  double Temp;
  unsigned int Vntcadcmin =0;
  unsigned int Vntcadcmax =0;	

  printf("#ifndef TEMPLUT_H\n");  
  printf("#define TEMPLUT_H\n");
  printf("const int16_t NTC_LUT[] PROGMEM = {");
  for (Vntcadc=0;Vntcadc<1024;Vntcadc++) {
    
// calculating temperature 
    Temp = TempCalc(Vntcadc);
 
    if ((Temp > (double)TEMP_MIN ) && (Temp < (double)TEMP_MAX)) { // limit temperature range to save memory
      if (!Vntcadcmin) {
        Vntcadcmin = Vntcadc;
      } else { 
        printf(",");
      }; 
        
      printf("%.0f",Temp*(double)TEMP_MULTIPLIER);
      Vntcadcmax = Vntcadc;	
    }		
  } 
  printf("};\n");
  printf("#define NTCADC_MIN %d\n",Vntcadcmin);	
  printf("#define NTCADC_MAX %d\n",Vntcadcmax);
  printf("#define TEMP_MIN %d\n",TEMP_MIN);
  printf("#define TEMP_MAX %d\n",TEMP_MAX);
  printf("#define TEMP_MULTIPLIER %d\n",TEMP_MULTIPLIER);

	
  printf("#endif\n");  
 
  return(0);
}
