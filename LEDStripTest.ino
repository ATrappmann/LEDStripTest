// NAME: LEDStripTest.ino
//
// DESC: Main program for testing an APA102 smart pixel RGB strip with Adafruit_DotStar library
//       and WS2815 smart pixel with Adafruit_NeoPixel library.
//
// Copyright (c) 2020-21 by Andreas Trappmann
// All rights reserved.

#include <TrappmannRobotics.h>
#include "LEDStripTest.h"
#include "LEDClusterController.h"
#include "LEDCluster.h"

#define NUMPIXELS     300 //271
#define MAXCLUSTER    10

#define RELAIS_PIN    5   // optional: pin for relais to turn on/off power to LED strip

/*
 * For HW-SPI use the following pins:
 * Uno: 11 for data, 13 for clock
 * Mega: 51 for data, 52 for clock
 */
#ifdef USE_DOTSTAR
LEDClusterController ledController(NUMPIXELS, APA102CONFIG, MAXCLUSTER);
#elif USE_NEOPIXEL
LEDClusterController ledController(NUMPIXELS, DATA_PIN, WS2815CONFIG, MAXCLUSTER);
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif

void setup() {  
  Serial.begin(115200);
  while (!Serial);
  Serial << F("------------------------------\n");
  Serial << F("Sketch: ") << getBaseName(__FILE__) << LF;
  Serial << F("Uploaded: ") << TrappmannRobotics::getUploadTimestamp() << LF;
  uint32_t initialFreeMemory = TrappmannRobotics::getFreeMemory();
  Serial << F("Free Memory at Start: ") << initialFreeMemory << F(" bytes\n");
#ifdef USE_DOTSTAR
  Serial << F("Uses Adafruit_DotStar: Data pin 11, Clock pin 13\n");
#elif USE_NEOPIXEL
  Serial << F("Uses Adafruit_NeoPixel: Data pin 11, opt. Backup pin 11\n");
#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif
  Serial << F("Running...\n");

  pinMode(RELAIS_PIN, OUTPUT);
  digitalWrite(RELAIS_PIN, HIGH);
  delay(100);

  ledController.begin();  // first 3 LEDs should be R-G-B
  delay(1000);

  LEDCluster *cluster1 = LEDCluster::initRGBRainbow(6);
  if (NULL != cluster1) {
    ledController.addCluster(cluster1, (NUMPIXELS-6)/2);
    Serial << F("Free Memory with Cluster1: ") << TrappmannRobotics::getFreeMemory() << F(" bytes\n");
  }

  LEDCluster *cluster2 = LEDCluster::initRGBPixel(COLOR_RED);
  if (NULL != cluster2) {
    cluster2->setDirection(LtR);
    cluster2->enableBackAndForth();
    cluster2->setUpdateInterval(1000L);
    ledController.addCluster(cluster2, 0);
    Serial << F("Free Memory with Cluster2: ") << TrappmannRobotics::getFreeMemory() << F(" bytes\n");
  }

  LEDCluster *cluster3 = LEDCluster::initRGBBar(COLOR_BLUE, 5);
  if (NULL != cluster3) {
    cluster3->setDirection(RtL);
    cluster3->enableWrapAround();
    ledController.addCluster(cluster3, NUMPIXELS-1);
    Serial << F("Free Memory with Cluster3: ") << TrappmannRobotics::getFreeMemory() << F(" bytes\n");
  }

  LEDCluster *cluster4 = LEDCluster::initRGBPattern(COLOR_GREEN, 0b11010101);
  if (NULL != cluster4) {
    cluster4->setDirection(LtR);
    cluster4->setUpdateInterval(500L);
    cluster4->setStartInterval(10000L);
    ledController.addCluster(cluster4, 0);
    Serial << F("Free Memory with Cluster4: ") << TrappmannRobotics::getFreeMemory() << F(" bytes\n");
  }
  
  LEDCluster *cluster5 = LEDCluster::initPulsarPixel(0, 1);
  if (NULL != cluster5) {
    cluster5->setDirection(LtR);
    cluster5->setUpdateInterval(750L);
    cluster5->enableWrapAround();
    ledController.addCluster(cluster5);
    Serial << F("Free Memory with Cluster5: ") << TrappmannRobotics::getFreeMemory() << F(" bytes\n");
  }

  LEDCluster *cluster6 = LEDCluster::initPulsarBar(0, 1, 3);
  if (NULL != cluster6) {
    cluster6->setDirection(RtL);
    cluster6->enableWrapAround();
    cluster6->setUpdateInterval(300L);
    ledController.addCluster(cluster6, NUMPIXELS-1);
    Serial << F("Free Memory with Cluster6: ") << TrappmannRobotics::getFreeMemory() << F(" bytes\n");
  }

  LEDCluster *cluster7 = LEDCluster::initPeakMeter(20, 17);
  if (NULL != cluster7) {
//    cluster7->setDirection(LtR);
    cluster7->setUpdateInterval(50L);
    ledController.addCluster(cluster7, 0);
    Serial << F("Free Memory with Cluster7: ") << TrappmannRobotics::getFreeMemory() << F(" bytes\n");
  }
  
  uint32_t freeMemory = TrappmannRobotics::getFreeMemory();
  Serial << F("Used Memory: ") << (initialFreeMemory - freeMemory) << LF;
  delay(1000);
}

unsigned long lastMinute = 0L;
void loop() {
  ledController.show();

  // flash all LEDs every 60secs 
  unsigned long minute = millis() / 60000L;
  if (minute > lastMinute) {
    ledController.flashAll(COLOR_WHITE);
    lastMinute = minute;
  }

  // auto stop sketch after 10min
  if (minute > 10) {
    Serial << millis() << F(": auto stop after 10min\n");
    ledController.end();
    digitalWrite(RELAIS_PIN, LOW);    
    Serial.flush();
    Serial.end();
    exit(0);
  }
}
