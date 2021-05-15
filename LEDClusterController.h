// NAME: LEDClusterController.h
//
// DESC: Control a group of LEDClusters and animate their behavior on an Adafruit_DotStar LED strip.
//
// DEPENDENCIES:
//  Adafruit_DotStar library from https://github.com/adafruit/Adafruit_DotStar
//  Adafruit_NeoPixel library from https://github.com/adafruit/Adafruit_NeoPixel
//
// Copyright (c) 2020-21 by Andreas Trappmann
// All rights reserved.

#ifndef LEDCLUSTERCONTROLLER_H
#define LEDCLUSTERCONTROLLER_H

#include <Arduino.h>
#include "LEDStripTest.h"

class LEDCluster;
typedef LEDCluster*  LEDClusterPtr;

class LEDClusterController : public 
#ifdef USE_DOTSTAR
  Adafruit_DotStar
#elif USE_NEOPIXEL
  Adafruit_NeoPixel
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif
{
private:
  uint8_t       maxClusters;  // maximum number of controlled LEDClusters
  LEDClusterPtr *clusters;    // array of pointers to LEDClusters
  
  uint8_t       numClusters;
  bool          running;
  
public:
#ifdef USE_DOTSTAR
  LEDClusterController(const uint16_t numLEDs, const uint8_t ledConfig, const uint8_t maxClusters);
  LEDClusterController(const uint16_t numLEDs, const uint8_t dataPin, const uint8_t clockPin, const uint8_t ledConfig, const uint8_t maxClusters);
#elif USE_NEOPIXEL
  LEDClusterController(const uint16_t numLEDs, const uint8_t dataPin, const uint8_t ledConfig, const uint8_t maxClusters);
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif

  ~LEDClusterController();
  
  void begin();
  void end();
  
  bool addCluster(const LEDCluster *cluster, const int32_t position = 0);

  void show();

  void flashAll(const uint32_t color) const;

};

#endif /* LEDCLUSTERCONTROLLER_H */ 
