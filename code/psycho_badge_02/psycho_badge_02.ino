// psycho badge code rev02

// Include Libraries
#include <Arduino.h>
#include <WiFi.h>
#include "Adafruit_NeoPixel.h"
#include <SPI.h>
#include <driver/adc.h>
#include <esp_wifi_types.h>
#include <esp_wifi.h>
// EINK Display Libraries
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "psycho_img01.h"
#include "psycho_img02.h"
#include "psycho_img03.h"
#include "psycho_img04.h"
#include "psycho_img05.h"
#include "psycho_img06.h"
#include "psycho_img07.h"
// Include CTF Library
// #include <psycho_badge_lib.h>

// Pin Definitions
//
// Pins to Avoid:
// GPIO 32 & 33: Connected to the 32kHz crystal oscillator for deep sleep. 
// GPIO 0, 2, 12, & 15 (Strapping Pins):
//   The ESP32 evaluates the voltage on these pins at startup to determine its boot mode
// GPIO 6, 7, 8, 9, 10, & 11: Internally connected to the SPI Flash memory. 
// GPIO 34, 35, 36 (VP), & 39 (VN): Input-only pins.
//
// Safe Pins to use on the LOLIN32 Lite:
// GPIO 4, 5, 13, 14, 25, 26, 27
// GPIO 16, 17, 18, 19, 21, 23
// (Note: GPIO 22 is used for the onboard LED on this board)
//
// NeoPixel Data Pins
#define NEO01_DATA 27 // Defcon
#define NEO02_DATA 14 // Motel
//
// One color LED Pins
#define LED_D1 33 // FREE
#define LED_D2 32 // WIFI
#define LED_D3 26 // Clouds x2
#define LED_D4 25 // Clouds
#define LED_D5 19 // Mansion x2
#define LED_D6 12 // Room_Door
//
// Built-in LED
#define LED_BI 22
//
// Capacitive Touch Pins
#define TCH01_PIN 4    // Bats
#define TCH02_PIN 2    // Satelite
#define TCH03_PIN 15   // Car
#define TCH04_PIN 13   // Knife
//
// EINK Display mapping for ESP32 LOLIN D32
// BUSY -> 34
// RST -> 16
// DC -> 17
// CS -> SS(5)
// CLK -> SCK(18)
// SDI -> MOSI(23)
// GND -> GND, 3.3V -> 3.3V
#define EPD_BUSY_PIN 34
#define EPD_RST_PIN 16
#define EPD_DC_PIN 17
#define EPD_CS_PIN 5
#define EPD_CLK_PIN 18
#define EPD_SDI_PIN 23
//
// Remapping standard SPI MISO pin due to conflict
#define NEW_MISO_PIN 0  // was 19 which is now LED since EINK does not use MISO

// NeoPixel Properties
//
// Define NeoPixel Strips - (Num pixels, pin to send signals, pixel type, signal rate)
Adafruit_NeoPixel NEO01 = Adafruit_NeoPixel(3, NEO01_DATA, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel NEO02 = Adafruit_NeoPixel(5, NEO02_DATA, NEO_RGB + NEO_KHZ800);

// LED Variables
//
// Main LED mode 0=default 1=coloshift 2+=?
int main_led_mode = 0;
// Door LED mode toggles on/off - helps light eink display
int door_led_mode = 0;
// Free Wifi LED mode toggles flashing/off
int freewifi_led_mode = 1;

//
// NeoPixel Big Color Value 32 bit = (W-8bit << 24) + (R-8bit << 16) + (G-8bit <<8) + (B-8bit)
uint32_t neo_big_color = 0;
uint8_t neo_col_whi = 0;
uint8_t neo_col_red = 0;
uint8_t neo_col_grn = 0;
uint8_t neo_col_blu = 0;
// Neo Color bitvector 1=red 2=green 4=blue (3=R&G 5=R&B 6=G&B 7=R&G&B)
int neo_color_bitv = 5;

// // PWM Properties
// //
// // Signal Frequency in Hz
// const int freq = 1000;
// // Duty Cycle Resolution in bits (1-16)
// const int resolution = 8;
// // PWM channel Assignment (0-15)
// const int LED_D1_pwm = 1;

// Wireless Properties
//
// WIFI status codes
const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
}

// Capacitive Touch Properties
//
// Touch Thresholds
int Touch01_Threshold = 28;
int Touch02_Threshold = 28;
int Touch03_Threshold = 28;
int Touch04_Threshold = 28;
// Touch Initial Values
int Touch01_Value = 30;
int Touch02_Value = 30;
int Touch03_Value = 30;
int Touch04_Value = 30;
//
// Touch Counters
//
// Touch Iteration Counter
int Touch01_IntCount = 0;
int Touch02_IntCount = 0;
int Touch03_IntCount = 0;
int Touch04_IntCount = 0;
// Touch Iteration Flag
int Touch01_IntFlag = 0;
int Touch02_IntFlag = 0;
int Touch03_IntFlag = 0;
int Touch04_IntFlag = 0;
// Touch Loop Counter
int Touch01_LoopCount = 0;
int Touch02_LoopCount = 0;
int Touch03_LoopCount = 0;
int Touch04_LoopCount = 0;
// Touch Loop Threshold (Touch Held for X Loops of Main)
//   Used to determine if there is a 'Long' touch to trigger specific events
//   Each Loop is between 6000 and 7000 ms (6 to 7 seconds)
int Touch01_Loop_Threshold = 3;
int Touch02_Loop_Threshold = 3;
int Touch03_Loop_Threshold = 1;
int Touch04_Loop_Threshold = 1;
// Touch Iter Threshold (Touch Held for X Iterations in the main loop)
//   Used to prevent glancing touches activating the touch events
//   Each iteration is between 3 and 4 ms
int Touch01_Iter_Threshold = 9;
int Touch02_Iter_Threshold = 9;
int Touch03_Iter_Threshold = 9;
int Touch04_Iter_Threshold = 9;

// Display Image List
const unsigned char* const DisplayImage[] = {
  gImage_psycho_hitchcock_02,
  gImage_psycho_poster_ylw_01,
  gImage_dying_reach_05,
  gImage_scream_red_04,
  gImage_psycho_crazy_dress_07,
  gImage_psycho_mother_03,
  gImage_psycho_poster_red_06
};
int ImgElement = 0;
int ImgMax = 6;  // array index max number (0-based)
int DisplayUpdateCooldownDefault = 2;  // loops to wait between display updates
int DisplayUpdateCooldown = DisplayUpdateCooldownDefault;

