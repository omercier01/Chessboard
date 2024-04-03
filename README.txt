
More info and video:
http://www.oliviermercier.com/res/projects/chessboard/

Code builds with the Arduino IDE, runs on a ESP32 (ESP-WROOM-32 38pins) or Raspberry Pico W. Select the board in Chessboar.h (BOARD_DEF_ESP32 or BOARD_DEF_RP2040)

Main game logic is in MainLoop_Game.ino .
All network logic is in NetworkLoop.ino and NetworkLoop_<something>.ino .

Pressing the side button cycles through the main game mode and the various debug modes. The code for the debug modes is in the various MainLoop_<something> files.

Uses the Lichess Board API https://lichess.org/api

See Cad/Chessboard.f3z for the Fusion360 CAD files for the computer case, see Cad/circuit.svg for the electronics.

MIT License (see LICENSE.txt)



--- INSTALLATION ---

install Arduino IDE 2.3.2

- For ESP32, can be installed from the Arduino IDE. esp32 by Espressif Systems, version 2.0.11 . Then select the board ESP32 Dev Module .
- For Raspberry Pico, follow this to install the Pico W on the Arduino IDE: https://dronebotworkshop.com/picow-arduino/  (TLDR add this string to the board manager list in the Arduino IDE Preference menu, and then the PicoW will show up in the boards manager: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json ). Then from the board manager, install Raspberry Pi Pico/RP2040 by Earle F. Philhower, III version 3.1.0 . THE VERSION MATTERS. The code might not compile on more recent versions. Then Select the board Raspberry Pi Pico W .

Install the following libraries from the Arduino library manager:
- TFT_eSPI 2.5.43
- ArduinoJson 7.0.4
- StreamUtils 1.8.0
- FreeRTOS 11.0.1-5
- XPT2046_Touchscreen 1.4
- GFX Library for Arduino 1.4.6

in C:\Users\<user>\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h , comment the line
#include <User_Setup.h>
and uncomment
#include <User_Setups/Setup60_RP2040_ILI9341.h>
and in Setup60_RP2040_ILI9341.h set the pins as

#define TFT_MISO  0
#define TFT_MOSI  3
#define TFT_SCLK  2
#define TFT_CS   4  // Chip select control pin
#define TFT_DC   5  // Data Command control pin
#define TFT_RST  6  // Reset pin (could connect to Arduino RESET pin)
//#define TFT_BL     // LED back-light
#define TOUCH_CS 7     // Chip select pin (T_CS) of touch screen




--- OTHER ASSETS ---

font from https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts

CAD parts from:
https://grabcad.com/library/2-8in-lcd-module-1
https://grabcad.com/library/esp-32s-development-board-1
https://grabcad.com/library/fci-hdmi-right-angle-header-1
https://grabcad.com/library/rocker-switch-kcd1-101-2pin-3pin-1
https://grabcad.com/library/small-power-switches-rocker-type-10-5x15mm-1

Chess glyphs based on the SansSerif font
https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode
https://en.wikipedia.org/wiki/Chess_symbols_in_Unicode#/media/File:Chess_symbols.svg


--- ESP32 MANUAL INSTALL AND UPLOAD ---

Iff needed, the ESP32 board can also be installed manually, see https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html . I then used the "ESP32 Dev Module" in Tools -> Board

Some ESP32 dev boards have issues when uploading code, see https://randomnerdtutorials.com/solved-failed-to-connect-to-esp32-timed-out-waiting-for-packet-header/
(I used a 2.2uF capacitor)


--- INCREASE APP SIZE IN PARTITION ---

If not enough app memory to upload sketch to the ESP32, we can change the memory partition to e.g. the Util/custom_maxApp.csv file. To change the partition in the Arduino IDE, put custom_maxApp.csv in 
C:\Users\<user>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.2\tools\partitions
or wherever the other esp32 partition files are. Then also modify
C:\Users\<user>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.2\boards.txt
and add these lines below the other ESP32 partitions (not esp32s3 or others similar):
esp32.menu.PartitionScheme.custom_maxApp=custom_maxApp
esp32.menu.PartitionScheme.custom_maxApp.build.partitions=custom_maxApp
esp32.menu.PartitionScheme.custom_maxApp.upload.maximum_size=4128768

Restart the arduino IDE, and select Tools -> Partition Scheme -> custom_maxApp. The max app should be increased, try to compile the sketch to see what max app size the ide says there is.

Note: this link ( https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html ) says the default program is at address 0x10000, so no point in removing the default partition stuff before then in the partition (although some of that space might not be reserved and available for spiff or data storage, not sure).

Note: See this for more info https://iotespresso.com/how-to-set-partitions-in-esp32/
