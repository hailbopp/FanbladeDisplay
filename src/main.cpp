#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

//#define FASTLED_ESP8266_D1_PIN_ORDERs
#include <FastLED.h>
#include <ArduinoOTA.h>

#include "deviceconfig.h"

//FASTLED_USING_NAMESPACE
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
#define BRIGHTNESS 15
#define FRAMES_PER_SECOND 120

//// STATE and CONSTS
ESP8266WiFiMulti wifiMulti;
CRGB leds[NUM_LEDS];

void configureOTA()
{
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        { // U_SPIFFS
            type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial1.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial1.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial1.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
        {
            Serial1.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial1.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial1.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial1.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial1.println("End Failed");
        }
    });
    ArduinoOTA.begin();
    Serial1.println("Ready");
    Serial1.print("IP address: ");
    Serial1.println(WiFi.localIP());
}

void setup()
{
    Serial1.begin(115200);
    delay(50);
    Serial1.println();
    Serial1.println("Starting...");

    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID_1, WIFI_KEY_1);
    wifiMulti.addAP(WIFI_SSID_2, WIFI_KEY_2);
    wifiMulti.addAP(WIFI_SSID_3, WIFI_KEY_3);

    Serial1.print("Connecting to wifi...");

    if (wifiMulti.run() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
    configureOTA();

    FastLED.addLeds<APA102, DATA_PIN, CLK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns
void rainbow()
{
    // FastLED's built-in rainbow generator
    fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void addGlitter(fract8 chanceOfGlitter)
{
    if (random8() < chanceOfGlitter)
    {
        leds[random16(NUM_LEDS)] += CRGB::White;
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
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS - 1);
    leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for (int i = 0; i < NUM_LEDS; i++)
    { //9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for (int i = 0; i < 8; i++)
    {
        leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {
    //rainbow,
    //rainbowWithGlitter,
    confetti,
    sinelon,
    //juggle,
    bpm};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void loop()
{
    if (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected!");
        delay(1000);
    }
    ArduinoOTA.handle();
    // Call the current pattern function once, updating the 'leds' array
    //gPatterns[gCurrentPatternNumber]();
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CHSV(gHue + (i % 3), i % 8 * 50, 255);
    }

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    // do some periodic updates
    EVERY_N_MILLISECONDS(20) { gHue++; }   // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS(10) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
