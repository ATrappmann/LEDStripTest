// NAME: LEDClusterController.cpp
//
// DESC:
//
// DEPENDENCIES:
//  Adafruit_DotStar library from https://github.com/adafruit/Adafruit_DotStar
//  Adafruit_NeoPixel library from https://github.com/adafruit/Adafruit_NeoPixel
//
// Copyright (c) 2020-21 by Andreas Trappmann
// All rights reserved.

//#define DEBUG 1
#include <TrappmannRobotics.h>

#include "LEDClusterController.h"
#include "LEDCluster.h"

#ifdef USE_DOTSTAR
LEDClusterController::LEDClusterController(const uint16_t numLEDs, const uint8_t ledConfig, const uint8_t maxClusters)
                     :Adafruit_DotStar(numLEDs, ledConfig) {
  this->maxClusters = maxClusters;
  clusters = new LEDClusterPtr[maxClusters];
  numClusters = 0;
}

LEDClusterController::LEDClusterController(const uint16_t numLEDs, const uint8_t dataPin, const uint8_t clockPin, const uint8_t ledConfig, const uint8_t maxClusters)
                     :Adafruit_DotStar(numLEDs, dataPin, clockPin, ledConfig) {
  this->maxClusters = maxClusters;
  clusters = new LEDClusterPtr[maxClusters];
  numClusters = 0;
}
#elif USE_NEOPIXEL
LEDClusterController::LEDClusterController(const uint16_t numLEDs, const uint8_t dataPin, const uint8_t ledConfig, const uint8_t maxClusters)
                     :Adafruit_NeoPixel(numLEDs, dataPin, ledConfig) {
  this->maxClusters = maxClusters;
  clusters = new LEDClusterPtr[maxClusters];
  numClusters = 0;
}
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif

LEDClusterController::~LEDClusterController() {
  SEROUT(millis() << F(": delete LEDClusterController\n"));
  delete[] clusters; clusters = NULL;
}

void LEDClusterController::begin() {
#ifdef USE_DOTSTAR
  Adafruit_DotStar::begin(); // initialize pins for output
  Adafruit_DotStar::show();  // turn off all LEDs
  Adafruit_DotStar::setBrightness(48);

  // test configuration setting, first 3 LEDs must be R-G-B
  Adafruit_DotStar::setPixelColor(0, COLOR_RED);
  Adafruit_DotStar::setPixelColor(1, COLOR_GREEN);
  Adafruit_DotStar::setPixelColor(2, COLOR_BLUE);
  Adafruit_DotStar::show();
#elif USE_NEOPIXEL
  Adafruit_NeoPixel::begin(); // initialize pins for output
  Adafruit_NeoPixel::show();  // turn off all LEDs
  Adafruit_NeoPixel::setBrightness(48);

  // test configuration setting, first 3 LEDs must be R-G-B
  Adafruit_NeoPixel::setPixelColor(0, COLOR_RED);
  Adafruit_NeoPixel::setPixelColor(1, COLOR_GREEN);
  Adafruit_NeoPixel::setPixelColor(2, COLOR_BLUE);
  Adafruit_NeoPixel::show();
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif

  running = true;
}

void LEDClusterController::end() {
#ifdef USE_DOTSTAR
  Adafruit_DotStar::clear();
  Adafruit_DotStar::show();
#elif USE_NEOPIXEL
  Adafruit_NeoPixel::clear();
  Adafruit_NeoPixel::show();
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif

  running = false;
}

bool LEDClusterController::addCluster(const LEDCluster *cluster, const int32_t position /* =0 */) {
  if (numClusters >= maxClusters) return false;

  if (position < numPixels()) {
    SEROUT(F("LEDClusterController::addCluster pos=") << position << LF);
    cluster->setStartPosition(position);
    cluster->setPosition(position);
    clusters[numClusters++] = cluster;
    return true;
  }

  return false;
}