// Loop Control Properties
//
// Main Loop LED Iteration Delay Time [in ms] - Default 25
int LEDDelayTime = 25;
//
// Timers
unsigned long IterTimer = 0;
unsigned long LoopTimer = 0;
//
// Debug Serial - If set greater than 0 it writes to serial for debugging
// 0 = no debug text
// 1 = basic debug info once per main loop
// 2 = extra debug info
int DebugSerial = 2;

// //////////////////////////////////////////////////
//
// SETUP - RUN ONCE
//
// //////////////////////////////////////////////////
void setup(){
  // Add a delay to allow opening serial monitor
  delay(800);

  // setup the serial output baud rate
  Serial.begin(115200);

  if (DebugSerial >= 1) {
    Serial.println("Starting Setup");
  }

  // Turn Off WiFi/BT
  if (DebugSerial >= 2) {
    Serial.println("Turn Off WiFi / BlueTooth");
  }
  setModemSleep();

  // // Configure LED PWM functionalitites per channel
  // if (DebugSerial >= 2) {
  //   Serial.println("Configure PWM Channels");
  // }
  // ledcSetup(LED_D1_pwm, freq, resolution);

  // // Attach the channel to the GPIO to be controlled
  // if (DebugSerial >= 2) {
  //   Serial.println("Attach PWM Channels to LED Pins");
  // }
  // ledcAttachPin(LED_D1, LED_D1_pwm);

  //Normal LED output
  if (DebugSerial >= 2) {
    Serial.println("Set Output for non-PWM LED Pins");
  }
  pinMode(LED_BI, OUTPUT);
  pinMode(LED_D1, OUTPUT);
  pinMode(LED_D2, OUTPUT);
  pinMode(LED_D3, OUTPUT);
  pinMode(LED_D4, OUTPUT);
  pinMode(LED_D5, OUTPUT);
  pinMode(LED_D6, OUTPUT);

  // Initialize the NeoPixels
  if (DebugSerial >= 2) {
    Serial.println("Initialize NeoPixels");
  }
  NEO01.begin();
  NEO02.begin();
  // Set Neopixel Brightness (0-255 scale)
  NEO01.setBrightness(170);
  NEO02.setBrightness(170);
  // Set an initial random color for neopixel functions
  // setRandomColor();
  // Set an initial static color for the NeoPixels
  setStaticColor(0, 0, 0, neo_color_bitv);

  // Start all LEDs in OFF mode
  if (DebugSerial >= 2) {
    Serial.println("Turn OFF all LEDs");
  }
  ledAllOff();

  // Set a Random Seed
  // int myrandseed = (analogRead(0) + analogRead(34) * 2);
  int myrandseed = (analogRead(0) + analogRead(34) + (touchRead(TCH01_PIN) * 2));
  randomSeed(myrandseed);
  if (DebugSerial >= 2) {
    Serial.print("Set Random Seed "); Serial.println(myrandseed);
  }

  // Initialize EINK Display
  if (DebugSerial >= 2) {
    Serial.println("Initialize EINK Display");
  }
  pinMode(EPD_BUSY_PIN, INPUT); // BUSY 34
  pinMode(EPD_RST_PIN, OUTPUT); // RST 16
  pinMode(EPD_DC_PIN, OUTPUT);  // DC 17
  pinMode(EPD_CS_PIN, OUTPUT);  // CS 5
  pinMode(EPD_CLK_PIN, OUTPUT); // CLK 18
  pinMode(EPD_SDI_PIN, OUTPUT); // SDI/MOSI 23
  //
  // EINK SPI Setup
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0)); 
  //SPI.begin ();
  SPI.begin(EPD_CLK_PIN, NEW_MISO_PIN, EPD_SDI_PIN, EPD_CS_PIN);
  //
  // DISPLAY FIRST IMAGE
  if (DebugSerial >= 2) {
    Serial.println("Display EINK Initial Image");
  }
  fastDisplayImg();

  if (DebugSerial >= 1) {
    Serial.println(F("Setup Done!"));
  }

  // END OF SETUP
}

