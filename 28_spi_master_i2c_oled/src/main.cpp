/*
 * project: 28_spi_master_i2c_oled
 * Simultaneously test the SPI and I2C peripherals of an ESP32-C3 Mini with 0.42" OLED I2C display
 * This is the SPI master device which must be connected to a slave SPI device.
 * 
 * Copyright 2026, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html> */
// SPDX-License-Identifier: 0BSD

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "SSD1315.h"

//////// User configuration //////
///
///  If defined the SPI clock is divided by 8
///#define CHANGE_CLOCK_DIVIDER
///
///  The frequency of the SPI clock signal. The default is 1000000 Hz,
///  but a slower speed may be necessary if there are communication 
///  problems
///#define SPI_CLOCK  250000
///
///  Define the delay between SPI transmissions in milliseconds
#define SPI_DELAY 1000
///
///  Scan for I2C devices before testing I2C OLED display
///  If defined, should report "I2C device found at address 0x3C"
#define SCAN_12C_BUS
///
///  Rate of USB to Serial chip if used on the development board.
///  This is ignored when the native USB peripheral of the 
///  ESP SoC is used.
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

#if !defined(ARDUINO_ESP32C3_OLED_MINI) && !defined(ARDUINO_MINI_ESP32C3_OLED)   // defined in esp32c3_oled_mini.json or in mini_esp32c3_oled.json
  #error "An ESP32-C3 Mini Dev board with 0.42\" OLED display is required"
#endif  

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
#endif 

#ifndef SPI_CLOCK
#define SPI_CLOCK 1000000
#endif


#ifdef SCAN_12C_BUS

// Scan I2C bus for devices
int i2cScan(void) {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }
  return nDevices;
}
#endif

void errorRestart(const char* msg) {
  if (strlen(msg))
    Serial.println(msg);
  Serial.println("Restarting in 5 seconds");
  delay(5000);
  ESP.restart();
}

// Create display object specifying that only the SDA and SCL signals are
// connected. Presumably the RES (reset) pin of the onboard OLED is always disabled.
SSD1315 display(NO_RESET_PIN);

void setup() {
  #if (ARDUINO_USB_CDC_ON_BOOT > 0)
  Serial.begin();
  #else 
  Serial.begin(SERIAL_BAUD);
  #endif  

  #if !defined(SERIAL_BEGIN_DELAY)
    #if defined(PLATFORMIO)
      #define SERIAL_BEGIN_DELAY 5000    // 5 seconds
    #else
      #define SERIAL_BEGIN_DELAY 2000    // 2 seconds
    #endif
  #endif 
  delay(SERIAL_BEGIN_DELAY);

  Serial.println("\nProject: 28_spi_master_i2c_oled");
  Serial.println("Purpose: Simultaneously test the SPI and I2C peripherals of an ESP32-C3 Mini with 0.42\" OLED I2C display");
  Serial.println("          This is the SPI master device which must be connected to a slave SPI device");
  Serial.print(  "  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32-C3 based board");
  #endif
  Serial.println();
  Serial.println("Initializing the SPI port as a master device");
  Serial.printf("  Connect MOSI (master output pin %d) to MOSI of slave device\n", MOSI);
  Serial.printf("  Connect MISO (master input pin  %d) to MISO of slave device\n", MISO);
  Serial.printf("  Connect SCK  (clock pin  %d) to SCK of slave device\n", SCK);
  Serial.printf("  Connect SS (slave select %d) to  SS of slave device\n", SS);

  Serial.println();
    
  if (Wire.begin()) 
    Serial.println("Wire (I2C) initialized");
  else 
    errorRestart("Wire (I2C) initialization failed");

  #ifdef SCAN_12C_BUS
  if (i2cScan() < 1) 
    errorRestart("No I2C devices found");
  #endif

  Serial.println("Initializing the OLED display");
  display.begin();
  //display.setRotation(0);       // default, top edge of the display is nearest the ceramic antenna
  //display.setRotation(1);       // rotate display 90 degrees - will only see half of the [ Hello! ] box
  //display.setRotation(2);       // rotate display 180 degrees - will only see half of the [ Hello! ] box
  //display.setRotation(3);       // rotate display 270 degrees - will only see half of the [ Hello! ] box
  //display.setBrightness(0x7F);  // medium brightness - default on reset/powering up
  display.drawString(20, 15, "Hello!", 5);
  display.drawRect(1, 1, 70, 38); //fill=false, invert=false)
  display.display();

  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);                // disable chip select
  if (SPI.begin()) { 
    //SPI.setDataMode(SPI_MODE0); // default
    Serial.println("SPI port successfully initialized");
    Serial.printf("Default SPI clock divider: %lu\n", (unsigned long)SPI.getClockDivider());
    #ifdef CHANGE_CLOCK_DIVIDER
    SPI.setClockDivider(SPI_CLOCK_DIV8);   //divide the clock by 8
    Serial.printf("Set SPI clock divider to %lu (SPI_CLOCK_DIV8)\n", (unsigned long)SPI_CLOCK_DIV8);
    #endif
    Serial.printf("SPI clock frequency: %d\n", SPI_CLOCK);
  } else {
    errorRestart("SPI port could not be initialized");
  }
  delay(2000);
 }

#define BUFFER_SIZE  32

uint8_t outBuffer[BUFFER_SIZE] = {0};
uint8_t inBuffer[BUFFER_SIZE] = {0};

void dumpBuffer(const char* msg, const uint8_t* buff) {
  if (strlen(msg))
    Serial.printf("%s ", msg);
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
  count++; // increment message id
  memset(inBuffer, 0x49, sizeof(inBuffer));  // fill inBuffer  with 'I'
  memset(outBuffer, 0x4F, sizeof(inBuffer)); // fill outBuffer with 'O'

  if (snprintf((char*)outBuffer, BUFFER_SIZE, "Message %d", count) >= BUFFER_SIZE) {
    outBuffer[BUFFER_SIZE-1] = 0x3E;  // '>' overflow indication
    outBuffer[BUFFER_SIZE-1] = '\0';  // count must be getting very big!
  }

  dumpBuffer("\nMaster transmitting", outBuffer);
  Serial.println(" with transferBytes(outBuffer, inBuffer, 32)");
  dumpBuffer("InBuffer before transmission", inBuffer);       

  digitalWrite(SS, LOW); // enable Slave Select 
  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE0));  
  SPI.transferBytes(outBuffer, inBuffer, BUFFER_SIZE);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();

  // Display the SPI output and input buffers on the serial monitor
  dumpBuffer("\n  Post transaction outBuffer >", outBuffer);  
  dumpBuffer("\n  Post transaction  inBuffer <", inBuffer);       
  Serial.println(""); 
 
  // Display the output and input message numbers on the OLED display

  // making sure the 'string' is terminated
  outBuffer[BUFFER_SIZE-1] = '\0';
  inBuffer[BUFFER_SIZE-1] = '\0';

  String s((char *) outBuffer);
  s.remove(0, s.indexOf(' ')+1);  
  display.clear();
  display.drawChar(0,0,'>');
  display.drawString(15,0, s.c_str());

  s = (char *) inBuffer;
  s.remove(0, s.indexOf(' ')+1);

  display.drawChar(0,20,'<');
  display.drawString(15,20, s.c_str());

  display.display();

  #ifdef SPI_DELAY
  delay(SPI_DELAY);
  #endif
}
