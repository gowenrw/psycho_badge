#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

//IO settings
  // EINK Display mapping for ESP32 LOLIN D32
  // BUSY -> 34
  // RST -> 16
  // DC -> 17
  // CS -> SS(5)
  // CLK -> SCK(18)
  // SDI -> MOSI(23)
  // GND -> GND, 3.3V -> 3.3V
#define isEPD_W21_BUSY digitalRead(34)  //BUSY
#define EPD_W21_RST_0 digitalWrite(16,LOW)  //RES
#define EPD_W21_RST_1 digitalWrite(16,HIGH)
#define EPD_W21_DC_0  digitalWrite(17,LOW) //DC
#define EPD_W21_DC_1  digitalWrite(17,HIGH)
#define EPD_W21_CS_0 digitalWrite(5,LOW) //CS
#define EPD_W21_CS_1 digitalWrite(5,HIGH)


void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);


#endif 
