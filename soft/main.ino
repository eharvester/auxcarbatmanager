// AuxBatMonitor
#include <LowPower.h>
#include <talkie.h>
Talkie voice;
#include "voicedata.h"
#include "tempLUT.h"

#define VOICEDEBUG 0

// pin definitions
#define LED_PIN A4
#define SW_PIN A3
#define AUXVON_PIN A2 
#define NTC_PIN A1
#define REFEN_PIN A0 
#define VBATDIV_PIN A6 
#define VOICE_PIN 3
#define TX_PIN 1
#define RX_PIN 0

// battery voltages
#define VBATALLOWED_MAX 14.8
#define VBATALLOWED_MIN 10.8

#define VBATCHARGING_MIN 13.4

#define VBAT100_PCT 12.7
#define VBAT90_PCT 12.5
#define VBAT80_PCT 12.4
#define VBAT70_PCT 12.3
#define VBAT60_PCT 12.2
#define VBAT50_PCT 12.1
#define VBAT40_PCT 11.9
#define VBAT30_PCT 11.8
#define VBAT20_PCT 11.6
#define VBAT10_PCT 11.3

#define VBAT_HYST 0.4

// battery allowed temps
#define TBAT_MAX 50
#define TBAT_MIN -20

// hours auto off
#define AUTOOFF_H 12 

#define ADCREFSETTLE_MS 10
#define LEDFLASH_MS 100

// timers
#define SLEEPPERIOD SLEEP_250MS

#define SLEEPPERIOD_S 0.256 // it is actually 0.256s
#define F128_KHZ 120.0 // ~~120 kHz (measured)
 
// crystal startup time (after every sleep cycle) = 4.1 ms
#define CRYSTAL_STARTUP_S 0.0041

//LED flashing (after every 64 sleep cycles) = 100 ms
#define LEDFLASH_S (double(LEDFLASH_MS)/1000.0)
 
// ADC supply voltage settle (after every 64 sleep cycles) = 10 ms 
#define ADCREFSETTLE_S (double(ADCREFSETTLE_MS)/1000.0)

// ADC conversion (after every 64 sleep cycles) = 0.1 ms
// ADC averages (after every 64 sleep cycles) = 64
#define ADCCONVERSION_S 0.0064

#define SLEEPCOUNTER 64

#define AUTOOFF_S (int)((double)AUTOOFF_H*3600.0/((double(SLEEPCOUNTER)*(SLEEPPERIOD_S+CRYSTAL_STARTUP_S)*128.0/F128_KHZ+LEDFLASH_S+ADCREFSETTLE_S+ADCCONVERSION_S))) 

#define LOWBATWARNINGOFF_C 10
#define WARNINGOFF_C 10


// macros
#define LEDON digitalWrite(LED_PIN, HIGH)  
#define LEDOFF digitalWrite(LED_PIN, LOW) 

#define AUXON digitalWrite(AUXVON_PIN, HIGH)
#define AUXOFF digitalWrite(AUXVON_PIN,LOW)

#define VOICEOFFDELAY_MS 100

#define VOICEON pinMode(VOICE_PIN, OUTPUT) 
#define VOICEOFF {delay(VOICEOFFDELAY_MS); pinMode(VOICE_PIN, INPUT);} // disable drive to reduce power consumption

#define say(DATA) voice.say((uint8_t*)DATA) //progmen data are const but voice.say requires non-const

extern int temp(const unsigned char); 
extern int volts(const unsigned char); 

// says number 0..9
void say0_9(int val) {
  
 switch (val) {
  case 9: say(spNINE); break; 
  case 8: say(spEIGHT); break;
  case 7: say(spSEVEN); break;
  case 6: say(spSIX); break;
  case 5: say(spFIVE); break;
  case 4: say(spFOUR); break;
  case 3: say(spTHREE); break;
  case 2: say(spTWO); break;
  case 1: say(spONE); break;
  case 0: say(spZERO); break;
  default: break;
 }
}

