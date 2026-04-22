/*
 * i2c_oled.ino
 * Test the I2C peripheral of an ESP32-C3 Mini with 0.42" OLED I2C display
 *
 * Copyright 2026, Michel Deslierres. No rights reserved, this code is in the public domain.
 * In those jurisdictions where this may be a problem, the BSD Zero Clause License applies.
 * <https://spdx.org/licenses/0BSD.html> */
// SPDX-License-Identifier: 0BSD

#include <Arduino.h>
#include <Wire.h>
#include "SSD1315.h"

//////// User configuration //////
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

#if !defined(ARDUINO_ESP32C3_OLED_MINI)  // defined in esp32c3_oled_mini.json
  #error "An ESP32-C3 Mini Dev board with 0.42\" OLED display is required"
#endif  

#if (ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 3, 7))    
  #warning ESP32 Arduino core version 3.3.7 or newer is available
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

  Serial.println("\n\nProject: 27_i2c_oled");
  Serial.println("Purpose: Test the I2C display of an ESP32-C3 mini development board with 0.42\" OLED display");
  Serial.print("  Board: ");
  #ifdef ARDUINO_BOARD
  Serial.println(ARDUINO_BOARD);
  #else 
  Serial.println("Unknown ESP32-C3 based board");
  #endif

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
}

const uint8_t levels = 4;
const uint8_t contrast[levels] = {31, 63, 127, 255};

void testContrast(void) {
  for (int i=0; i < levels; i++) {
    uint8_t cval = contrast[i];
    display.setBrightness(cval);
    Serial.printf("  Contrast: %3d (0x%02x)\n", cval, cval);
    delay(3000);
  }  
  display.setBrightness(127); // back to default
}

void loop() {
  Serial.println("\nDisplay: Normal");
  testContrast();

  // put the display to sleep to save power
  Serial.println("Display: Sleeping");
  display.sleep(true);
  delay(5000);

  // back to normal for a short while
  Serial.println("Display: Normal");
  display.sleep(false);
  delay(1000);

  // before inverting display
  Serial.println("Display: Inverted");
  display.invert(true);
  testContrast();
  display.invert(false);
}
