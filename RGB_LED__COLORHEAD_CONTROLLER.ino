/*##########################################################
  COLORHEAD LIGHT SOURCE AND TIMER V. 1.2.1
  By Marco Glattfelder, Tuggen, Switzerland, July 2020
############################################################
Changelog: (5.12.2019)
- Added IR control. Maybe not really needed, but I had it in the older code. 
- Added EEPROM library to store last YMC values and retrieve them at startup
Changelog: (21.7.2020)
- Added B/W only options and parameter section
Changelog: (26.7.2020)
- Added 0.1s option 

This is a coltroller for a NeoPixel matrix with WS2812B LED
Wavelenghts: Red: 620-625nm / Green 522-525nm / Blue 465-467nm

You need following Libraries: 
Keypad: https://github.com/Chris--A/Keypad
Neopixel: https://github.com/adafruit/Adafruit_NeoPixel
(if nor present) Wire: https://github.com/esp8266/Arduino/tree/master/libraries/Wire
Grove RGB LCD: https://github.com/Seeed-Studio/Grove_LCD_RGB_Backlight
IR (Optional is IR remote is wanted): https://github.com/z3t0/Arduino-IRremote

Bill of material:
- Arduino Mega (1280 or 2560). Can be found everywhere on Aliexpress, eBay or so
- Neopixel Matrix. Original of Adafruit is quiet expansive. WS2812b matrix clones can be found everywhere on Aliexpress, eBay or so
- Grove RGB LCD: At Seedstudio .https://www.seeedstudio.com/Grove-LCD-RGB-Backlight.html or Google it
- 4x4 foil Keypad. Can be found everywhere on Aliexpress, eBay or so. Please check with layout in the hookup drawing.
- 3x (on)-off-(on) toggle switches or 6 push button switches. For the (on)-off-(on) switch, the switch should go back to middle position once relased.
- 3x on-on toggle switches
- 1x push buttons normally open
- A suitable 5V stabilized power supply for the LED Matrix. Be sure it's enough Ampère. A 256 Matrix needs at least 75-100Watt resp 12-20A. 
 
 Control of the LCD Menu and switches:
 - With one toggle switch you can set the light to be continuosly on (see hookup drawing)
 - LCD backlight can be set with a toggle switch (see hookup drawing)
 
 Default mode:
 - set the timer value with the keyboard
 - The '*' key erases the time and sets to 0
 - Use the 3 toggle switches to rise or lower the YMC values. Each tip is about +1 or -1 in value.
 - The button starts the timer
 - The 'A' key gives access to the YMC Keypad enter mode
 - The 'B' key gives access to the RGB Keypad enter mode
 - The 'C' key gives access to the B/W Contrast control
 - The 'D' key gives access to the EEPROM memory for writing an retieving the YMC values. Up to 400 storage places can be set and read.
 - The '#' key toggles between 0.1s and 1s increment mode
 
 YMC Keypad enter mode (Press 'A' from default mode)
 - The timer cannot be operated in this mode
 - Set a value from 0 to 255 with the keypad
 - The '*' key erases the values and sets to 0
 - The 'A' assigns the entered value to the Y chanell. This value is permament if you change the mode to default.
 - The 'B' assigns the entered value to the M chanell. This value is permament if you change the mode to default.
 - The 'C' assigns the entered value to the C chanell. This value is permament if you change the mode to default.
 - The '#' key sends you back to the default mode
 
RGB Keypad enter mode (Press 'B' from default mode)
 - The timer cannot be operated in this mode
 - Set a value from 0 to 255 with the keypad
 - The '*' key erases the values and sets to 0
 - The 'A' assigns the entered value to the R chanell. This value is permament if you change the mode to default.
 - The 'B' assigns the entered value to the G chanell. This value is permament if you change the mode to default.
 - The 'C' assigns the entered value to the B chanell. This value is permament if you change the mode to default.
 - The '#' key sends you back to the default mode
 
 Conrast control or B/W Mode (Press 'C' from default mode)
 - set the timer value with the keyboard
 - The timer CAN be used and set in this mode
 - The '*' key erases the time and sets to 0
 - The 'A' increases the contrast value 
 - The 'B' lowers the contrast value
 - The '#' key sends you back to the default mode
 
  EEPROM memory YMC storage Mode (Press 'D' from default mode)
  - Set a value from 0 to 400 for  the storage place
  - The 'A' writes the actual YMC values to the selected EEPROM storage place 
  - The 'B' retrieves YMC values from the selected EEPROM storage place assign these to the actual YMC values 
  - The '*' key erases the values and sets to 0
  - The '#' key sends you back to the default mode
 
###########################################################*/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#include <Keypad.h>
#include <Wire.h>
#include "rgb_lcd.h"
#include <EEPROM.h>
#define PIN        6 // Neopixel Pin