// says number val = value, point = position of decimal point, 0 = no decimal point, 1 = 1 decimal point aso. 
// max 3 decimal points
// max number is 9,999
void sayNumber(int val, byte point) {

 int whole, thousands,hundreds, tens, rem;

 if (val < 0) {say(spMINUS); val = abs(val);}
 switch (point) {
  case 3: whole = val/1000; rem = val % 1000; break; 
  case 2: whole = val/100; rem = val % 100; break;
  case 1: whole = val/10; rem = val % 10; break;
  case 0: whole = val; break;
  default: whole = val; break;
 }

 if (!whole) say0_9(0); // only if just zero, say zero
 
 thousands = whole/1000;
 if (thousands) {say0_9(thousands); say(spTHOUSAND); whole -= thousands*1000;}
 hundreds = whole/100;
 if (hundreds) {say0_9(hundreds); say(spHUNDRED); whole -= hundreds*100;}
 tens = whole/10;
 if (tens) {
  if (tens == 5) {say(spFIF_); say(sp_TY); whole -= tens*10;}
  else if (tens == 3) {say(spTHIR_); say(sp_TY); whole -= tens*10;}
  else if (tens == 2) {say(spTWENTY); whole -= tens*10;}
  else if (tens != 1) {say0_9(tens); say(sp_TY); whole -= tens*10;}  
  else if (tens == 1) {
   if (whole == 15) { say(spFIF_); say(sp_TEEN);}
   else if (whole == 13) {say(spTHIR_); say(sp_TEEN);}
   else if (whole == 12) say(spTWELVE); 
   else if (whole == 11) say(spELEVEN);
   else if (whole == 10) say(spTEN);
   else {say0_9(whole-10); say(sp_TEEN);}
   whole = 0;
  }
 }
 if (whole) say0_9(whole);
 if (point) { 
  say(spPOINT); 
  if (point > 2) {hundreds = rem/100; say0_9(hundreds); rem -= hundreds*100;}
  if (point > 1) {tens = rem/10; say0_9(tens); rem -= tens*10;}
  say0_9(rem);
 }

}

// says temperature and warns of if temp is out of range
void sayTemperature(int val) {

 VOICEON;  
 if ((val == TEMP_MIN) || (val == TEMP_MAX))  { say(spFAIL); say(spCHECK); say(spN); say(spT); say(spC);}
 else {
  say(spTEMPERATURE);
  sayNumber(val,0);
  say(spDEGREES); 
 }
 if (val > TBAT_MAX) { say(spALERT); say(spOVER); say(spTEMPERATURE);}
 else if (val < TBAT_MIN) { say(spALERT); say(spUNDER); say(spTEMPERATURE);}
 VOICEOFF; 
}


// says voltage or warns of over/under voltage 
void sayVoltage(int volts) {

 VOICEON;
 sayNumber(volts,1);   say(spVOLTS);  
 if (volts > VBATALLOWED_MAX*10) { say(spALERT); say(spOVER); say(spVOLTS);}
 else if (volts < VBATALLOWED_MIN*10) { say(spALERT); say(spUNDER); say(spVOLTS);}
 else {
  if (volts >= VBAT100_PCT*10) sayNumber(100,0);
  else if (volts >= VBAT90_PCT*10) sayNumber(90,0);
  else if (volts >= VBAT80_PCT*10) sayNumber(80,0);
  else if (volts >= VBAT70_PCT*10) sayNumber(70,0);
  else if (volts >= VBAT60_PCT*10) sayNumber(60,0);
  else if (volts >= VBAT50_PCT*10) sayNumber(50,0);
  else if (volts >= VBAT40_PCT*10) sayNumber(40,0);
  else if (volts >= VBAT30_PCT*10) sayNumber(30,0);
  else if (volts >= VBAT20_PCT*10) sayNumber(20,0);
  else if (volts >= VBAT10_PCT*10) sayNumber(10,0);
  else sayNumber(0,0);
  say(spPERCENT);  
 }
 VOICEOFF; 
}

#define sayLowBatteryWarning(countdown) {VOICEON; say(spALERT); say(spLOW); say(spVOLTS); say(spPOWER); say(spOFF); sayNumber(countdown,0); VOICEOFF;} 
#define sayTimerOffWarning(countdown) {VOICEON; say(spCAUTION); say(spPOWER); say(spOFF); sayNumber(countdown,0); VOICEOFF;} 
#define sayTest {VOICEON; say(spTEST); VOICEOFF;} 
#define sayMotorOn {VOICEON; say(spMOTOR); say(spON); VOICEOFF;} 
#define sayMotorOff {VOICEON; say(spMOTOR); say(spOFF); VOICEOFF;}
#define sayManualOn {VOICEON; say(spMANUAL); say(spON); VOICEOFF;} 
#define sayManualOff {VOICEON; say(spMANUAL); say(spOFF); VOICEOFF;}