// //////////////////////////////////////////////////
//
// LOOP - MAIN 
//
// //////////////////////////////////////////////////
void loop(){
  if (DebugSerial >= 1) {
    Serial.println("******************** TOP OF MAIN LOOP ********************");
  }

  // //////////////////////////////////
  //     START OF MAIN BADGE LOOP
  // //////////////////////////////////

  if (DebugSerial >= 1) {
    Serial.println("********** MAIN BADGE LOOP **********");
  }

  // Record the start time of the badge loop
  LoopTimer = millis();

  // Capacitive Touch Dynamic Threshold Adjustment
  // Adjust thresholds UP to account for assembly conditions and battery vs usb
  Touch01_Value = touchRead(TCH01_PIN);
  Touch02_Value = touchRead(TCH02_PIN);
  Touch03_Value = touchRead(TCH03_PIN);
  Touch04_Value = touchRead(TCH04_PIN);
  if ( (Touch01_Value / Touch01_Threshold) > 2 ) { Touch01_Threshold = int(Touch01_Threshold * 1.8); }
  if ( (Touch02_Value / Touch02_Threshold) > 2 ) { Touch02_Threshold = int(Touch02_Threshold * 1.8); }
  if ( (Touch03_Value / Touch03_Threshold) > 2 ) { Touch03_Threshold = int(Touch03_Threshold * 1.8); }
  if ( (Touch04_Value / Touch04_Threshold) > 2 ) { Touch04_Threshold = int(Touch04_Threshold * 1.8); }

  // //////////////////////////////////
  //     START OF ITERATION LOOP
  // //////////////////////////////////
  
  // Iterate 0 to 254
  for(int i=0; i<255; i++){
    // Set position value to iteration value
    int pos = i;

    // Record the start time of the iteration loop
    IterTimer = millis();

    // DEBUG - Print current Iteration value to serial console for troubleshooting
    if (DebugSerial >= 2) {
      Serial.print(" I="); Serial.print(i);
      Serial.print(" Pos="); Serial.print(pos);
    }

    // DEBUG - Print Display Cooldown
    if (DebugSerial >= 2) {
      Serial.print(" D="); Serial.print(DisplayUpdateCooldown);
    }

    //
    // TOUCH
    //
    // Read Touch Values
    Touch01_Value = touchRead(TCH01_PIN);
    Touch02_Value = touchRead(TCH02_PIN);
    Touch03_Value = touchRead(TCH03_PIN);
    Touch04_Value = touchRead(TCH04_PIN);
    //
    // **************************************************************
    //
    // Do Stuff If We Detect a Touch on TCH01_PIN
    if (Touch01_Value < Touch01_Threshold) {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T1_TCH="); Serial.print(Touch01_Value);
        Serial.print("/"); Serial.print(Touch01_Threshold);
        Serial.print("-"); Serial.print(Touch01_IntCount);
        Serial.print("/"); Serial.print(Touch01_LoopCount);
      }
      // STUFF - TCH01_PIN TOUCHED (use IntCount to avoid glancing touches)
      if (Touch01_IntFlag == 0 and Touch01_IntCount > Touch01_Iter_Threshold){
        // Put stuff to happen once per iteration loop here
        Touch01_IntFlag = 1;
        // Change Static Color for Neopixels
        if (neo_color_bitv < 7) { neo_color_bitv++; } else { neo_color_bitv = 1;}
        setStaticColor(0, 0, 0, neo_color_bitv);
      }
      // Put stuff to happen every iteration here
      Touch01_IntCount++;
      //
      if (Touch01_IntCount > Touch01_Iter_Threshold){
        // **************
        // FUNCTION TO CALL GOES HERE
        // EXAMPLE:
        // monarch_neo_color();
        digitalWrite(LED_D5, HIGH); // Turn ON Mansion LED
        // **************
      }
    //
    // Do Stuff If We DONT Detect a Touch on TCH01_PIN
    } else {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T1="); Serial.print(Touch01_Value);
        Serial.print("/"); Serial.print(Touch01_Threshold);
        Serial.print("-"); Serial.print(Touch01_IntCount);
        Serial.print("/"); Serial.print(Touch01_LoopCount);
      }
      // STUFF - TCH01_PIN NOT TOUCHED
      if (Touch01_IntCount > 1) { Touch01_IntCount--; } else { Touch01_IntCount = 0; }
    }
    //
    // **************************************************************
    //
    // Do Stuff If We Detect a Touch on TCH02_PIN
    if (Touch02_Value < Touch02_Threshold) {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T2_TCH="); Serial.print(Touch02_Value);
        Serial.print("/"); Serial.print(Touch02_Threshold);
        Serial.print("-"); Serial.print(Touch02_IntCount);
        Serial.print("/"); Serial.print(Touch02_LoopCount);
      }
      // STUFF - TCH02_PIN TOUCHED (use IntCount to avoid glancing touches)
      if (Touch02_IntFlag == 0 and Touch02_IntCount > Touch02_Iter_Threshold){
        // Put stuff to happen once per iteration loop here
        Touch02_IntFlag = 1;
        // Change Main LED Mode
        main_led_mode = main_led_mode + 1;
        if (main_led_mode > 1) { main_led_mode = 0; }
      }
      // Put stuff to happen every iteration here
      Touch02_IntCount++;
      //
      if (Touch02_IntCount > Touch02_Iter_Threshold){
        // **************
        // FUNCTION TO CALL GOES HERE
        // EXAMPLE:
        // monarch_neo_color();
        digitalWrite(LED_D5, HIGH); // Turn ON Mansion LED
        // **************
      }
    //
    // Do Stuff If We DONT Detect a Touch on TCH02_PIN
    } else {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T2="); Serial.print(Touch02_Value);
        Serial.print("/"); Serial.print(Touch02_Threshold);
        Serial.print("-"); Serial.print(Touch02_IntCount);
        Serial.print("/"); Serial.print(Touch02_LoopCount);
      }
      // STUFF - TCH02_PIN NOT TOUCHED
      if (Touch02_IntCount > 1) { Touch02_IntCount--; } else { Touch02_IntCount = 0; }
    }
    //
    // **************************************************************
    //
    // Do Stuff If We Detect a Touch on TCH03_PIN
    if (Touch03_Value < Touch03_Threshold) {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T3_TCH="); Serial.print(Touch03_Value);
        Serial.print("/"); Serial.print(Touch03_Threshold);
        Serial.print("-"); Serial.print(Touch03_IntCount);
        Serial.print("/"); Serial.print(Touch03_LoopCount);
      }
      // STUFF - TCH03_PIN TOUCHED (use IntCount to avoid glancing touches)
      if (Touch03_IntFlag == 0 and Touch03_IntCount > Touch03_Iter_Threshold){
        // Put stuff to happen once per iteration loop here
        Touch03_IntFlag = 1;
        // Change Door LED Mode
        freewifi_led_mode = freewifi_led_mode + 1;
        if (freewifi_led_mode > 1) { freewifi_led_mode = 0; }
        //
      }
      // Put stuff to happen every iteration here
      Touch03_IntCount++;
      //
      if (Touch03_IntCount > Touch03_Iter_Threshold){
        // **************
        // FUNCTION TO CALL GOES HERE
        // EXAMPLE:
        // monarch_neo_color();
        digitalWrite(LED_D5, HIGH); // Turn ON Mansion LED
        // **************
      }
    //
    // Do Stuff If We DONT Detect a Touch on TCH03_PIN
    } else {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T3="); Serial.print(Touch03_Value);
        Serial.print("/"); Serial.print(Touch03_Threshold);
        Serial.print("-"); Serial.print(Touch03_IntCount);
        Serial.print("/"); Serial.print(Touch03_LoopCount);
      }
      // STUFF - TCH03_PIN NOT TOUCHED
      if (Touch03_IntCount > 1) { Touch03_IntCount--; } else { Touch03_IntCount = 0; }
    }
    //
    // **************************************************************
    //
    // Do Stuff If We Detect a Touch on TCH04_PIN
    if (Touch04_Value < Touch04_Threshold) {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T4_TCH="); Serial.print(Touch04_Value);
        Serial.print("/"); Serial.print(Touch04_Threshold);
        Serial.print("-"); Serial.print(Touch04_IntCount);
        Serial.print("/"); Serial.print(Touch04_LoopCount);
      }
      // STUFF - TCH04_PIN TOUCHED (use IntCount to avoid glancing touches)
      if (Touch04_IntFlag == 0 and Touch04_IntCount > Touch04_Iter_Threshold){
        // Put stuff to happen once per iteration loop here
        Touch04_IntFlag = 1;
        // Change Door LED Mode
        door_led_mode = door_led_mode + 1;
        if (door_led_mode > 1) { door_led_mode = 0; }
        //
      }
      // Put stuff to happen every iteration here
      Touch04_IntCount++;
      //
      if (Touch04_IntCount > Touch04_Iter_Threshold){
        // **************
        // FUNCTION TO CALL GOES HERE
        // EXAMPLE:
        // monarch_neo_color();
        digitalWrite(LED_D5, HIGH); // Turn ON Mansion LED
        // **************
      }
    //
    // Do Stuff If We DONT Detect a Touch on TCH04_PIN
    } else {
      // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" T4="); Serial.print(Touch04_Value);
        Serial.print("/"); Serial.print(Touch04_Threshold);
        Serial.print("-"); Serial.print(Touch04_IntCount);
        Serial.print("/"); Serial.print(Touch04_LoopCount);
      }
      // STUFF - TCH04_PIN NOT TOUCHED
      if (Touch04_IntCount > 1) { Touch04_IntCount--; } else { Touch04_IntCount = 0; }
    }
    //
    // **************************************************************
    //

    if (main_led_mode == 0) {
      //
      // DEFAULT MODE
      //
      // First of three position groups i 0-84
      if (pos < 85) {
        //
        // LED FUNCTIONS
        BI_blink_three(pos);
        freeWifi(pos);
        doorlogo();
        defcon_neo_color();
        motel_neo_color(pos, 1);
      // Second of three position groups i 85-169 (pos-85 = 0-84)
      } else if (pos < 170) {
        pos = pos - 85;
        //
        // LED FUNCTIONS
        BI_blink_three(pos);
        freeWifi(pos);
        doorlogo();
        defcon_neo_color();
        motel_neo_color(pos, 2);
      // Third of three position groups i 170-254 (pos-170 = 0-84)
      } else {
        pos = pos -170;
        //
        // LED FUNCTIONS
        BI_blink_three(pos);
        freeWifi(pos);
        doorlogo();
        defcon_neo_color();
        motel_neo_color(pos, 3);
        thunderLightning(pos);
        // // Split third group 3/4 (pos 0-42) for even number of transitions
        // if (pos <43) {
        //   //
        // // Split third group 4/4 (pos 43-84) for even number of transitions
        // } else {
        //   //
        // }
      }
    } else if (main_led_mode == 1) {
      //
      // COLORSHIFT MODE
      //
      // First of three position groups i 0-84
      if (pos < 85) {
        //
        // LED FUNCTIONS
        BI_blink_three(pos);
        freeWifi(pos);
        doorlogo();
        defcon_neo_colorshift(pos, 1);
        motel_neo_colorshift(pos, 1);
      // Second of three position groups i 85-169 (pos-85 = 0-84)
      } else if (pos < 170) {
        pos = pos - 85;
        //
        // LED FUNCTIONS
        BI_blink_three(pos);
        freeWifi(pos);
        doorlogo();
        defcon_neo_colorshift(pos, 2);
        motel_neo_colorshift(pos, 2);
      // Third of three position groups i 170-254 (pos-170 = 0-84)
      } else {
        pos = pos -170;
        //
        // LED FUNCTIONS
        BI_blink_three(pos);
        freeWifi(pos);
        doorlogo();
        defcon_neo_colorshift(pos, 3);
        motel_neo_colorshift(pos, 3);
        // thunderLightning(pos);
        // // Split third group 3/4 (pos 0-42) for even number of transitions
        // if (pos <43) {
        //   //
        // // Split third group 4/4 (pos 43-84) for even number of transitions
        // } else {
        //   //
        // }
      }
    // } else if (main_led_mode == 1) {
    //   //
    //   // BATTLE MODE
    //   //
    //   // First of three position groups i 0-84
    //   if (pos < 85) {
    //     //
    //     // LED FUNCTIONS
    //     BI_blink_two(pos);
    //   // Second of three position groups i 85-169 (pos-85 = 0-84)
    //   } else if (pos < 170) {
    //     pos = pos - 85;
    //     //
    //     // LED FUNCTIONS
    //     BI_blink_two(pos);
    //   // Third of three position groups i 170-254 (pos-170 = 0-84)
    //   } else {
    //     pos = pos -170;
    //     //
    //     // LED FUNCTIONS
    //     BI_blink_two(pos);
    //     // Split third group 3/4 (pos 0-42) for even number of transitions
    //     if (pos <43) {
    //       //
    //     // Split third group 4/4 (pos 43-84) for even number of transitions
    //     } else {
    //       //
    //     }
    //   }
    }
    
    // DEBUG - Print NEO color vars
    if (DebugSerial >= 2) {
      Serial.print(" C="); Serial.print(neo_col_red);
      Serial.print("/"); Serial.print(neo_col_grn);
      Serial.print("/"); Serial.print(neo_col_blu);
      Serial.print("/"); Serial.print(neo_color_bitv);
    }

    // DEBUG - Print LED mode
    if (DebugSerial >= 2) {
      Serial.print(" Mode="); Serial.print(main_led_mode);
    }

    // DEBUG - Print Iteration Time
    if (DebugSerial >= 2) {
      Serial.print(" ms="); Serial.print(millis() - IterTimer);
    }

    // DEBUG - Print Carriage Return for iteration level debug output
    if (DebugSerial >= 2) {
      Serial.println();
    }

    // Display Neopixel values
    neo_show();

    // Pause the loop to display everything
    delay(LEDDelayTime);

    // Turn off digital LEDs at the end of the iteration loop
    ledDigitalOff();

    // END OF FOR ITERATION LOOP
  }
  // //////////////////////////////////
  //     END OF ITERATION LOOP
  // //////////////////////////////////

  // Touch Loop Counters - USE TBD
  if (Touch01_IntCount >= 1) { Touch01_LoopCount++; Touch01_IntCount = 0; } else { Touch01_LoopCount = 0; }
  if (Touch02_IntCount >= 1) { Touch02_LoopCount++; Touch02_IntCount = 0; } else { Touch02_LoopCount = 0; }
  if (Touch03_IntCount >= 1) { Touch03_LoopCount++; Touch03_IntCount = 0; } else { Touch03_LoopCount = 0; }
  if (Touch04_IntCount >= 1) { Touch04_LoopCount++; Touch04_IntCount = 0; } else { Touch04_LoopCount = 0; }

  // Reset Touch Iteration Flags
  Touch01_IntFlag = 0;
  Touch02_IntFlag = 0;
  Touch03_IntFlag = 0;
  Touch04_IntFlag = 0;

  // Decrement Display update cooldown
  if (DisplayUpdateCooldown > 0) { DisplayUpdateCooldown--; }

  // Turn off all LEDs at end of loop (Optional for troubleshooting)
  // ledAllOff();

  // //////////////////////////////////////////////////
  //
  // Change Display to PREV Image When
  // Touch03_LoopCount exceeds Touch03_Loop_Threshold 
  // and DisplayUpdateCooldown is 0
  // Touch04 is the CAR
  //
  // //////////////////////////////////////////////////
  if (Touch03_LoopCount > Touch03_Loop_Threshold and DisplayUpdateCooldown == 0) {
    //
    Serial.println("LONG TOUCH DETECTED on TCH03 - Update Display PREV Image");
    //
    Touch03_LoopCount = 0;
    //
    // Update Display PREV Image
    prevImgElement();
    fastDisplayImg();
  }

  // //////////////////////////////////////////////////
  //
  // Change Display to NEXT Image When
  // Touch04_LoopCount exceeds Touch04_Loop_Threshold 
  // and DisplayUpdateCooldown is 0
  // Touch04 is the KNIFE
  //
  // //////////////////////////////////////////////////
  if (Touch04_LoopCount > Touch04_Loop_Threshold and DisplayUpdateCooldown == 0) {
    //
    Serial.println("LONG TOUCH DETECTED on TCH04 - Update Display NEXT Image");
    //
    Touch04_LoopCount = 0;
    //
    // Update Display NEXT Image
    nextImgElement();
    fastDisplayImg();
  }

  // //////////////////////////////////////////////////
  //
  // Launch BATT_CHRG_NOLED Alternate Mainline Code When
  // Touch01_LoopCount exceeds Touch01_Loop_Threshold
  // Touch01 is the Monarch Logo
  //
  // //////////////////////////////////////////////////
  // if (Touch01_LoopCount > Touch01_Loop_Threshold) {
  //   //
  //   Serial.println("LONG TOUCH DETECTED on TCH02 - JUMP TO ALTERNATE CODE");
  //   //
  //   ledAllOff();
  //   //
  //   Touch01_LoopCount = 0;
  //   //
  //   // Alternate code loop
  //   batt_chrg_noled();
  //   //
  //   // END ALTERNATE MAIN LOOP
  //   Serial.println("****************************************");
  //   Serial.println("***** EXITING BATT_CHRG_NOLED MODE *****");
  //   Serial.println("****************************************");
  //   //
  //   ledAllOff();
  //   //
  //   Touch01_LoopCount = 0;
  //   //
  //   main_led_mode = 0;
  //   // Pause before exiting
  //   delay(100);
  // }

  // DEBUG - Print Loop Run Time
  if (DebugSerial >= 1) {
    Serial.println("  ********** ");
    Serial.print("  LOOP RUN TIME="); Serial.print(millis() - LoopTimer); Serial.println(" ms");
    Serial.println("  ********** ");
    Serial.println("********** END OF MAIN BADGE LOOP **********");
  }