//-------------------------------------------------------------------------------------------------------------------------
//-------------------------- PARAMETERS TO BE CHANGED ---------------------------------------------------------------------

#define NUMPIXELS 16 // Amount of the LED on the matrix. Change it if you have a smaller matrix or daisy chained more matrix
int operate_mode = 0; // Start mode. Set 0 if you want to start with color mode. Set 3 if you want to start with B/W mode
int no_color = 0; // set this if want to stay only in B/W mode. This is the case if your enclosure is compact. 
int rgb_min = 1;  // min brightness for the LCD luminosity switch position 1
int rgb_mid = 20; // mid brightness for the LCD luminosity switch position 2
int color_min = 0; // color for the min position for the LCD luminosity switch position 1 -> 1=green or 0=red
int color_mid = 0; // color for the min position for the LCD luminosity switch position 2 -> 1=green or 0=red
int color_def = 0; // Start color fot the LCD -> 1=green or 0=red

//------------------------- END PARAMETERS TO BE CHANGED -------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------

//################# DO NOT CHANGE NOTHING FROM HERE ON IF YOU ARE NOT AWARE OF WHAT TO DO ##################################

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// RGB Values for contrast control
int multival[10][3]={
   {0,255,0},
   {0,225,30},  
   {0,200,60},  
   {0,170,85},
   {0,143,115},
   {0,115,143},
   {0,85,170},
   {0,60,200},
   {0,30,225},
   {0,0,255}    
   };

int contrast = 5;

const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {22, 24, 26, 28}; 
byte colPins[COLS] = {30, 32, 34, 36};

const byte ROWS_1 = 1; //four rows
const byte COLS_1 = 6; //three columns
char keys_1[ROWS_1][COLS_1] = {
  {'a','b','c','d','e','f'},
};
byte rowPins_1[ROWS_1] = {50}; 
//byte colPins_1[COLS_1] = {38,40,42,44,46,48}; 
byte colPins_1[COLS_1] = {48,46,44,42,40,38}; 
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
Keypad keypad2 = Keypad( makeKeymap(keys_1), rowPins_1, colPins_1, ROWS_1, COLS_1 );

char customKey;
unsigned long previousMillis_1 = 0; 
long interval_1 = 1000;

int switch_changed = 0;
int switch_changed_2 = 0;
int toggle_timer = 0;
int default_interval;
int default_time;
int default_interval_pause;
int multitime = 0;

int actual_time = 5;
int former_time = 0;
int starttime = 0;
int start_interval = 1;
int static_l = 0;
rgb_lcd lcd;

 int colorR = 0;
 int color_changed = 0;
 int colorG = 40;
 int colorB = 0; 

//int operate_mode = 0;

int redlight = 0;

int brightness = 255;

int val_r = 0;
int val_g = 0;
int val_b = 0;

int val_b_s = 230; // This is the default Y start value to meet the  start color temperature
int val_g_s = 150; // This is the default M start value to meet the  start color temperature

int toggle_switch = 0;

int split_mode = 0;
int split_run = 0;

String debug_1 = "";
String debug_1_old = "";
String stringval = "";