int getTemp (void) {
 int celsius;
 digitalWrite(REFEN_PIN, HIGH); // voltage reference on
 delay(ADCREFSETTLE_MS); // allow 1uF cap voltage to settle (measure!)
 celsius = temp(NTC_PIN);
 digitalWrite(REFEN_PIN, LOW); // voltage reference off
 return celsius;
}

int getTenthvolts (void) {
 int tenthvolts; 
 digitalWrite(REFEN_PIN, HIGH); // voltage reference on
 delay(ADCREFSETTLE_MS); // allow 1uF cap voltage to settle (measure!)
 tenthvolts = volts(VBATDIV_PIN);
 digitalWrite(REFEN_PIN, LOW); // voltage reference off
 return tenthvolts;
}

// FSMs
// control FSM
#define INIT 0
#define TEST 1
#define GOOD 2
#define LOWBAT 3
#define MOTORON 4
#define MOTOROFF 5
#define MANUALON 6
#define MANUALOFF 11
#define CHARGING 7
#define ONTIMER 8
#define LOWBATWARNING 9
#define OFFWARNING 10
#define MANUALONOFFSW 12
#define MANUALOFFOFFSW 13

byte BatFsm = INIT;

int sleepperiodtimer = 0;
int warningtimer = 0;
int sleepcounter = 0;

int celsius = 0;
int tenthvolts = 0;

bool CtrlButton = false;
bool TestButton = false;


void setup() {

 pinMode(SW_PIN,INPUT);
 digitalWrite(SW_PIN, HIGH); //pull up   

 pinMode(RX_PIN,INPUT);
 digitalWrite(RX_PIN, HIGH); //pull up   

 pinMode(LED_PIN,OUTPUT); 
 LEDOFF; 
  
 pinMode(TX_PIN,OUTPUT);
 digitalWrite(TX_PIN, LOW);

 pinMode(AUXVON_PIN,OUTPUT);
 digitalWrite(AUXVON_PIN, LOW);

 pinMode(REFEN_PIN,OUTPUT);
 digitalWrite(REFEN_PIN, LOW);
 analogReference(EXTERNAL);

#if VOICEDEBUG==1
 VOICEON; sayNumber(AUTOOFF_S,0); VOICEOFF;
#endif

}

