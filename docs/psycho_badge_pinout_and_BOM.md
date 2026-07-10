# Psycho Badge Pinout and BOM

## Pinout

ESP32 PINS and Thier Connections on the Badge

NEOPIXEL Strings
* 27 - Neo String 1 - D10, D11, D12 - Defcon
* 14 - Neo String 2 - D20, D21, D22, D23, D24 - MOTEL

Tradional LEDs
* 22 - Built In LED
* 33 - D1 - FREE
* 32 - D2 - WIFI
* 26 - D31, D32 - Clouds
* 25 - D4 - Clouds
* 19 - D51, D52 - Mansion
* 12 - D6 - Room_Door

EINK Dev Board
* 23 - DevBd 3 - SDI
* 18 - DevBd 4 - SCK
* 5  - DevDb 5 - CS
* 17 - DevBd 6 - DS
* 16 - DevBd 7 - RES
* 34 - DevBd 8 - BSY
* 0  - Remap ESP32 MISO default pin 19 (not used by EINK but conflict with LED)

Capacitive Touch Pads
* 4  - TCH1 1 - Bats
* 2  - TCH1 2 - Satelite
* 15 - TCH1 3 - Car
* 13 - TCH1 4 - Knife

## Bill of Materials

Controller - Qty 1
* ESP32-WEMOS-LOLIN32-LITE v1.0 with USB-C connector
  * https://www.aliexpress.us/item/3256808856916958.html

Addressable LEDs - Qty 8
* WS2812D F5 5mm LED (NeoPixel)
  * https://www.aliexpress.us/item/2251832645784910.html

Traditional LEDs White - Qty 8
* 3mm Round White LEDs
  * https://www.aliexpress.us/item/2251832158199793.html

Resistors 47Ohm - Qty 10
* 47 Ohm 1/4W Resistor
  * https://www.aliexpress.us/item/3256805626032565.html

Capacitors 100nF - Qty 8
* Ceramic Disc Capacitor 0.1uF 50V 104PF 100NF
  * https://www.amazon.com/Cermant-Ceramic-Capacitor-Electronic-Components/dp/B09XV2H8JQ

SAO Connectors (Badge Side) - Qty 2
* 2.54mm 0.1" Pitch Female Pin Header Strip 2x3 Pin 6 Pin With Polarizing Key
  * https://www.aliexpress.us/item/2251832874579437.html

LIPO Battery - Qty 1
* 103040 3.7V 1200mAh Polymer Lithium Rechargeable Battery
  * https://www.aliexpress.us/item/3256811756405914.html

JST Battery Connector Cable - Qty 1
* JST 1.25mm 2 Pin Micro Female Connector Plug with Red Black Wire
  * https://www.amazon.com/dp/B013JRWCBU

Header Pins 1x2/90-Degree for Battery Holding - Qty 2
* 1x2 2.54mm Male 90-Degree Right Angle Header Pins
  * https://www.amazon.com/dp/B07C89T16T

Rubber Bands for Battery Holding - Qty 2
* #8 Rubber Bands - 7/8" x 1/16"
  * https://www.amazon.com/dp/B0BN6MKF99

Two Position Slide Switch - Qty 1
* Slide Switch SS12D00 SS12D03G5 3pin Curved 90 Degree Needle 1P2T 2 Position 5mm Handle
  * https://www.aliexpress.us/item/2251832665517359.html

EINK Display - Qty 1
* GDEM0154F61H GooDisplay 1.54 inch E-paper Four Color (Yellow and Red) Display 200x200
  * https://www.aliexpress.us/item/3256806898128301.html
  * https://www.good-display.com/product/555.html

Custom Dev Board for EINK Display - Qty 1
* Custom PCB for EINK Display (See psycho_badge_eink_dev schematic for details)
  * Parts for the custom dev board (SMT components)
    * FPC Connector FPC05-H0.5-24AH2.0 24 Position 0.5mm Pitch Double-Sided - Qty 1
      * https://www.lcsc.com/product-detail/C19267963.html
    * Ceramic Capacitor 1uF X7R ±10% 25V 0805 - Qty 8
      * https://www.lcsc.com/product-detail/C53084463.html
    * Ceramic Capacitor 4.7uF X7R ±10% 25V 0805 - Qty 2
      * https://www.lcsc.com/product-detail/C513766.html
    * Resistor 2.2Ohm ±1% 0.125W 0805 - Qty 1
      * https://www.lcsc.com/product-detail/C137549.html
    * Resistor 1MOhm ±1% 0.125W 0805 - Qty 1
      * https://www.lcsc.com/product-detail/C2933280.html
    * Schottky Diode MBR0530 0.5A 30V SOD-123 - Qty 3
      * https://www.lcsc.com/product-detail/C53100353.html
    * Transistor MOSFET SI1308EDL 30V 1.4A 360mW SOT-323 - Qty 1
      * https://www.lcsc.com/product-detail/C7603347.html
    * Inductor NR3015 47uH ±20% 350mA 1.195Ohm SMD-3mmx3mm - Qty 1
      * https://www.lcsc.com/product-detail/C42370344.html

Custom 3D printed spacer insert for PCB sandwich mounting - Qty 1
* 3D printed board spacer for mounting the PCBs
  * 4inch x 6inch overall outer dimentions, 10mm tall, 5mm think with 1.5mm holes for screw mounting, cutout on one side for esp/switch access, divots on top for lanyard hole access.
  * [/3d_parts/board_spacer.3mf](/3d_parts/board_spacer.3mf)

M2 Mounting Screws - Qty 8
* M2 Carbon Steel Cross Phillips Countersunk Pan Head Self Tapping Screws
  * https://www.aliexpress.us/item/3256803991037432.html

