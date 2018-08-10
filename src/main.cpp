#include <Arduino.h>
#include <ESP8266WiFi.h>

//#define FASTLED_ESP8266_D1_PIN_ORDERs
#include <FastLED.h>

FASTLED_USING_NAMESPACE
//// NOTES
/**
 * If FastLED fails to correctly determine GPIO order, refer to https://github.com/FastLED/FastLED/wiki/ESP8266-notes
 * 
 **/

//// DEFS
#define NUM_LEDS 64
#define DATA_PIN 3
#define CLK_PIN 2
#define COLOR_ORDER GRB
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND 120
//// VARS
CRGB leds[NUM_LEDS];
const char *WIFI_SSID = "tubes";
const char *WIFI_KEY = "lawlbears";

void setup()
{
    Serial1.begin(115200);
    delay(50);
    Serial1.println();
    Serial1.println("Starting...");

    WiFi.begin(WIFI_SSID, WIFI_KEY);
    Serial1.print("Connecting to ");
    Serial1.print(WIFI_SSID);
    Serial1.println(" ...");

    int i = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial1.print(++i);
        Serial1.print(' ');
    }

    Serial1.println('\n');
    Serial1.println("Connection established!");
    Serial1.print("IP address:\t");
    Serial1.println(WiFi.localIP());


    FastLED.addLeds<APA102, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}
void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}


void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };



uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
  
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