void loop() {

// FSM
 switch (BatFsm) {
  case INIT:
   AUXOFF;
   LEDOFF;
   BatFsm = TEST;
   sleepperiodtimer=0;
   warningtimer=0;
   break;

  case TEST:
   AUXOFF;
   LEDOFF;
   celsius = getTemp();
   tenthvolts = getTenthvolts();
   sayTest; sayTemperature(celsius);sayVoltage(tenthvolts);
   if (tenthvolts >= VBAT30_PCT*10) BatFsm = GOOD;
   else BatFsm = LOWBAT;
   sleepperiodtimer=0;
   warningtimer=0;
   break;

  case GOOD:
   AUXOFF;
   LEDON; delay(LEDFLASH_MS); LEDOFF; 
   tenthvolts = getTenthvolts();
   if (tenthvolts < VBAT30_PCT*10) BatFsm = LOWBAT;
   else if (tenthvolts >= (VBATCHARGING_MIN)*10) BatFsm = MOTORON;
   else if (CtrlButton) BatFsm = MANUALON; // manual on (low active)
   sleepperiodtimer=0;
   warningtimer=0;
   break;

 case LOWBAT:
   AUXOFF;
   LEDOFF;
   tenthvolts = getTenthvolts();
   if (tenthvolts >= VBAT30_PCT*10) BatFsm = GOOD; // if battery recovers or gets charged
   sleepperiodtimer=0;
   warningtimer=0;
   break;

  case MOTORON:
   AUXON;
   LEDON;
   celsius = getTemp();
   tenthvolts = getTenthvolts();
   sayMotorOn;sayTemperature(celsius);sayVoltage(tenthvolts);
 
   BatFsm = CHARGING;
   sleepperiodtimer=0;
   warningtimer=0;
   break;

  case MANUALON:
   AUXON;
   LEDON;
   celsius = getTemp();
   tenthvolts = getTenthvolts();
   sayManualOn;sayTemperature(celsius);sayVoltage(tenthvolts);
   BatFsm = MANUALONOFFSW;
   sleepperiodtimer=0;
   warningtimer=0;
   break;


  case MANUALONOFFSW:
   AUXON;
   LEDON;
   if (!CtrlButton) BatFsm = ONTIMER;
   sleepperiodtimer=0;
   warningtimer=0;
   break;

  case CHARGING:
   AUXON;
   LEDON; 
   tenthvolts = getTenthvolts();
   if (tenthvolts < (VBATCHARGING_MIN-VBAT_HYST)*10) BatFsm = MOTOROFF; // motor off, battery runs on timer 
   sleepperiodtimer=0;
   warningtimer=0;
   break;

  case MOTOROFF:
   AUXON;
   LEDON;
   sayMotorOff;
   BatFsm = ONTIMER;
   sleepperiodtimer=0;
   warningtimer=0;
   break;

 case ONTIMER:
   AUXON;
   LEDON;
   tenthvolts = getTenthvolts();
   if (tenthvolts < VBATALLOWED_MIN*10) BatFsm = LOWBATWARNING; // low battery
   else if (CtrlButton) BatFsm = MANUALOFF; // manual off 
   else if (sleepperiodtimer >= AUTOOFF_S) BatFsm = OFFWARNING; // time out 
   else if (tenthvolts >= (VBATCHARGING_MIN)*10) BatFsm = MOTORON; // charging again
   sleepperiodtimer++;
   warningtimer=0;
   break;

  case MANUALOFF:
   AUXOFF;
   LEDOFF;
   sayManualOff;
   BatFsm =  MANUALOFFOFFSW;
   sleepperiodtimer=0;
   warningtimer=0;
   break;

 case MANUALOFFOFFSW:
   AUXOFF;
   LEDOFF;
   if (!CtrlButton) BatFsm = GOOD; // check switched released
   sleepperiodtimer=0;
   warningtimer=0;
   break;


 case LOWBATWARNING:
   AUXON;
   LEDON; 
   sayLowBatteryWarning(warningtimer-LOWBATWARNINGOFF_C);
   tenthvolts = getTenthvolts();
   if (tenthvolts >= (VBATCHARGING_MIN)*10) BatFsm = MOTORON; // charging again
   else if (tenthvolts >= VBAT30_PCT*10) BatFsm = ONTIMER;  // back on timer in case voltage recovers
   else if (warningtimer >= LOWBATWARNINGOFF_C) BatFsm = LOWBAT; //warning timer elapsed
   else if (CtrlButton) BatFsm = LOWBAT; // manual off
   sleepperiodtimer++; 
   warningtimer++;
   break;
  
 case OFFWARNING:
   AUXON;
   LEDON;
   sayTimerOffWarning(warningtimer-WARNINGOFF_C);
   tenthvolts = getTenthvolts();
   if (tenthvolts >= (VBATCHARGING_MIN)*10) BatFsm = MOTORON; // charging again
   else if (CtrlButton) BatFsm = MANUALON;  // turn on manually again
   else if (warningtimer >= WARNINGOFF_C) BatFsm = GOOD; // warning timer elapsed
   sleepperiodtimer=0;
   warningtimer++;
   break;

  default:
  break;
 }


 CtrlButton = digitalRead(SW_PIN)?false:true; 
 TestButton = digitalRead(RX_PIN)?false:true; 

// delay timer control
 
 switch (BatFsm) {
  case INIT:
  case TEST:
  case MOTORON:
  case MANUALON:
  case MOTOROFF:
  case MANUALOFF:
   LowPower.powerDown(SLEEPPERIOD, ADC_OFF, BOD_OFF);  
   break;

  case GOOD:
  case LOWBAT:
  case ONTIMER:
  case LOWBATWARNING:
  case OFFWARNING:
  case CHARGING:
   for(sleepcounter=0; !CtrlButton && !TestButton && (sleepcounter<SLEEPCOUNTER); sleepcounter++) {
     LowPower.powerDown(SLEEPPERIOD, ADC_OFF, BOD_OFF);
     CtrlButton = digitalRead(SW_PIN)?false:true; 
     TestButton = digitalRead(RX_PIN)?false:true;
   }
   break;

  default:
  break;
 }

 if (TestButton) {
  celsius = getTemp();
  tenthvolts = getTenthvolts();
  sayTest; sayTemperature(celsius);sayVoltage(tenthvolts);
 }

 
}
