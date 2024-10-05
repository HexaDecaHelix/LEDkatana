#include <FastLED.h>
#include <Adafruit_ADXL345_U.h>

#define DATA_PIN 2
#define NUM_LEDS 40
#define CS_PIN 10
#define SCK_PIN  13
#define MISO_PIN 12
#define MOSI_PIN 11

CRGB leds[NUM_LEDS];

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

enum AnimationState {
  HSL_ANIMATION,
  RAPID_CHANGE_ANIMATION
};

AnimationState currentState = HSL_ANIMATION;

float previousAngle = 0.0;
float previousMag = 0.0;
unsigned long lastTap = 0;
const unsigned long doubleTapWindow = 250;
const float threshold = 20.0;
unsigned long lastDoubleTapTime = 0;

CRGB targetColor = CRGB::Black;
CRGB currentColor = CRGB::Black;
unsigned long transitionStartTime = 0;
const unsigned long TRANSITION_DURATION = 1000; // 1 second
float lastAngle = 0.0;

CRGB lerpColor(CRGB startColor, CRGB endColor, float fraction) {
  return CRGB(
    lerp8by8(startColor.r, endColor.r, fraction * 255),
    lerp8by8(startColor.g, endColor.g, fraction * 255),
    lerp8by8(startColor.b, endColor.b, fraction * 255)
  );
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  if (!accel.begin()) {
    Serial.println("Failed to initialize accelerometer!");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_16_G);
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  // Double tap detection
  float magnitude = sqrt(event.acceleration.x * event.acceleration.x +
                         event.acceleration.y * event.acceleration.y +
                         event.acceleration.z * event.acceleration.z);

  if (abs(magnitude - previousMag) > threshold) {
    unsigned long currentTime = millis();
    if (currentTime - lastTap < doubleTapWindow && currentTime - lastDoubleTapTime > 800) {
      Serial.println("Double tap detected");
      currentState = (currentState == HSL_ANIMATION) ? RAPID_CHANGE_ANIMATION : HSL_ANIMATION;
      lastDoubleTapTime = currentTime;
    }
    lastTap = currentTime;
  }

  previousMag = magnitude;

  float currentAngle = atan2(event.acceleration.y, event.acceleration.x) * (180.0 / PI);
  if (currentAngle < 0) currentAngle += 360;

  float angleChange = abs(currentAngle - lastAngle);
  if (angleChange > 180) angleChange = 360 - angleChange;

  switch (currentState) {
    case HSL_ANIMATION: {
        byte hue = map(currentAngle, 0, 340, 0, 240);
        fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255)); 
        break;
    }

    case RAPID_CHANGE_ANIMATION: {
        if (angleChange > 120.0) {
            targetColor = CHSV(random(0, 255), 255, 255);
            transitionStartTime = millis();
        }
        unsigned long elapsedTime = millis() - transitionStartTime;
        float fraction = (float)elapsedTime / TRANSITION_DURATION;

        if (fraction < 1.0) {
            CRGB interpolatedColor = lerpColor(currentColor, targetColor, fraction);
            fill_solid(leds, NUM_LEDS, interpolatedColor);
        } else {
            currentColor = targetColor;
            fill_solid(leds, NUM_LEDS, currentColor);
        }
        break;
  }
  Serial.println((currentState == HSL_ANIMATION) ? "HSL Mode" : "Rapid Change Mode");
  lastAngle = currentAngle;
  FastLED.show();
  delay(20);
}
}
