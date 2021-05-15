// NAME: LEDCluster.h
//
// DESC: Define a group of LEDs as an LEDCluster with common attributes.
//
// Copyright (c) 2020-21 by Andreas Trappmann
// All rights reserved.

#ifndef LEDCLUSTER_H
#define LEDCLUSTER_H

#include "Arduino.h"

#define COLOR_RED     ((uint32_t)0xFF0000)
#define COLOR_GREEN   ((uint32_t)0x00FF00)
#define COLOR_YELLOW  ((uint32_t)0xFFFF00)
#define COLOR_BLUE    ((uint32_t)0x0000FF)
#define COLOR_CYAN    ((uint32_t)0x00FFFF)
#define COLOR_MAGENTA ((uint32_t)0xFF00FF)
#define COLOR_BLACK   ((uint32_t)0x000000)
#define COLOR_WHITE   ((uint32_t)0xffffff)

union PixelColor {
  struct {
    uint8_t   red;
    uint8_t   green;
    uint8_t   blue;
  } rgbColor;
  struct {
    uint16_t  hue;
    uint8_t   saturation;
    // no value, always 255!
  } hsvColor;
};

enum Direction {
  NoD,  // no direction
  LtR,  // left to right
  RtL,  // right to left
  BaF   // back and forth
};

class LEDCluster {
private:
  uint16_t    length;           // length of cluster
  PixelColor  *pixels;          // array of colors for the LEDs in the cluster

  /*
   * attributes of the LEDCluster
   */
  Direction direction;          // direction of movement
  bool      wrapAround;         // restart cluster at other end of LED strip if it moves out of the end of the strip 
  bool      backAndForth;       // change direction, when cluster reaches end of LED strip
  uint32_t  updateInterval;     // speed of movement in milliseconds
  uint32_t  startTime;          // start time for activation of this cluster in milliseconds
  uint32_t  startInterval;      // periodic start interval in milliseconds
  int32_t   startPosition;      // position of this cluster in the LED strip, when the cluster gets started
  uint8_t   saturationInterval; // pulse saturation
  uint8_t   peakLength;
  
  /*
   * control attributes for LEDClusterController
   */
  int32_t   position;         // current position; not an uint, may be negative!
  bool      done;             // flag, if cluster is done (will be deleted)
  uint32_t  lastUpdate;       // in milliseconds
  
  /*
   * some handy initialization methods with predefined behavior
   */
public:
  static LEDCluster *initRGBPixel(const uint32_t color);
  static LEDCluster *initRGBBar(const uint32_t color, const uint16_t length);
  static LEDCluster *initRGBRainbow(const uint16_t width);
  static LEDCluster *initRGBPattern(const uint32_t color, const uint8_t pattern);
  static LEDCluster *initPeakMeter(const uint16_t width, const uint8_t peakLength);
  
  static LEDCluster *initPulsarPixel(const uint16_t hue, const uint8_t saturationInterval);
  static LEDCluster *initPulsarBar(const uint16_t hue, const uint8_t saturationInterval, const uint16_t length);
  static LEDCluster *initPulsarRainbow(const uint8_t saturationInterval, const uint16_t width);
  
public:
  LEDCluster(const uint16_t numLEDs);
  ~LEDCluster();

  bool isInitialized();
  
  /*
   * getter & setter methods for attributes of the LEDcluster
   */
  void setRGBPixel(const uint16_t no, const uint32_t color);
  uint32_t getRGBPixel(const uint16_t no) const;
  
  void setHSVPixel(const uint16_t no, const uint16_t hue, const uint8_t saturation);
  uint32_t getHSVPixel(const uint16_t no) const;
  
  void  setDirection(const Direction dir);
  Direction getDirection() const  { return direction; }

  void setUpdateInterval(const uint32_t interval);
  uint32_t getUpdateInterval() const  { return updateInterval; }

  void setStartTime(const uint32_t time);
  uint32_t getStartTime() const { return startTime; }

  void setStartInterval(const uint32_t interval);
  uint32_t getStartInterval() const   { return startInterval; }

  void setStartPosition(const int32_t pos);
  int32_t getStartPosition() const  { return startPosition; }  
  
  void  enableWrapAround();
  bool  doWrapAround() const  { return wrapAround; }

  void  enableBackAndForth();
  bool  doBackAndForth() const  { return backAndForth; }

  /*
   * controlling methods for LEDClusterController
   */
  uint16_t  getLength() const { return length; }
  uint8_t   getPeakLength() const { return peakLength; }

  void    setPosition(const int32_t pos);
  int32_t getPosition() const { return position; }

  void  markDone()      { done = true; }
  bool  isDone() const  { return done; }
   
  bool  hasPixel(const uint16_t pixelNo) const;
  bool  isPulsar() const;
  bool  isPeakMeter() const;
  uint32_t getPixelColorAtIndex(const uint16_t pixelNo);
  uint32_t getPulsarAtIndex(const uint16_t pixelNo);
  
  bool  shouldMove();
};

#endif /* LEDCLUSTER_H */