void setup(){ 
  pixels.begin(); 
  pixels.clear();
  for(int i=0; i<NUMPIXELS; i++)
  { 
  uint32_t gammacolor = pixels.Color(0, 0, 0);pixels.setPixelColor(i, gammacolor);delay(1);
  }
  // initialized the toggle switch inputs
   pinMode(45,INPUT_PULLUP);
  pinMode(47,INPUT_PULLUP);      
  pinMode(49,INPUT_PULLUP);
  pinMode(51,INPUT_PULLUP);
  pinMode(53,INPUT_PULLUP);
  Serial.begin(9600);
  // LCD Display init
  lcd.begin(16, 2);
  if(color_def == 1){lcd.setRGB(colorR, colorG, colorB);}
  if(color_def == 0){lcd.setRGB(colorG, colorR , colorB);}
 // Sets LED panel to max brightness 
 
 val_r = brightness;
 val_g = brightness;
 val_b = brightness;
 
 val_r = EEPROM.read(0); 
 val_g = EEPROM.read(1); 
 val_b = EEPROM.read(2);
 
 PaintLED(val_r,val_g,val_b,"noshow");


Serial.println("EEPROM 0:" + (String)EEPROM.read(0));
}
  
void loop(){
  unsigned long currentMillis = millis();
  int mult = brightness / 25;
  int  s_1 = digitalRead(47); // Min LCD Backlight state
  int  s_2 = digitalRead(53);// Start timer state
  int s_3 = digitalRead(49);// Mid resp. red b/w LCD Backlight state
  int  s_4 = digitalRead(51);// LED continuosly on
  int s_5 = digitalRead(45);// RedLight

          if (s_5==0)
              {redlight = 1;}
              else
              {redlight = 0;}

  //Serial.println((String)s_1 + " - " + (String)s_2 + " - " + (String)s_3 + " - " + (String)s_4 + " - " + (String)s_5);
  //delay(2000);

 // Check if LED should be switched on
  if (s_4 == 0) 
  {
  if (switch_changed==0) 
     {  
         if (operate_mode!=3)
         {
         PaintLED(val_r,val_g,val_b,"Show");pixels.show();
         }
         else
         {
         PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"Show");pixels.show();      
         }  
         switch_changed=1;Serial.println("Show");
         static_l=1;
     }
  }
  else
  {
  if (switch_changed==1) {if (starttime != 1){pixels.clear();pixels.show();switch_changed=0;Serial.println("Clear");static_l=0;}}
  }
 
  // Start timer check
  if (s_2 == 0) 
  {
  if (starttime != 1){StartTimer();}
  }

  // Set background colors of LCD
  if (s_1 == 0) 
  {
  if (color_changed==0) {if (color_min == 1){lcd.setRGB(colorR, rgb_min, colorB);color_changed=1;}else{lcd.setRGB(rgb_min, colorR, colorB);color_changed=1;}}
  } 
  else
  {
    if (s_3 == 0) 
    {
    if (color_changed==0) {if (color_mid == 1){lcd.setRGB(colorR, rgb_mid, colorB);color_changed=1;}else{lcd.setRGB(rgb_mid, colorR, colorB);color_changed=1;}}
    //if (color_changed==0) {lcd.setRGB( rgb_mid,colorG, colorB);color_changed=1;}
    }
    else
    {
    if (color_changed==1) {
      //lcd.setRGB(colorR, colorG, colorB);color_changed=0;
        if(color_def == 1){lcd.setRGB(colorR, colorG, colorB);color_changed=0;}
        if(color_def == 0){lcd.setRGB(colorG, colorR , colorB);color_changed=0;}
      }
    }
  }

  customKey = keypad.getKey(); 
  char key2 = keypad2.getKey();

    if (key2)
    {   if (operate_mode == 3)
       {
        if (key2=='f') {if (contrast<9){contrast++;};customKey=(char)0;PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"noshow");}
        if (key2=='e') {if (contrast>0){contrast--;};customKey=(char)0;PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"noshow");} 
        if (key2=='d') {actual_time++;customKey=(char)0;PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"noshow");}
        if (key2=='c') {if (actual_time>0){actual_time--;};customKey=(char)0;PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"noshow");} 
        //actual_time
       }
       else
       {
        if (key2=='e'){if (val_b<brightness){val_b = val_b + 1 ;PaintLED(val_r,val_g,val_b,"noshow");}}
        if (key2=='f'){if (val_b>0){val_b = val_b - 1 ;PaintLED(val_r,val_g,val_b,"noshow");}}
        if (key2=='c'){if (val_g<brightness){val_g=val_g+5;PaintLED(val_r,val_g,val_b,"noshow");}}
        if (key2=='d'){if (val_g>0){val_g=val_g-5;PaintLED(val_r,val_g,val_b,"noshow");}} 
        if (key2=='a'){if (val_r<brightness){val_r = val_r + 10;PaintLED(val_r,val_g,val_b,"noshow");}}
        if (key2=='b'){if (val_r>0){val_r = val_r - 10;PaintLED(val_r,val_g,val_b,"noshow");}}
       }

    }
    
  
    if (customKey) 
    {

    if (operate_mode==0) 
    {
    //toggle_timer
    if (customKey== '#'){if (toggle_timer==0){toggle_timer=1;interval_1=100;}else{toggle_timer=0;interval_1=1000;};customKey=(char)0;Serial.println("Toggle timer: " + (String)toggle_timer); }
    
    if (customKey== '*'){stringval="";customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");} // Deletes the time  
    if (customKey== 'A'){operate_mode = 1;former_time = actual_time;stringval="";ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;} // Goes in manual enter mode of the YMC vallues by keypad
    if (customKey== 'B'){operate_mode = 2;former_time = actual_time;stringval="";ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;} // Goes in manual enter mode of the RGB vallues by keypad
    if (customKey== 'C'){operate_mode = 3;former_time = actual_time;ClearLCD();PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"noshow");customKey=(char)0;} // Goes to B/W contrast control mode
    if (customKey== 'D'){operate_mode = 4;former_time = actual_time;stringval="";ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;}
    }
     if (operate_mode==1) 
    {
      if (customKey== '#'){operate_mode=0;stringval=(String)former_time;customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
      if (customKey== 'A'){int valuit = stringval.toInt(); val_b = 255 - ValitVal(valuit);PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== 'B'){int valuit = stringval.toInt(); val_g = 255 - ValitVal(valuit);PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== 'C'){int valuit = stringval.toInt(); val_r = 255 - ValitVal(valuit);PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== 'D'){val_b = brightness; val_g = brightness; val_r = brightness; PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== '*'){stringval="";customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
     //Serial.println(stringval.substring(0, stringval.length() - 1));}
      //val_b
    }
    if (operate_mode==2) 
    {
      if (customKey== 'A'){int valuit = stringval.toInt(); val_r = ValitVal(valuit);PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== 'B'){int valuit = stringval.toInt(); val_g = ValitVal(valuit);PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== 'C'){int valuit = stringval.toInt(); val_b = ValitVal(valuit);PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== 'D'){val_b = brightness; val_g = brightness; val_r = brightness; PaintLED(val_r,val_g,val_b,"noshow");customKey=(char)0;stringval="";}
      if (customKey== '#'){operate_mode=0;stringval=(String)former_time;customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
      if (customKey== '*'){stringval="";customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
    }
    if (operate_mode==3) 
    {
      if (customKey== 'A'){if (contrast<9){contrast++;};customKey=(char)0;PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"noshow");} // stringval="";
      if (customKey== 'B'){if (contrast>0){contrast--;};customKey=(char)0;PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"noshow");} //stringval="";
      if (customKey== 'C'){split_mode =3;customKey=(char)0; stringval="";}
      if (customKey== 'D'){split_mode =0;customKey=(char)0; stringval="";}
      if ((customKey== '#')&&(no_color == 0)){operate_mode=0;stringval=(String)former_time;customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
      if (customKey== '*'){stringval="";customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
    }
    if (operate_mode==4) // Storage of values
    {
      if (customKey== 'A')
      {
            int store_start = stringval.toInt() * 10;
            EEPROM.write(store_start + 0, val_r);
            EEPROM.write(store_start + 1, val_g);
            EEPROM.write(store_start + 2, val_b);
            Serial.println(store_start);

          operate_mode=0;stringval=(String)former_time;customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");
      }  
       if (customKey== 'B')
      {
            int store_start = stringval.toInt() * 10;
            val_r = EEPROM.read(store_start + 0);
            val_g = EEPROM.read(store_start + 1);
            val_b = EEPROM.read(store_start + 2);
            Serial.println(store_start);

          operate_mode=0;stringval=(String)former_time;customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");
      }  
      if (customKey== '#'){operate_mode=0;stringval=(String)former_time;customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
      if (customKey== '*'){stringval="";customKey=(char)0;ClearLCD();PaintLED(val_r,val_g,val_b,"noshow");}  
    }
      
    if (customKey!=(char)0) // Time values are composed from Keypad entry
    { 
    if (starttime == 0) (stringval = stringval + (String)customKey);
    }
    
    if ((operate_mode==0)||(operate_mode==3)) // Sets the time from the keypad
    {
      if (stringval!="") 
        {actual_time = stringval.toInt();Serial.println("Actual Time 1 : " + (String)actual_time);}
      else
        {actual_time=0;}
    }

    //Serial.println(customKey);
    }
    
    // IR Case. Optional if you want IR remote control
          if (customKey== 'S') {StartTimer();customKey=(char)0;stringval="";}
          
          if (customKey== 'X') 
          {
           customKey=(char)0;stringval="";
           if (toggle_switch == 0)
           {
             
               if (operate_mode!=3)
               {
               PaintLED(val_r,val_g,val_b,"Show");pixels.show();
               }
               else
               {
               PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"Show");pixels.show();
               }  
               static_l=1;
             
             toggle_switch = 1;
           }
           else
           { 
             if (starttime != 1){pixels.clear();pixels.show();Serial.println("Clear");static_l=0;}     
             toggle_switch = 0;
           }
          }

    
    if ((operate_mode==0)||(operate_mode==3)) // Allows the timer to start if in default or contrast control mode
    {
      if (starttime == 0) // Timer not started. Will show the actual time that is set
      {
       if (toggle_timer==1)
       {
        float actual_time_d = (float)actual_time / 10.0;
        Serial.println(actual_time_d,1);
        lcd.setCursor(0, 0);
        lcd.print("Time set: ");
        lcd.print(actual_time_d,1);
        lcd.print("s    ");
      }
      else
      { 
        lcd.setCursor(0, 0);
        lcd.print("Time set: " +(String)actual_time + "s    ");
      }
      }
      else // Timer started
      {
         if (currentMillis - previousMillis_1 >= interval_1)  // Count 1 second
          { 
               previousMillis_1 = currentMillis;       

               if (toggle_timer==1)
               {
                float actual_time_d = (float)actual_time / 10.0;
                Serial.println(actual_time_d,1);
                lcd.setCursor(0, 0);
                lcd.print("Timing: ");
                lcd.print(actual_time_d,1);
                lcd.print("s    ");
              }
              else
              { 
                lcd.setCursor(0, 0);
                lcd.print("Timing: " +(String)actual_time + "s    ");
              }


               /*    
               lcd.setCursor(0, 0);
               lcd.print("Timing: " +(String)actual_time + "s    "); 
              */
               if (actual_time == 0) // Stops timer once 0 is reached
               {
               actual_time = former_time+1;
               pixels.clear();pixels.show();
               starttime = 0;
               if (split_mode == 1) {split_mode = 0;actual_time = actual_time -1;}
               if (split_mode == 3)
               {
               split_mode = 1;
               delay(500);
               StartTimer();
               }
               }
               actual_time = actual_time -1;  
         }  
     }
   }  

  if (operate_mode==1)
  {
  lcd.setCursor(0, 0);
  lcd.print("Val: " + stringval);  
  }
  if (operate_mode==2)
  {
  lcd.setCursor(0, 0);
  lcd.print("Val: " + stringval);  
  }
  if (operate_mode==3)
  {
    
  int contracts_c = contrast + 1;
  double cc = (double)contracts_c/2.0;
  lcd.setCursor(0, 1);
  if (split_mode==0){lcd.print("Contrast: " + (String)cc  );}else{lcd.print("Split Contrast   ");}
  }
  if (operate_mode==4)
  {
  lcd.setCursor(0, 0);
  lcd.print("Storage#: " + stringval);  
  }
  
}

int ValitVal(int val) // Cleans values under 0 or above 255 in order to mach 8 bit 
{
  int argval = val;
  if (argval<0){argval=0;}
  if (argval>255){argval=255;}
  return argval;
}

void ClearLCD() // Cleans the LCD
{
              lcd.setCursor(0, 0);
              lcd.print("                   -");
              lcd.setCursor(0, 1);
              lcd.print("                   -");
}

void StartTimer() // Starts the timer
{
 if (actual_time == 0) {actual_time=5;} // dontknow if still needed 
 if (operate_mode!=3) // if not in contrast control ist paints the LCD with YMC Values
 {
 PaintLED(val_r,val_g,val_b,"Show");pixels.show();
 }
 else // if  in contrast control ist paints the LCD with RGB Values from the 3x10 array "multival"
 {
 if (split_mode == 0) {PaintLED(multival[contrast][0],multival[contrast][1],multival[contrast][2],"Show");pixels.show();}
 if (split_mode == 3) {PaintLED(multival[0][0],multival[0][1],multival[0][2],"Show");pixels.show();}
 if (split_mode == 1) {PaintLED(multival[9][0],multival[9][1],multival[9][2],"Show");pixels.show();}
 }
 starttime = 1;
 former_time = actual_time;
}


void PaintLED(int R_1, int G_1, int B_1, String Origin)
{

  if (redlight == 1)
  {R_1=255; 
   G_1=0; 
   B_1=0;}
  
  Serial.println("Painting");  
   // cleanup procedure... maybe obsolete but cannot damage anything
    if (R_1<0){R_1=0;}
    if (R_1>255){R_1=255;}
        if (G_1<0){G_1=0;}
    if (G_1>255){G_1=255;}
        if (B_1<0){B_1=0;}
    if (B_1>255){B_1=255;}

    EEPROM.write(0, val_r);
    EEPROM.write(1, val_g);
    EEPROM.write(2, val_b);

    if (operate_mode!=2)
    {
      if (operate_mode!=3)
             {
              lcd.setCursor(0, 1);
              lcd.print("                   -");
              lcd.setCursor(0, 1);
              lcd.print("Y" + (String)(255-val_b) +" M" + (String)(255-val_g)+" C" + (String)(255-val_r));
              }  
    }
    else
    {
              lcd.setCursor(0, 1);
              lcd.print("                   -");
              lcd.setCursor(0, 1);
              lcd.print("R" + (String)(val_r) +" G" + (String)(val_g)+" B" + (String)(val_b));
    }
    
    pixels.clear();
    for(int i=0; i<NUMPIXELS; i++)
    { 
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    //pixels.setPixelColor(i, pixels.Color(R_1, G_1, B_1)); //Cyan (0;x;x) / Magenta(x;0;x) / Yellow(x;x;0)
    uint32_t gammacolor = pixels.Color(R_1, G_1, B_1);
    pixels.setPixelColor(i, gammacolor); 
    delay(1); // Pause before next pass through loop
  }
   if(Origin!="noshow") {pixels.show();} // Here the light goes on
   if (static_l ==1) {pixels.show();} 
}
