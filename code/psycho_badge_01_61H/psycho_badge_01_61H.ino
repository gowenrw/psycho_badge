// g0dzilla_vs badge code rev01

// Include Libraries
#include <Arduino.h>
#include <WiFi.h>
#include "Adafruit_NeoPixel.h"
#include "SPI.h"
#include <driver/adc.h>
#include <esp_wifi_types.h>
#include <esp_wifi.h>
// EINK Display Libraries
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "Ap_29demo.h"
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
#define NEO01_DATA 25
#define NEO02_DATA 26
//
// One color LED Pins
#define LED_D1 19
#define LED_D2 21
#define LED_D3 27
//
// Built-in LED
#define LED_BI 22
//
// Capacitive Touch Pins
#define TCH01_PIN 4   // ESP32 Touch0
#define TCH02_PIN 13  // ESP32 Touch4
#define TCH03_PIN 14  // ESP32 Touch6
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

// ENABLE EINK TEST LOOP - DISABLES MAIN BADGE LOOP
#define EINK_TEST_LOOP 1

// NeoPixel Properties
//
// Define NeoPixel Strips - (Num pixels, pin to send signals, pixel type, signal rate)
Adafruit_NeoPixel NEO01 = Adafruit_NeoPixel(4, NEO01_DATA, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel NEO02 = Adafruit_NeoPixel(4, NEO02_DATA, NEO_RGB + NEO_KHZ800);

// LED Variables
//
// Main LED mode 0=default 1=battleone 2=?
int main_led_mode = 0;
//
// NeoPixel Big Color Value 32 bit = (W-8bit << 24) + (R-8bit << 16) + (G-8bit <<8) + (B-8bit)
uint32_t neo_big_color = 0;
uint8_t neo_col_whi = 0;
uint8_t neo_col_red = 0;
uint8_t neo_col_grn = 0;
uint8_t neo_col_blu = 0;
// Neo Color bitvector 1=red 2=green 4=blue (3=R&G 5=R&B 6=G&B 7=R&G&B)
int neo_color_bitv = 0;

// PWM Properties
//
// Signal Frequency in Hz
const int freq = 1000;
// Duty Cycle Resolution in bits (1-16)
const int resolution = 8;
// PWM channel Assignment (0-15)
const int LED_D1_pwm = 1;

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
// Touch Initial Values
int Touch01_Value = 30;
//
// Touch Counters
//
// Touch Iteration Counter
int Touch01_IntCount = 0;
// Touch Iteration Flag
int Touch01_IntFlag = 0;
// Touch Loop Counter
int Touch01_LoopCount = 0;
// Touch Loop Threshold (Touch Held for X Loops of Main)
int Touch01_Loop_Threshold = 3;

// Analog Input Values
// int JSAO01_Value = 0;
// int JSAO02_Value = 0;

// Loop Control Properties
//
// Main Loop LED Iteration Delay Time [in ms] - Default 20
int LEDDelayTime = 20;
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

  // Configure LED PWM functionalitites per channel
  if (DebugSerial >= 2) {
    Serial.println("Configure PWM Channels");
  }
  ledcSetup(LED_D1_pwm, freq, resolution);

  // Attach the channel to the GPIO to be controlled
  if (DebugSerial >= 2) {
    Serial.println("Attach PWM Channels to LED Pins");
  }
  ledcAttachPin(LED_D1, LED_D1_pwm);

  //Normal LED output
  if (DebugSerial >= 2) {
    Serial.println("Set Output for non-PWM LED Pins");
  }
  pinMode(LED_BI, OUTPUT);

  // Initialize the NeoPixels
  if (DebugSerial >= 2) {
    Serial.println("Initialize NeoPixels");
  }
  NEO01.begin();
  // Set Neopixel Brightness (0-255 scale)
  NEO01.setBrightness(170);
  // Set an initial random color for neopixel functions
  setRandomColor();

  // Start all LEDs in OFF mode
  if (DebugSerial >= 2) {
    Serial.println("Turn OFF all LEDs");
  }
  ledAllOff();

  // Set a Random Seed
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
  SPI.begin ();

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
  //     BEGIN EINK TEST LOOP
  // //////////////////////////////////
  //  SET EINK_TEST_LOOP TO 1
  //  TO RUN EINK TEST CODE
  //  INSTEAD OF MAIN BADGE LOOP
  // //////////////////////////////////
  if (EINK_TEST_LOOP == 1) {

    if (DebugSerial >= 1) {
      Serial.println("********** EINK TESTING **********");
    }

    BI_on(); // Turn on built-in LED to show badge is on

    // /************Full display Image *******************/
    if (DebugSerial >= 1) {
      Serial.println(" *** EPD Full Display Image ***");
      Serial.println(" **** EPD_init");
    }
    EPD_init();            // Full screen update initialization.
    if (DebugSerial >= 1) {
      Serial.println(" **** PIC_display");
    }
    PIC_display(gImage_1); // To Display one image using full screen update.
    if (DebugSerial >= 1) {
      Serial.println(" **** EPD_sleep");
    }
    EPD_sleep();           // Enter sleep mode - do not delete or reduce lifespan of screen.
    delay(5000);           // Delay for 5s.
    /************Fast update mode(12s) Image *******************/
    if (DebugSerial >= 1) {
      Serial.println(" *** EPD Fast Display Image ***");
      Serial.println(" **** EPD_init_Fast");
    }
    EPD_init_Fast();       // Fast screen update initialization.
    if (DebugSerial >= 1) {
      Serial.println(" **** PIC_display");
    }
    PIC_display(gImage_1); // To Display one image using full screen update.
    if (DebugSerial >= 1) {
      Serial.println(" **** EPD_sleep");
    }
    EPD_sleep();           // Enter sleep mode - do not delete or reduce lifespan of screen.
    delay(5000);           // Delay for 5s.
    /************Full display ALL BLACK *******************/
    if (DebugSerial >= 1) {
      Serial.println(" *** EPD Fast Display ALL BLACK ***");
      Serial.println(" **** EPD_init_Fast");
    }
    EPD_init_Fast();            // Full screen update initialization.
    if (DebugSerial >= 1) {
      Serial.println(" **** Display_All_Black");
    }
    Display_All_Black();   // To Display one image using full screen update.
    if (DebugSerial >= 1) {
      Serial.println(" **** EPD_sleep");
    }
    EPD_sleep();           // Enter sleep mode - do not delete or reduce lifespan of screen.
    delay(3000);           // Delay for 3s.
    /************Full display ALL YELLOW *******************/
    if (DebugSerial >= 1) {
      Serial.println(" *** EPD Fast Display ALL YELLOW ***");
      Serial.println(" **** EPD_init_Fast");
    }
    EPD_init_Fast();            // Full screen update initialization.
    if (DebugSerial >= 1) {
      Serial.println(" **** Display_All_Yellow");
    }
    Display_All_Yellow();  // To Display one image using full screen update.
    if (DebugSerial >= 1) {
      Serial.println(" **** EPD_sleep");
    }
    EPD_sleep();           // Enter sleep mode - do not delete or reduce lifespan of screen.
    delay(3000);           // Delay for 3s.
    /************Full display ALL RED *******************/
    if (DebugSerial >= 1) {
      Serial.println(" *** EPD Fast Display ALL RED ***");
      Serial.println(" **** EPD_init_Fast");
    }
    EPD_init_Fast();            // Full screen update initialization.
    if (DebugSerial >= 1) {
      Serial.println(" **** Display_All_Red");
    }
    Display_All_Red();     // To Display one image using full screen update.
    if (DebugSerial >= 1) {
      Serial.println(" **** EPD_sleep");
    }
    EPD_sleep();           // Enter sleep mode - do not delete or reduce lifespan of screen.
    delay(3000);           // Delay for 3s.
    /************Full display ALL WHITE *******************/
    if (DebugSerial >= 1) {
      Serial.println(" *** EPD Fast Display ALL WHITE ***");
      Serial.println(" **** EPD_init_Fast");
    }
    EPD_init_Fast();            // Full screen update initialization.
    if (DebugSerial >= 1) {
      Serial.println(" **** Display_All_White");
    }
    Display_All_White();   // To Display one image using full screen update.
    if (DebugSerial >= 1) {
      Serial.println(" **** EPD_sleep");
    }
    EPD_sleep();           // Enter sleep mode - do not delete or reduce lifespan of screen.
    delay(5000);           // Delay for 5s.

  }
  // //////////////////////////////////
  //     END EINK TEST LOOP
  // //////////////////////////////////

  // //////////////////////////////////
  //     START OF MAIN BADGE LOOP
  // //////////////////////////////////
  //     ENSURE EINK_TEST_LOOP NOT 1
  //     TO ENABLE MAIN BADGE LOOP
  // //////////////////////////////////
  if (EINK_TEST_LOOP != 1) {

    if (DebugSerial >= 1) {
      Serial.println("********** MAIN BADGE LOOP **********");
    }

    // Capacitive Touch Dynamic Threshold Adjustment
    // Adjust thresholds UP to account for assembly conditions and battery vs usb
    Touch01_Value = touchRead(TCH01_PIN);
    if ( (Touch01_Value / Touch01_Threshold) > 2 ) { Touch01_Threshold = int(Touch01_Threshold * 1.8); }

    // //////////////////////////////////
    //     START OF ITERATION LOOP
    // //////////////////////////////////
    
    // Iterate 0 to 254
    for(int i=0; i<255; i++){
      // Set position value to iteration value
      int pos = i;

      // DEBUG - Print current Iteration value to serial console for troubleshooting
      if (DebugSerial >= 2) {
        Serial.print(" Iteration="); Serial.print(i);
        Serial.print(" Pos="); Serial.print(pos);
      }

      //
      // TOUCH
      //
      // Read Touch Values
      Touch01_Value = touchRead(TCH01_PIN);
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
        // STUFF - TCH01_PIN TOUCHED
        if (Touch01_IntFlag == 0){
          // Put stuff to happen once per iteration loop here
          Touch01_IntFlag = 1;
        }
        // Put stuff to happen every iteration here
        Touch01_IntCount++;
        //
        // **************
        // FUNCTION TO CALL GOES HERE
        // EXAMPLE:
        // monarch_neo_color();
        // **************
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

      // Read Analog Input Values
      // JSAO01_Value = analogRead(JSAO01);
      // JSAO02_Value = analogRead(JSAO02);
      // Display Analog Values
      // if (DebugSerial >= 2) {
      //   Serial.print(" SAO1="); Serial.print(JSAO01_Value);
      //   Serial.print(" SAO2="); Serial.print(JSAO02_Value);
      // }

      if (main_led_mode == 0) {
        //
        // DEFAULT MODE
        //
        // First of three position groups i 0-84
        if (pos < 85) {
          //
          // LED FUNCTIONS
          BI_blink_three(pos);
        // Second of three position groups i 85-169 (pos-85 = 0-84)
        } else if (pos < 170) {
          pos = pos - 85;
          //
          // LED FUNCTIONS
          BI_blink_three(pos);
        // Third of three position groups i 170-254 (pos-170 = 0-84)
        } else {
          pos = pos -170;
          //
          // LED FUNCTIONS
          BI_blink_three(pos);
          // Split third group 3/4 (pos 0-42) for even number of transitions
          if (pos <43) {
            //
          // Split third group 4/4 (pos 43-84) for even number of transitions
          } else {
            //
          }
        }
      } else if (main_led_mode == 1) {
        //
        // BATTLE MODE
        //
        // First of three position groups i 0-84
        if (pos < 85) {
          //
          // LED FUNCTIONS
          BI_blink_two(pos);
        // Second of three position groups i 85-169 (pos-85 = 0-84)
        } else if (pos < 170) {
          pos = pos - 85;
          //
          // LED FUNCTIONS
          BI_blink_two(pos);
        // Third of three position groups i 170-254 (pos-170 = 0-84)
        } else {
          pos = pos -170;
          //
          // LED FUNCTIONS
          BI_blink_two(pos);
          // Split third group 3/4 (pos 0-42) for even number of transitions
          if (pos <43) {
            //
          // Split third group 4/4 (pos 43-84) for even number of transitions
          } else {
            //
          }
        }
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

      // DEBUG - Print Carriage Return for iteration level debug output
      if (DebugSerial >= 2) {
        Serial.println();
      }

      // Display Neopixel values
      neo_show();

      // Pause the loop to display everything
      delay(LEDDelayTime);

      // END OF FOR ITERATION LOOP
    }
    // //////////////////////////////////
    //     END OF ITERATION LOOP
    // //////////////////////////////////

    // Touch Loop Counters - USE TBD
    if (Touch01_IntCount >= 1) { Touch01_LoopCount++; Touch01_IntCount = 0; } else { Touch01_LoopCount = 0; }

    // Reset Touch Iteration Flags
    Touch01_IntFlag = 0;

    // Turn off all LEDs at end of loop (Optional for troubleshooting)
    // ledAllOff();

    // //////////////////////////////////////////////////
    //
    // Launch BATT_CHRG_NOLED Alternate Mainline Code When
    // Touch01_LoopCount exceeds Touch01_Loop_Threshold
    // Touch01 is the Monarch Logo
    //
    // //////////////////////////////////////////////////
    if (Touch01_LoopCount > Touch01_Loop_Threshold) {
      //
      Serial.println("LONG TOUCH DETECTED on TCH02 - JUMP TO ALTERNATE CODE");
      //
      ledAllOff();
      //
      Touch01_LoopCount = 0;
      //
      // Alternate code loop
      batt_chrg_noled();
      //
      // END ALTERNATE MAIN LOOP
      Serial.println("****************************************");
      Serial.println("***** EXITING BATT_CHRG_NOLED MODE *****");
      Serial.println("****************************************");
      //
      ledAllOff();
      //
      Touch01_LoopCount = 0;
      //
      main_led_mode = 0;
      // Pause before exiting
      delay(100);
    }

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
// LED Functions
// //////////////////////////////////////////////////
void ledAllOff() {
    digitalWrite(LED_BI, HIGH); // HIGH = OFF?
    ledcWrite(LED_D1_pwm, 0);
    NEO01.setPixelColor(0, 0, 0, 0);
    NEO01.setPixelColor(1, 0, 0, 0);
    NEO01.setPixelColor(2, 0, 0, 0);
    NEO01.setPixelColor(3, 0, 0, 0);
    NEO01.show();
}
//
void ledPwmAllOn() {
    ledcWrite(LED_D1_pwm, 255);
}
//
void ledPwmAllOff() {
    ledcWrite(LED_D1_pwm, 0);
}
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
void batt_chrg_noled() {
    // Set an exit var
    bool batt_chrg_noled_active = true;
    //
    while (batt_chrg_noled_active) {
      BI_off();
      // Print Serial Message About Mode
      Serial.println("****************************************");
      Serial.println("****************************************");
      Serial.println("********* BATT_CHRG_NOLED MODE *********");
      Serial.println("****************************************");
      Serial.println("*** ACTIVATED BY LONG TOUCH ON TCH02 ***");
      Serial.println("***      THE MONARCH LOGO BUTTON     ***");
      Serial.println("****************************************");
      Serial.println("** LONG PRESS AGAIN TO EXIT THIS MODE **");
      Serial.println("****************************************");
      Serial.println("****************************************");
      // Pause
      delay(3500);
      // Turn on-board LED on briefly to show badge is still on
      BI_on();
      // Pause
      delay(500);
      //
      // Touch for exit mode settings
      //
      Touch01_Value = touchRead(TCH01_PIN);
      // Do Stuff If We Detect a Touch on TCH01_PIN
      if (Touch01_Value < Touch01_Threshold) {
        // DEBUG - Print current Touch value/threshold to serial console for troubleshooting
        if (DebugSerial >= 2) {
          Serial.print("T1_TCH="); Serial.print(Touch01_Value);
          Serial.print("/"); Serial.print(Touch01_Threshold);
          Serial.print("-"); Serial.println(Touch01_LoopCount);
        }
        // STUFF - TCH01_PIN TOUCHED
        Touch01_LoopCount++;
      //
      // Do Stuff If We DONT Detect a Touch on TCH01_PIN
      } else {
        // STUFF - TCH01_PIN NOT TOUCHED
        Touch01_LoopCount = 0;
      }
      if (Touch01_LoopCount > Touch01_Loop_Threshold) {
        batt_chrg_noled_active = false;
      }
    }
}