void LEDClusterController::show() {
  if (!running) return;

  /*
   * clear LED strip
   */
#ifdef USE_DOTSTAR
  Adafruit_DotStar::clear();
#elif USE_NEOPIXEL
  Adafruit_NeoPixel::clear();
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif

  /*
   * set pixels of all clusters in LED strip
   */
  for (uint8_t clusterNo=0; clusterNo<numClusters; clusterNo++) {
    LEDCluster *cluster = clusters[clusterNo];
    if ((cluster->getStartInterval() > 0L) && (cluster->getStartTime() > millis())) { // not yet
      continue;
    }

    /*
     * modify pixels in cluster
     */
    if (cluster->isPeakMeter() && cluster->shouldMove()) {
      uint8_t peak = cluster->getPeakLength();
      uint16_t len = cluster->getLength() - peak;
      uint16_t width = random(len-peak, len+peak);
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
      for (uint16_t i=width; i<cluster->getLength(); i++) {
        cluster->setRGBPixel(i, COLOR_BLACK);
      }
    }
    else if (cluster->isPixelSource()) {
      uint16_t center = cluster->getLength() / 2;
      uint8_t saturationDelta = 255 / center;

      // run backward from center
      // copy pixel[i] to pixel[i-1] and reduce saturation
      for (uint16_t i=1; i<center; i++) {
        uint16_t hue = cluster->getHue(i);
        uint16_t saturation = cluster->getSaturation(i);
        if (saturation >= saturationDelta) {
          cluster->setHSVPixel(i-1, hue, saturation-saturationDelta);
        }
        else cluster->setRGBPixel(i-1, COLOR_BLACK);
      }

      // instantiate new pixel in center
      cluster->setHSVPixel(center, cluster->getSourceHue(), 255);

      // run outward from center
      // copy pixel[i] to pixel[i+1] and reduce saturation
      for (uint16_t i=cluster->getLength()-2; i>center; i--) {
        uint16_t hue = cluster->getHue(i);
        uint16_t saturation = cluster->getSaturation(i);
        if (saturation >= saturationDelta) {
          cluster->setHSVPixel(i+1, hue, saturation-saturationDelta);
        }
        else cluster->setRGBPixel(i+1, COLOR_BLACK);
      }
    }

    /*
     * modify pixels in LED strip
     */
    for (uint16_t pixelNo=0; pixelNo<numPixels(); pixelNo++) {
      if (cluster->hasPixel(pixelNo)) {
        uint32_t color;
        if (cluster->isPulsar()) {
          color = cluster->getPulsarAtIndex(pixelNo);
        }
        else {
          color = cluster->getPixelColorAtIndex(pixelNo);
        }
#ifdef USE_DOTSTAR
        Adafruit_DotStar::setPixelColor(pixelNo, color);
#elif USE_NEOPIXEL
        Adafruit_NeoPixel::setPixelColor(pixelNo, color);
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif
      }
    }

    if (cluster->shouldMove()) {
      int32_t position = cluster->getPosition();

      switch (cluster->getDirection()) {
        case NoD: // no direction
          break;
        case LtR: // left to right
          position++;
          break;
        case RtL: // right to left
          position--;
          break;
      }

      int32_t length = cluster->getLength();
      if (position <=  0L - length) {
        if (cluster->doWrapAround()) {
          cluster->setPosition(numPixels()-1);
        }
        else if (cluster->doBackAndForth()) {
          cluster->setDirection(LtR);
        }
        else if (cluster->getStartInterval() > 0L) {
          cluster->setStartTime(cluster->getStartInterval() + millis());
          cluster->setPosition(cluster->getStartPosition());
        }
        else {
          cluster->markDone();
        }
      }
      else if (position >= numPixels()) {
        if (cluster->doWrapAround()) {
          position = 1L - length;
          cluster->setPosition(1L - length);
        }
        else if (cluster->doBackAndForth()) {
          cluster->setDirection(RtL);
        }
        else if (cluster->getStartInterval() > 0L) {
          cluster->setStartTime(cluster->getStartInterval() + millis());
          cluster->setPosition(cluster->getStartPosition());
        }
        else cluster->markDone();
      }
      else cluster->setPosition(position);
    }
  }

  /*
   * display pixels of LED strip
   */
#ifdef USE_DOTSTAR
  Adafruit_DotStar::show();
#elif USE_NEOPIXEL
  Adafruit_NeoPixel::show();
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif

  /*
   * remove clusters which are done
   */
  for (uint8_t clusterNo=0; clusterNo<numClusters; clusterNo++) {
    if (clusters[clusterNo]->isDone()) {
      Serial << millis() << F(": Cluster #") << clusterNo << F(" done!\n");
      delete clusters[clusterNo];
      for (uint8_t idx=clusterNo+1; idx<numClusters; idx++) {
        clusters[idx-1] = clusters[idx];
      }
      numClusters--;
      clusters[numClusters] = NULL;
    }
  }
}

void LEDClusterController::flashAll(const uint32_t color) const {
  SEROUT(millis() << F(": flashAll color = 0x") << toHexString(color) << LF);
#ifdef USE_DOTSTAR
  uint8_t oldBrightness = Adafruit_DotStar::getBrightness();
  Adafruit_DotStar::setBrightness(255); // max
  Adafruit_DotStar::fill(color, 0, numPixels());
  Adafruit_DotStar::show();
  delay(50);
  Adafruit_DotStar::setBrightness(oldBrightness);
  Adafruit_DotStar::clear();
  Adafruit_DotStar::show();
#elif USE_NEOPIXEL
  uint8_t oldBrightness = Adafruit_NeoPixel::getBrightness();
  Adafruit_NeoPixel::setBrightness(255); // max
  Adafruit_NeoPixel::fill(color, 0, numPixels());
  Adafruit_NeoPixel::show();
  delay(50);
  Adafruit_NeoPixel::setBrightness(oldBrightness);
  Adafruit_NeoPixel::clear();
  Adafruit_NeoPixel::show();
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif
}
