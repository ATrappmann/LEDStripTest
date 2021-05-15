// NAME: LEDCluster.cpp
//
// DESC: Define a group of LEDs as an LEDCluster with common attributes.
//
// DEPENDENCIES:
//  Adafruit_DotStar library from https://github.com/adafruit/Adafruit_DotStar
//  Adafruit_NeoPixel library from https://github.com/adafruit/Adafruit_NeoPixel
//
// Copyright (c) 2020-21 by Andreas Trappmann
// All rights reserved.

//#define DEBUG 1
#include <TrappmannRobotics.h>

#include "LEDCluster.h"
#include "LEDStripTest.h"

/*
 * Constructor & Destructor
 */
LEDCluster::LEDCluster(uint16_t numLEDs) {
  SEROUT(F("LEDCluster::LEDCluster(") << numLEDs << ")\n"); 
  length = numLEDs;
  pixels = new PixelColor[length];
  if (NULL == pixels) {
    Serial << "ERROR: Cannot allocate enought memory!\n";
  }
  
  direction = NoD;
  wrapAround = false;
  backAndForth = false;
  updateInterval = 0L;
  startTime = 0L;
  startInterval = 0L;
  startPosition = 0;
  saturationInterval = 0;
  peakLength = 0;
  
  position = 0;
  done = false;  
  lastUpdate = 0L;
}

LEDCluster::~LEDCluster() {
  SEROUT(millis() << F(": delete LEDCluster\n"));
  if (NULL != pixels) delete[] pixels; pixels = NULL;
}

bool LEDCluster::isInitialized() {
  if (NULL != pixels) {
    return true;  
  }
  else return false;
}

/*
 * some handy initialization methods with predefined behavior
 */
LEDCluster *LEDCluster::initRGBPixel(const uint32_t color) {
  SEROUT(F("LEDCluster::initRGBPixel(") << toHexString(color) << ")\n");
  LEDCluster *cluster = new LEDCluster(1);
  if (cluster->isInitialized()) {
    cluster->setRGBPixel(0, color);
    return cluster;
  } 
  else return NULL;
}

LEDCluster *LEDCluster::initRGBBar(const uint32_t color, const uint16_t length) {
  LEDCluster *cluster = new LEDCluster(length);
  if (cluster->isInitialized()) {
    for (uint16_t i=0; i<length; i++) {
      cluster->setRGBPixel(i, color);
    }
    return cluster;
  }
  else return NULL;
}

LEDCluster *LEDCluster::initRGBRainbow(const uint16_t width) {
  LEDCluster *cluster = new LEDCluster(width);
  if (cluster->isInitialized()) {
    const uint16_t spread = 65536L / width;
    for (uint16_t i=0; i<width; i++) {
#ifdef USE_DOTSTAR
      uint32_t color = Adafruit_DotStar::gamma32(Adafruit_DotStar::ColorHSV(spread*i));
#elif USE_NEOPIXEL      
      uint32_t color = Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(spread*i));
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif
      cluster->setRGBPixel(i, color);
    }
    return cluster;
  }
  else return NULL;
}

LEDCluster *LEDCluster::initRGBPattern(const uint32_t color, const uint8_t pattern) {
  LEDCluster *cluster = new LEDCluster(8);
  if (cluster->isInitialized()) {
    for (uint8_t bit=0; bit<8; bit++) {
      if (0 != (pattern & (1 << bit))) {
        cluster->setRGBPixel(bit, color);
      }
      else cluster->setRGBPixel(bit, COLOR_BLACK);
    }
    return cluster;
  }
  else return NULL;
}

LEDCluster *LEDCluster::initPeakMeter(const uint16_t width, const uint8_t peakLength) {
  if (peakLength >= width) return NULL;
  LEDCluster *cluster = new LEDCluster(width+peakLength);
  if (cluster->isInitialized()) {
    cluster->peakLength = peakLength;
    for (uint16_t i=0; i<width; i++) {
      if (i < (width/2)) {
        cluster->setRGBPixel(i, COLOR_GREEN);
      }
      else if (i < (width/2 + width/3)) {
        cluster->setRGBPixel(i, COLOR_YELLOW);
      }
      else {
        cluster->setRGBPixel(i, COLOR_RED);
      }
    }
    for (uint16_t i=width; i<width+peakLength; i++) {
      cluster->setRGBPixel(i, COLOR_BLACK);
    }
    return cluster;
  }
  else return NULL;  
}

LEDCluster *LEDCluster::initPulsarPixel(const uint16_t hue, const uint8_t saturationInterval) {
  LEDCluster *cluster = new LEDCluster(1);
  if (cluster->isInitialized()) {
    cluster->setHSVPixel(0, hue, saturationInterval);
    cluster->saturationInterval = saturationInterval;
    return cluster;
  }
  else return NULL;
}

LEDCluster *LEDCluster::initPulsarBar(const uint16_t hue, const uint8_t saturationInterval, const uint16_t length) {
  LEDCluster *cluster = new LEDCluster(length);
  if (cluster->isInitialized()) {
    for (uint16_t i=0; i<length; i++) {
      cluster->setHSVPixel(i, hue, saturationInterval);
    }
    cluster->saturationInterval = saturationInterval;
    return cluster;
  }
  else return NULL;
}