// //////////////////////////////////
//        END OF MAIN BADGE LOOP
// //////////////////////////////////

}
// //////////////////////////////////
//        END OF LOOP - MAIN
// //////////////////////////////////


// //////////////////////////////////////////////////
//
// FUNCTIONS
//
// //////////////////////////////////////////////////
//
// WiFi & BT Functions
// //////////////////////////////////////////////////
void disableWiFi(){
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
    Serial.println("WiFi disabled!");
}
//
void disableBluetooth(){
    btStop();
    Serial.println("Bluetooth stopped!");
}
//
void setModemSleep() {
    disableWiFi();
    disableBluetooth();
    setCpuFrequencyMhz(80);
}
//
void enableWiFi(){
    delay(200);
    // Switch Wifi ON in mode AP/STA/AP_STA
    WiFi.mode(WIFI_AP); // Defaulting to AP mode
    delay(200);
    Serial.println("WiFi Started!");
}
//
void wakeModemSleep() {
    setCpuFrequencyMhz(240);
    enableWiFi();
}
// //////////////////////////////////////////////////
//
// DISPLAY Functions
// //////////////////////////////////////////////////
void nextImgElement() {
    ImgElement++;
    if (ImgElement > ImgMax) { ImgElement = 0; }
}
//
void prevImgElement() {
    ImgElement--;
    if (ImgElement < 0) { ImgElement = ImgMax; }
}
//
void fastDisplayImg() {
    digitalWrite(LED_D6, HIGH); // Turn ON Door LED during update
    EPD_init_Fast();                        // Fast screen initialization.
    PIC_display(DisplayImage[ImgElement]);  // Display image
    EPD_sleep();                            // Enter sleep mode
    DisplayUpdateCooldown = DisplayUpdateCooldownDefault;
    digitalWrite(LED_D6, LOW); // Turn OFF Door LED after update
}
//
void fullDisplayImg() {
    digitalWrite(LED_D6, HIGH); // Turn ON Door LED during update
    EPD_init();                             // Full screen initialization.
    PIC_display(DisplayImage[ImgElement]);  // Display image
    EPD_sleep();                            // Enter sleep mode
    DisplayUpdateCooldown = DisplayUpdateCooldownDefault;
    digitalWrite(LED_D6, LOW); // Turn OFF Door LED after update
}
//
// //////////////////////////////////////////////////
//
// LED Functions
// //////////////////////////////////////////////////
void ledAllOff() {
    digitalWrite(LED_BI, HIGH); // HIGH = OFF???
    digitalWrite(LED_D1, LOW); // LOW = OFF
    digitalWrite(LED_D2, LOW); // LOW = OFF
    digitalWrite(LED_D3, LOW); // LOW = OFF
    digitalWrite(LED_D4, LOW); // LOW = OFF
    digitalWrite(LED_D5, LOW); // LOW = OFF
    digitalWrite(LED_D6, LOW); // LOW = OFF
    // ledcWrite(LED_D1_pwm, 0);
    NEO01.setPixelColor(0, 0, 0, 0);
    NEO01.setPixelColor(1, 0, 0, 0);
    NEO01.setPixelColor(2, 0, 0, 0);
    NEO01.show();
    NEO02.setPixelColor(0, 0, 0, 0);
    NEO02.setPixelColor(1, 0, 0, 0);
    NEO02.setPixelColor(2, 0, 0, 0);
    NEO02.setPixelColor(3, 0, 0, 0);
    NEO02.setPixelColor(4, 0, 0, 0);
    NEO02.show();
}
//
void ledDigitalOn() {
    digitalWrite(LED_D1, HIGH); // HIGH = ON
    digitalWrite(LED_D2, HIGH); // HIGH = ON
    digitalWrite(LED_D3, HIGH); // HIGH = ON
    digitalWrite(LED_D4, HIGH); // HIGH = ON
    digitalWrite(LED_D5, HIGH); // HIGH = ON
    digitalWrite(LED_D6, HIGH); // HIGH = ON
}
//
void ledDigitalOff() {
    digitalWrite(LED_D1, LOW); // LOW = OFF
    digitalWrite(LED_D2, LOW); // LOW = OFF
    digitalWrite(LED_D3, LOW); // LOW = OFF
    digitalWrite(LED_D4, LOW); // LOW = OFF
    digitalWrite(LED_D5, LOW); // LOW = OFF
    digitalWrite(LED_D6, LOW); // LOW = OFF
}
//
void thunderLightning(uint8_t pos) {
    // Implement the thunder and lightning effect for the cloud LEDs
    //
    // POS = 0-84 | 85 div 4 = 21.25 | 0-21, 22-42, 43-63, 64-84
    if ((pos >= 0 and pos <= 21) or (pos >= 43 and pos <= 63)) {
        digitalWrite(LED_D3, LOW); // OFF
        digitalWrite(LED_D4, HIGH); // ON
        if (random(0-5) == 2) {
            digitalWrite(LED_D3, HIGH); // ON
            digitalWrite(LED_D4, LOW); // OFF
        }
    } else if (pos >= 22 and pos <= 42) {
        digitalWrite(LED_D3, HIGH); // ON
        digitalWrite(LED_D4, LOW); // OFF
        if (random(0-5) == 3) {
            digitalWrite(LED_D3, LOW); // OFF
            digitalWrite(LED_D4, HIGH); // ON
        }
    } else if (pos >= 64 and pos <= 84) {
        if (pos % 2 == 1) {
            // If pos is an odd number, flash the corresponding cloud LED
            digitalWrite(LED_D3, LOW); // OFF
            digitalWrite(LED_D4, HIGH); // ON
        } else {
            // If pos is an even number, flash the other cloud LED
            digitalWrite(LED_D3, HIGH); // ON
            digitalWrite(LED_D4, LOW); // OFF
        }
    }
}
//
void freeWifi(uint8_t pos) {
  // freewifi_led_mode 1=toggle-flashing
  //
  if (freewifi_led_mode == 1) {
    // Toggle free and wifi LEDs
    //
    // POS = 0-84 | 85 div 4 = 21.25 | 0-21, 22-42, 43-63, 64-84
    if ((pos >= 0 and pos <= 21) or (pos >= 43 and pos <= 63)) {
        digitalWrite(LED_D1, HIGH); // ON
        digitalWrite(LED_D2, LOW); // OFF
    } else {
        digitalWrite(LED_D2, HIGH); // ON
        digitalWrite(LED_D1, LOW); // OFF
    }
  } else {
    digitalWrite(LED_D1, LOW); // OFF
    digitalWrite(LED_D2, LOW); // OFF
  }
}
//
void doorlogo() {
  // door_led_mode 1=on
  //
  if (door_led_mode == 1) {
    digitalWrite(LED_D6, HIGH); // ON
  } else {
    digitalWrite(LED_D6, LOW); // OFF
  }
}
//
// void ledPwmAllOn() {
//     ledcWrite(LED_D1_pwm, 255);
// }
//
// void ledPwmAllOff() {
//     ledcWrite(LED_D1_pwm, 0);
// }
//
void setRandomColor() {
    // Basic Random numbers for on/off
    int colorrand41 = random(0-4); int colorrand42 = random(0-4); int colorrand43 = random(0-4);
    int colorrand4t = colorrand41 + colorrand42 + colorrand43;
    // Reset colors
    neo_col_whi = 0;
    neo_col_red = 0;
    neo_col_grn = 0;
    neo_col_blu = 0;
    // Set Red
    if (colorrand4t > 3) { neo_col_red = random(50, 255); }
    // Set Blue
    if (neo_col_red == 0 and colorrand4t > 1) { neo_col_blu = random(50, 255); } else if (colorrand4t > 3) { neo_col_blu = random(50, 255); }
    // Set Green
    if (neo_col_blu == 0 and colorrand4t > 3) { neo_col_grn = random(50, 255); } else if (colorrand4t > 5) { neo_col_grn = random(50, 255); }
    // Make sure at least one color is not zero
    if ((neo_col_red + neo_col_grn + neo_col_blu) == 0) { neo_col_red = 255; }
    // Set Color
    neo_big_color = (neo_col_whi << 24) + (neo_col_red << 16) + (neo_col_grn << 8) + neo_col_blu;
}
//
void setStaticColor(uint8_t sred, uint8_t sgrn, uint8_t sblu, uint8_t bitv) {
    // Define colors
    neo_col_whi = 0;
    neo_col_red = 0;
    neo_col_grn = 0;
    neo_col_blu = 0;
    // Neo Color bitvector 1=red 2=green 4=blue (3=R&G 5=R&B 6=G&B 7=R&G&B)
    if (bitv > 0) {
      // Red
      if (bitv == 1 || bitv == 3 || bitv == 5 || bitv == 7) { neo_col_red = 255; }
      // Green
      if (bitv == 2 || bitv == 3 || bitv == 6 || bitv == 7) { neo_col_grn = 255; }
      // Blue
      if (bitv == 4 || bitv == 5 || bitv == 6 || bitv == 7) { neo_col_blu = 255; }
      // Catchall
      if (bitv > 7) {
        neo_col_red = 255;
        neo_col_grn = 255;
        neo_col_blu = 255;
      }
    } else {
      neo_col_red = sred;
      neo_col_grn = sgrn;
      neo_col_blu = sblu;
    }
    // Make sure at least one color is not zero
    if ((neo_col_red + neo_col_grn + neo_col_blu) == 0) { neo_col_red = 255; }
    // Set Color
    neo_big_color = (neo_col_whi << 24) + (neo_col_red << 16) + (neo_col_grn << 8) + neo_col_blu;
}
//
void neo_show() {
    // Display neopixel colors set by other functions
    NEO01.show();
    NEO02.show();
}
//
void all_neo_colorshift(uint8_t pos, uint8_t pass) {
    //
    // If Touch Areas were pressed dont do anything here
    if (Touch01_IntCount > 0 || Touch02_IntCount > 0 || Touch03_IntCount > 0 || Touch04_IntCount > 0) {
      pass = 0;
    }
    // overall pass color order for 1-3 passes
    // 10 - 11 - 12 - B R
    // 11 - 12 - 24 - R G
    // 12 - 24 - 23 - G B
    // 24 - 23 - 22 - B R
    // 23 - 22 - 21 - R G
    // 22 - 21 - 20 - G B
    // 21 - 20 - 10 - B R
    // 20 - 10 - 11 - R G
    //
    // Pass 1 pos 0-84
    if (pass == 1){
      // pass 1 color order
      // 10 B R
      // 24 B R
      // 21 B R
      // 11 R G
      // 23 R G
      // 20 R G
      // 12 G B
      // 22 G B
      // Blue 255-0 Red 0-255
      NEO01.setPixelColor(0, int(pos*3), 0, int(255 - pos*3));
      NEO02.setPixelColor(4, int(pos*3), 0, int(255 - pos*3));
      NEO02.setPixelColor(1, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO01.setPixelColor(1, int(255 - (pos*3)), int(pos*3), 0);
      NEO02.setPixelColor(3, int(255 - (pos*3)), int(pos*3), 0);
      NEO02.setPixelColor(0, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO01.setPixelColor(2, 0, int(255 - (pos*3)), int(pos*3));
      NEO02.setPixelColor(2, 0, int(255 - (pos*3)), int(pos*3));
    }
    // Pass 2 pos 0-84
    if (pass == 2){
      // pass 2 color order
      // 11 B R
      // 23 B R
      // 20 B R
      // 12 R G
      // 22 R G
      // 10 R G
      // 24 G B
      // 21 G B
      // Blue 255-0 Red 0-255
      NEO01.setPixelColor(1, int(pos*3), 0, int(255 - pos*3));
      NEO02.setPixelColor(3, int(pos*3), 0, int(255 - pos*3));
      NEO02.setPixelColor(0, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO01.setPixelColor(2, int(255 - (pos*3)), int(pos*3), 0);
      NEO02.setPixelColor(2, int(255 - (pos*3)), int(pos*3), 0);
      NEO01.setPixelColor(0, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO02.setPixelColor(4, 0, int(255 - (pos*3)), int(pos*3));
      NEO02.setPixelColor(1, 0, int(255 - (pos*3)), int(pos*3));
    }
    // Pass 3 pos 0-84
    if (pass == 3){
      // pass 3 color order
      // 12 B R
      // 22 B R
      // 10 B R
      // 24 R G
      // 21 R G
      // 11 R G
      // 23 G B
      // 20 G B
      // Blue 255-0 Red 0-255
      NEO01.setPixelColor(2, int(pos*3), 0, int(255 - pos*3));
      NEO02.setPixelColor(2, int(pos*3), 0, int(255 - pos*3));
      NEO01.setPixelColor(0, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO02.setPixelColor(4, int(255 - (pos*3)), int(pos*3), 0);
      NEO02.setPixelColor(1, int(255 - (pos*3)), int(pos*3), 0);
      NEO01.setPixelColor(1, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO02.setPixelColor(3, 0, int(255 - (pos*3)), int(pos*3));
      NEO02.setPixelColor(0, 0, int(255 - (pos*3)), int(pos*3));
    }
}
//
void defcon_neo_colorshift(uint8_t pos, uint8_t pass) {
    //
    // If Touch Areas were pressed dont do anything here
    if (Touch01_IntCount > 0 || Touch02_IntCount > 0 || Touch03_IntCount > 0 || Touch04_IntCount > 0) {
      pass = 0;
    }
    // overall pass color order for 1-3 passes
    // 10 - 11 - 12 - B R
    // 11 - 12 - 10 - R G
    // 12 - 10 - 11 - G B
    //
    // Pass 1 pos 0-84
    if (pass == 1){
      // pass 1 color order
      // 10 B R
      // 11 R G
      // 12 G B
      // Blue 255-0 Red 0-255
      NEO01.setPixelColor(0, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO01.setPixelColor(1, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO01.setPixelColor(2, 0, int(255 - (pos*3)), int(pos*3));
    }
    // Pass 2 pos 0-84
    if (pass == 2){
      // pass 2 color order
      // 11 B R
      // 12 R G
      // 10 G B
      // Blue 255-0 Red 0-255
      NEO01.setPixelColor(1, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO01.setPixelColor(2, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO01.setPixelColor(0, 0, int(255 - (pos*3)), int(pos*3));
    }
    // Pass 3 pos 0-84
    if (pass == 3){
      // pass 3 color order
      // 12 B R
      // 10 R G
      // 11 G B
      // Blue 255-0 Red 0-255
      NEO01.setPixelColor(2, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO01.setPixelColor(0, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO01.setPixelColor(1, 0, int(255 - (pos*3)), int(pos*3));
    }
}
//
void defcon_neo_color() {
    //
    // If Touch Areas were pressed dont do anything here
    if (Touch01_IntCount > 0 || Touch02_IntCount > 0 || Touch03_IntCount > 0 || Touch04_IntCount > 0) {
        // pass = 0;
    } else {
        //
        NEO01.setPixelColor(0, neo_big_color);
        NEO01.setPixelColor(1, neo_big_color);
        NEO01.setPixelColor(2, neo_big_color);
        //
    }
}
//
void motel_neo_colorshift(uint8_t pos, uint8_t pass) {
    //
    // If Touch Areas were pressed dont do anything here
    if (Touch01_IntCount > 0 || Touch02_IntCount > 0 || Touch03_IntCount > 0 || Touch04_IntCount > 0) {
      pass = 0;
    }
    // overall pass color order for 1-3 passes
    // 24 - 22 - 20 - B R
    // 22 - 20 - 24 - R G
    // 20 - 24 - 22 - G B
    //
    // Pass 1 pos 0-84
    if (pass == 1){
      // pass 1 color order
      // 24 B R
      // 22 R G
      // 20 G B
      // Blue 255-0 Red 0-255
      NEO02.setPixelColor(4, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO02.setPixelColor(2, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO02.setPixelColor(0, 0, int(255 - (pos*3)), int(pos*3));
      // OFF
      NEO02.setPixelColor(1, 0, 0, 0);
      NEO02.setPixelColor(3, 0, 0, 0);
    }
    // Pass 2 pos 0-84
    if (pass == 2){
      // pass 2 color order
      // 22 B R
      // 20 R G
      // 24 G B
      // Blue 255-0 Red 0-255
      NEO02.setPixelColor(2, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO02.setPixelColor(0, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO02.setPixelColor(4, 0, int(255 - (pos*3)), int(pos*3));
      // OFF
      NEO02.setPixelColor(1, 0, 0, 0);
      NEO02.setPixelColor(3, 0, 0, 0);
    }
    // Pass 3 pos 0-84
    if (pass == 3){
      // pass 3 color order
      // 20 B R
      // 24 R G
      // 22 G B
      // Blue 255-0 Red 0-255
      NEO02.setPixelColor(0, int(pos*3), 0, int(255 - pos*3));
      // Red 255-0 Green 0-255
      NEO02.setPixelColor(4, int(255 - (pos*3)), int(pos*3), 0);
      // Green 255-0 Blue 0-255
      NEO02.setPixelColor(2, 0, int(255 - (pos*3)), int(pos*3));
      // OFF
      NEO02.setPixelColor(1, 0, 0, 0);
      NEO02.setPixelColor(3, 0, 0, 0);
    }
}
//
void motel_neo_color(uint8_t pos, uint8_t pass) {
    //
    // If Touch Areas were pressed dont do anything here
    if (Touch01_IntCount > 0 || Touch02_IntCount > 0 || Touch03_IntCount > 0 || Touch04_IntCount > 0) {
      pass = 0;
    }
    //
    // POS = 0-84 | 85 div 4 = 21.25 | 0-21, 22-42, 43-63, 64-84
    //
    // Pass 1
    // _-_-_-_-_
    // M-_-_-_-_
    // _-O-_-_-_
    // _-_-T-_-_
    // Pass 2
    // _-_-_-E-_
    // _-_-_-_-L
    // _-_-_-_-_
    // M-_-_-_-_
    // Pass 3
    // _-O-_-_-_
    // _-_-T-_-_
    // _-_-_-E-_
    // _-_-_-_-L
    //
    // Pass 1 pos 0-84
    if (pass == 1){
        if (pos >= 0 and pos <= 21) {
            // _-_-_-_-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 21 and pos <= 42) {
            // M-_-_-_-_
            NEO02.setPixelColor(4, neo_big_color);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 42 and pos <= 63) {
            // _-O-_-_-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, neo_big_color);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 63 and pos <= 84) {
            // _-_-T-_-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, neo_big_color);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        }
    }
    // Pass 2 pos 0-84
    if (pass == 2){
        if (pos >= 0 and pos <= 21) {
            // _-_-_-E-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, neo_big_color);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 21 and pos <= 42) {
            // _-_-_-_-L
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, neo_big_color);
        } else if (pos > 42 and pos <= 63) {
            // _-_-_-_-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 63 and pos <= 84) {
            // M-_-_-_-_
            NEO02.setPixelColor(4, neo_big_color);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        }
    }
    // Pass 3 pos 0-84
    if (pass == 3){
        if (pos >= 0 and pos <= 21) {
            // _-O-_-_-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, neo_big_color);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 21 and pos <= 42) {
            // _-_-T-_-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, neo_big_color);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 42 and pos <= 63) {
            // _-_-_-E-_
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, neo_big_color);
            NEO02.setPixelColor(0, 0, 0, 0);
        } else if (pos > 63 and pos <= 84) {
            // _-_-_-_-L
            NEO02.setPixelColor(4, 0, 0, 0);
            NEO02.setPixelColor(3, 0, 0, 0);
            NEO02.setPixelColor(2, 0, 0, 0);
            NEO02.setPixelColor(1, 0, 0, 0);
            NEO02.setPixelColor(0, neo_big_color);
        }
    }
}
//
void BI_on() {
    digitalWrite(LED_BI, LOW); // LOW = ON?
}
//
void BI_off() {
    digitalWrite(LED_BI, HIGH); // HIGH = OFF?
}
//
void BI_blink_one(uint8_t pos) {
    if (pos <= 21) {
      digitalWrite(LED_BI, LOW); // LOW = ON?
    } else {
      digitalWrite(LED_BI, HIGH); // HIGH = OFF?
    }
}
//
void BI_blink_two(uint8_t pos) {
    if (pos <= 11) {
      digitalWrite(LED_BI, LOW); // LOW = ON?
    } else if (pos > 11 and pos <= 22) {
      digitalWrite(LED_BI, HIGH); // HIGH = OFF?
    } else if (pos > 22 and pos <= 33) {
      digitalWrite(LED_BI, LOW); // LOW = ON?
    } else {
      digitalWrite(LED_BI, HIGH); // HIGH = OFF?
    }
}
//
void BI_blink_three(uint8_t pos) {
    if (pos <= 5) {
      digitalWrite(LED_BI, LOW); // LOW = ON?
    } else if (pos > 5 and pos <= 10) {
      digitalWrite(LED_BI, HIGH); // HIGH = OFF?
    } else if (pos > 10 and pos <= 15) {
      digitalWrite(LED_BI, LOW); // LOW = ON?
    } else if (pos > 15 and pos <= 20) {
      digitalWrite(LED_BI, HIGH); // HIGH = OFF?
    } else if (pos > 20 and pos <= 25) {
      digitalWrite(LED_BI, LOW); // LOW = ON?
    } else {
      digitalWrite(LED_BI, HIGH); // HIGH = OFF?
    }
}
//
// void batt_chrg_noled() {
//     // Set an exit var
//     bool batt_chrg_noled_active = true;
//     //
//     while (batt_chrg_noled_active) {
//       BI_off();
//       // Print Serial Message About Mode
//       Serial.println("****************************************");
//       Serial.println("****************************************");
//       Serial.println("********* BATT_CHRG_NOLED MODE *********");
//       Serial.println("****************************************");
//       Serial.println("*** ACTIVATED BY LONG TOUCH ON TCH02 ***");
//       Serial.println("***      THE MONARCH LOGO BUTTON     ***");
//       Serial.println("****************************************");
//       Serial.println("** LONG PRESS AGAIN TO EXIT THIS MODE **");
//       Serial.println("****************************************");
//       Serial.println("****************************************");
//       // Pause
//       delay(3500);
//       // Turn on-board LED on briefly to show badge is still on
//       BI_on();
//       // Pause
//       delay(500);
//       //
//       // Touch for exit mode settings
//       //
//       Touch01_Value = touchRead(TCH01_PIN);
//       // Do Stuff If We Detect a Touch on TCH01_PIN
//       if (Touch01_Value < Touch01_Threshold) {
//         // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
//         if (DebugSerial >= 2) {
//           Serial.print("T1_TCH="); Serial.print(Touch01_Value);
//           Serial.print("/"); Serial.print(Touch01_Threshold);
//           Serial.print("-"); Serial.println(Touch01_LoopCount);
//         }
//         // STUFF - TCH01_PIN TOUCHED
//         Touch01_LoopCount++;
//       //
//       // Do Stuff If We DONT Detect a Touch on TCH01_PIN
//       } else {
//         // STUFF - TCH01_PIN NOT TOUCHED
//         Touch01_LoopCount = 0;
//       }
//       if (Touch01_LoopCount > Touch01_Loop_Threshold) {
//         batt_chrg_noled_active = false;
//       }
//     }
// }
