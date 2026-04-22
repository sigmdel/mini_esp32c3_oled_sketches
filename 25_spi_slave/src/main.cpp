/*
 * spi_slave
 * Test of SPI communication; SPI slave device firmware.
 * 
 * See spi_master or spi_master_i2c_oled for the firmware for the other device.
 *
 * This is a stub to satisfy the Arduino IDE, the source code is in
 * the main.cpp file in the same directory.
 *
 * This sketch will compile in the Arduino IDE
 *
 *  1- Add https://espressif.github.io/arduino-esp32/package_esp32_index.json
 *     in the Additional Boards Manager URLS in the Preferences window.
 *  2- Install platform esp32 by Espressif version 3.3.7 or newer with the Boards Manager
 *  3- Select the Nologo ESP32C3 Super Mini board
 *
 * Michel Deslierres
 * April 21, 2026
 *
 * Based on tranfer_one_by_one_slave.ino by hidekitai in the prerequisite ESP32SPISlave library
 * @ https://github.com/hideakitai/ESP32SPISlave/blob/main/examples/transfer_one_by_one/transfer_one_by_one_slave/transfer_one_by_one_slave.ino
 * 
 * Copyright 2026, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html> */
// SPDX-License-Identifier: 0BSD

#include <Arduino.h>
#include <SPI.h>
#include <ESP32SPISlave.h>


//////// User configuration //////
///
///  If defined the SPI clock is divided by 8
///#define CHANGE_CLOCK_DIVIDER
///
///  Define the delay between SPI transmissions in milliseconds
#define SPI_DELAY 1000
///
/// Rate of USB to Serial chip if used on the development board.
/// This is ignored when the native USB peripheral of the 
/// ESP SoC is used.
#define SERIAL_BAUD 115200
///
///  Time in milliseconds to wait after Serial.begin() in 
///  the setup() function. If not defined, it will be set
///  to 5000 if running in the PlaformIO IDE to manually switch
///  to the serial monitor otherwise to 2000 if an native USB 
///  peripheral is used or 1000 if a USB-serial adpater is used.
///#define SERIAL_BEGIN_DELAY 8000
///
//////////////////////////////////


#if !defined(CONFIG_IDF_TARGET_ESP32C3)
  #error An ESP32-C3 SoC is required
#endif  

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

ESP32SPISlave slave;

void setup() {
  #if !defined(SERIAL_BEGIN_DELAY)
    #if defined(PLATFORMIO)
      #define SERIAL_BEGIN_DELAY 5000    // 5 seconds
    #else
      #define SERIAL_BEGIN_DELAY 2000    // 2 seconds
    #endif
  #endif 

  #if (ARDUINO_USB_CDC_ON_BOOT > 0)
  Serial.begin();
  delay(SERIAL_BEGIN_DELAY);
  #else 
  Serial.begin(SERIAL_BAUD);
  delay(SERIAL_BEGIN_DELAY);
  Serial.println();
  #endif  

  Serial.println("\nProject: spi_slave");
  Serial.println("Purpose: SPI slave device in test of SPI communication between two devices");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32-C3 based board");
  #endif
  #if defined(ARDUINO_ESP32C3_DEV)
    if (!String(ARDUINO_VARIANT).equals("nologo_esp32c3_super_mini")) {
      Serial.println("Warning: Expected the Nologo ESP32C3 Super Mini board definition");
  }
  #endif

  Serial.println();
  Serial.println("Initializing the SPI port as a slave device");
  Serial.printf("  SPI bus (FSPI) : %d\n", FSPI);
  Serial.printf("  Connect MOSI (master output pin %d) to MOSI of master device\n", MOSI);
  Serial.printf("  Connect MISO (master input pin  %d) to MISO of master device\n", MISO);
  Serial.printf("  Connect SCK  (clock pin  %d) to SCK of master device\n", SCK);
  Serial.printf("  Connect SS (slave select %d) to  SS of master device\n", SS);
  
  #define QUEUE_SIZE 1 
  slave.setDataMode(SPI_MODE0);     // this is the default
  slave.setQueueSize(QUEUE_SIZE);   // this is the default 
  //slave.begin(uint8_t spi_bus, int sck, int miso, int mosi, int ss)
  //slave.begin(FSPI, SCK, MISO, MOSI, SS))
	//slave.begin(FSPI);  
  if (slave.begin()) {
    Serial.println("SPI slave started successfully");
  } else {
    Serial.println("Unable to start SPI slave");
    Serial.println("Resetting system in 5 seconds");
    delay(5000);
    ESP.restart();    
  }
}

#define BUFFER_SIZE  32

uint8_t outBuffer[BUFFER_SIZE] = {0};
uint8_t inBuffer[BUFFER_SIZE] = {0};

void dumpBuffer(const char* msg, const uint8_t* buff) {
  if (strlen(msg))
    Serial.printf("%s: ", msg);
  for (int i = 0; i <  BUFFER_SIZE; ++i) {
    uint8_t c = buff[i]; 
    if ((c > 31) && (c < 127))
      Serial.print((char) c);
    else if (c == 0)
      Serial.print("\\0"); 
    else 
      Serial.printf("0x%.2x ", c);
  }    
}

int count = 0;

void loop (void) {
  memset(inBuffer, 0x69, sizeof(inBuffer));   // fill inBuffer  with 'i'
  memset(outBuffer, 0x6F, sizeof(outBuffer)); // fill outBuffer with 'o'

  Serial.println();
  count++;

  if (snprintf((char*) outBuffer, BUFFER_SIZE, "Reply %d", count) >= BUFFER_SIZE) {
    outBuffer[BUFFER_SIZE-1] = 0x3E;  // '>' overflow indication
    outBuffer[BUFFER_SIZE-1] = '\0';  // count must be getting very big!
  }

  // Complete one 32 byte transaction initiated by the master device. The incoming master 
  // data will be copied to the inBuffer, while the content of the outBuffer is synchronously 
  // sent to the master
  const size_t received_bytes = slave.transfer(outBuffer, inBuffer, BUFFER_SIZE); 

  // Show received data
  Serial.printf("Received %d bytes from the master, reply count: %d", received_bytes, count);
  dumpBuffer("\n  inBuffer", inBuffer);
  dumpBuffer("\n outBuffer", outBuffer);  
  Serial.println();   
}