LEDCluster *LEDCluster::initPulsarRainbow(const uint8_t saturationInterval, const uint16_t width) {
  LEDCluster *cluster = new LEDCluster(width);
  if (cluster->isInitialized()) {
    const uint16_t spread = 65536L / width;
    for (uint16_t i=0; i<width; i++) {
      cluster->setHSVPixel(i, spread*i, saturationInterval);
    }
    cluster->saturationInterval = saturationInterval;
    return cluster;
  }
  else return NULL;
}

/*
 * getter & setter
 */
void LEDCluster::setRGBPixel(const uint16_t no, const uint32_t color) {
  SEROUT(F("LEDCluster::setRGBPixel(") << no << ", " << toHexString(color) << ")\n");
  if (no < length) {
    pixels[no].rgbColor.red   = (color >> 16) & 0xff;
    pixels[no].rgbColor.green = (color >> 8) & 0xff;
    pixels[no].rgbColor.blue  = color & 0xff;    
  }
}

uint32_t LEDCluster::getRGBPixel(const uint16_t no) const {
  SEROUT(F("LEDCluster::getRGBPixel(") << no << ")\n");
  if (no < length) {
    uint32_t color = ((uint32_t)pixels[no].rgbColor.red << 16) | ((uint32_t)pixels[no].rgbColor.green << 8) | (uint32_t)pixels[no].rgbColor.blue;
    SEROUT(F("LEDCluster::getRGBPixel(") << no << ") color=" << toHexString(color) << LF);
    return color;  
  }
  else return 0L;
}

void LEDCluster::setHSVPixel(const uint16_t no, const uint16_t hue, const uint8_t saturation) {
  if (no < length) {
    pixels[no].hsvColor.hue = hue;
    pixels[no].hsvColor.saturation = saturation;
  }
}

uint32_t LEDCluster::getHSVPixel(const uint16_t no) const {
  if (no < length) {
#ifdef USE_DOTSTAR
    return Adafruit_DotStar::gamma32(Adafruit_DotStar::ColorHSV(pixels[no].hsvColor.hue, pixels[no].hsvColor.saturation, 255));  
#elif USE_NEOPIXEL
    return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(pixels[no].hsvColor.hue, pixels[no].hsvColor.saturation, 255));  
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif
  }
  else return 0L;
}

void LEDCluster::setDirection(const Direction dir) {
  direction = dir;
}

void LEDCluster::enableWrapAround() {
  wrapAround = true;  
  backAndForth = false; // exclusive with back-and-forth
}

void LEDCluster::enableBackAndForth() {
  backAndForth = true;  
  wrapAround = false;   // exclusive with wrap-around
}

void LEDCluster::setUpdateInterval(const uint32_t interval) {
  updateInterval = interval;
}

void LEDCluster::setStartInterval(const uint32_t interval) {
  startInterval = interval;
}

void LEDCluster::setStartPosition(const int32_t pos) {
  startPosition = pos;
}

void LEDCluster::setStartTime(const uint32_t time) {
  SEROUT(millis() << F(": restarting cluster at ") << time << LF);
  startTime = time;
}

/*
 * control methods for LEDClusterController
 */
void LEDCluster::setPosition(const int32_t pos) {
  position = pos;
}

bool LEDCluster::shouldMove() {
  uint32_t currentTime = millis();
  if (currentTime - lastUpdate > updateInterval) {
    lastUpdate = currentTime;
    return true;
  }
  else return false;
}

bool LEDCluster::hasPixel(const uint16_t pixelNo) const {
  if ((pixelNo < position) || (pixelNo >= position+length)) {
    return false;
  }
  else return true;
}

bool LEDCluster::isPulsar() const {
  if (saturationInterval > 0) {
    return true;
  }
  else return false;
}

bool LEDCluster::isPeakMeter() const {
  if (peakLength > 0) {
    return true;
  }
  else return false;
}

uint32_t LEDCluster::getPixelColorAtIndex(const uint16_t pixelNo) {
  if (!hasPixel(pixelNo)) return 0L;
  int32_t index = pixelNo - position;  
  uint32_t color = getRGBPixel(index);
  SEROUT(F("LEDCluster::getPixelColorAtIndex(") << pixelNo << ") idx=" << index << ", color=" << toHexString(color) << LF);
  return color;
}

uint32_t LEDCluster::getPulsarAtIndex(const uint16_t pixelNo) {
  if (!hasPixel(pixelNo)) return 0L;
  int32_t index = pixelNo - position;
  pixels[index].hsvColor.saturation += saturationInterval;
#ifdef USE_DOTSTAR
  uint32_t color = Adafruit_DotStar::gamma32(Adafruit_DotStar::ColorHSV(pixels[index].hsvColor.hue, pixels[index].hsvColor.saturation, 255));
#elif USE_NEOPIXEL
  uint32_t color = Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(pixels[index].hsvColor.hue, pixels[index].hsvColor.saturation, 255));
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif
  return color;
}
