# Board Definitions and Variant pins_arduino.h for the ESP32-C3 Mini with 0.42" OLED Display

*April 24, 2026*

---

##  Lists of Files and Directories in this Directory

| File | Description | Environment |
| ---  | --- | --- |
| esp32c3_oled_mini.json | pioarduino board definition where SS = 0 | PIO/pio |
| mini_esp32c3_oled.json | pioarduino board definition where SS = 8 | PIO/pio  |
| README.md | This file | PIO/pio |
| ArduinoVersion.md | Instructions on running these projects in the Arduino IDE | Arduino |
| oled_boards.txt | Board definitions for the Arduino IDE | Arduino |
| oled_arduino_sketches.zip | The projects as Arduino sketches and the 3rd party libraries | Arduino |


| Directory | Description | Environment |
| ---  | --- | --- |
| esp32c3_oled_mini | PIO/pio, Arduino |
| mini_esp32c3_oled | PIO/pio, Arduino |


## Board Definitions and Variant `pins_arduino.h` Files in PIO/pio

This directory contains two proposed `.json` board definition manifests form the ESP32-C3 Mini with 0.42" OLED Display. The `SDA` and `SCL` pin defined in both `pins_arduino.h` files are GPIO 5 and GPIO 6 respectively. These correspond to the connection between the ESP32-C3 and the onboard I2C OLED display. This means that GPIO 5 and 6 can no longer be attached to the SPI controller which was the usual assignment for Super Mini ESP32C3 boards. Consequently, the following SPI pins are defined in both variant definitions:  `MOSI = 10`, `MISO = 9` and `SCK = 7`. The variant pin definitions differ with respect to the pin assigned to the slave select signal `SS`.

| Variant | slave select `SS` pin |
| :---:  | :---: |
| esp32c3_oled_mini |  0 | 
| mini_esp32c3_oled |  8 |

The relative merits of these variants are discussed the [2.3 Better Choice](../README.md#23-better-choice)

This configuration of the files works very well in PlatformIO / pioarduino. Even when the `pioarduino-espressif32` platform is updated the sketches in this repository should still work. This is true even though an update of the platform means that the Arduino ESP32 core is updated which includes changes to the board definitions and to the variant `pins_arduino.h` files. The only foreseeable problem would be if the updated platform contained a board manifest with a name that conflicts with `esp32c3_oled_mini.json` or `mini_esp32c3_oled.json` or a variant directory with a name that conflicts with `esp32c3_oled_mini` or `mini_esp32c3_oled`. That must be rather unlikely.

## Arduino IDE

See [ArduinoVersion.md](ArduinoVersion.md) for summary instructions for using these definitions in the Arduino IDE.
