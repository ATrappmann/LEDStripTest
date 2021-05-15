// NAME: LEDStripTest.h
//
#ifndef LEDSTRIPTEST_H
#define LEDSTRIPTEST_H

/*
 * Define 1 smart pixel configuration here:
 */
//#define USE_DOTSTAR  1
#define USE_NEOPIXEL  1

/*
 * Define 1 color configuration here:
 */
#define RGB 1
//#define RBG 1
//#define GRB 1
//#define GBR 1
//#define BRG 1
///#define BGR 1

/*
 * Do not change
 */
#ifdef USE_DOTSTAR

#include <Adafruit_DotStar.h>

#if defined(RGB)
#define COLOR_CONFIG  DOTSTAR_RGB
#elif defined(RBG)
#define COLOR_CONFIG  DOTSTAR_RBG
#elif defined(GRB)
#define COLOR_CONFIG  DOTSTAR_GRB
#elif defined(GBR)
#define COLOR_CONFIG  DOTSTAR_GBR
#elif defined(BRG)
#define COLOR_CONFIG  DOTSTAR_BRG
#elif defined(BGR)
#define COLOR_CONFIG  DOTSTAR_BGR
#else
#error Missing definition of a color configuration
#endif /* RGB */

#define APA102CONFIG  COLOR_CONFIG

#elif USE_NEOPIXEL

#include <Adafruit_NeoPixel.h>

#if defined(RGB)
#define COLOR_CONFIG  NEO_RGB
#elif defined(RBG)
#define COLOR_CONFIG  NEO_RBG
#elif defined(GRB)
#define COLOR_CONFIG  NEO_GRB
#elif defined(GBR)
#define COLOR_CONFIG  NEO_GBR
#elif defined(BRG)
#define COLOR_CONFIG  NEO_BRG
#elif defined(BGR)
#define COLOR_CONFIG  NEO_BGR
#else
#error Missing definition of a color configuration
#endif /* RGB */

#define WS2815CONFIG  (COLOR_CONFIG|NEO_KHZ800)
#define DATA_PIN      11

#else
#error Either define USE_NEOPIXEL or USE_DOTSTAR
#endif /* USE_DOTSTAR */

#endif /* LEDSTRIPTEST_H */
