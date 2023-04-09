
More info and video:
http://www.oliviermercier.com/res/projects/chessboard/

Code builds with the Arduino IDE, runs on a ESP32 (ESP-WROOM-32 38pins) or Raspberry Pico W. Select the board in Chessboar.h (BOARD_DEF_ESP32 or BOARD_DEF_RP2040)

Main game logic is in MainLoop_Game.ino .
All network logic is in NetworkLoop.ino and NetworkLoop_<something>.ino .

Pressing the side button cycles through the main game mode and the various debug modes. The code for the debug modes is in the various MainLoop_<something> files.

Uses the Lichess Board API https://lichess.org/api

See Cad/Chessboard.f3z for the Fusion360 CAD files for the computer case, see Cad/circuit.svg for the electronics.

MIT License (see LICENSE.txt)


--- LIBRARIES ---

ArduinoJson https://arduinojson.org

StreamUtils https://github.com/bblanchon/ArduinoStreamUtils

QRCode https://github.com/ricmoo/qrcode/
(had to rename that one to qrcode_renamed, otherwise it would conflict with other qrcode libraries)

XPT2046_Touchscreen https://github.com/PaulStoffregen/XPT2046_Touchscreen


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


--- ESP32 INSTALL WITH ARDUINO IDE ---

To use ESP32 in the Arduino IDE, see https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html . I then used the "ESP32 Dev Module" in Tools -> Board

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
