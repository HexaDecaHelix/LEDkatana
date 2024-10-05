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

// Previous acceleration data and timestamp
float previousAngle = 0.0;
unsigned long previousTime = 0;

// Double tap variables
float previousMag = 0.0;
unsigned long lastTap = 0;
const unsigned long doubleTapWindow = 250;
const float threshold = 20.0;
unsigned long lastDoubleTapTime = 0;

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(153);  // Set global brightness to 60%

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  if (!accel.begin()) {
    Serial.println("Failed to initialize accelerometer!");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_16_G);
}

//... (rest of the code remains unchanged)

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  // Double tap detection
  float magnitude = sqrt(event.acceleration.x * event.acceleration.x +
                         event.acceleration.y * event.acceleration.y +
                         event.acceleration.z * event.acceleration.z);

  if (abs(magnitude - previousMag) > threshold) {
    unsigned long currentTime = millis();
    if (currentTime - lastTap < doubleTapWindow && currentTime - lastDoubleTapTime > 800) {  // 500ms cooldown
      Serial.println("Double tap detected");
      currentState = (currentState == HSL_ANIMATION) ? RAPID_CHANGE_ANIMATION : HSL_ANIMATION;
      lastDoubleTapTime = currentTime;
    }
    lastTap = currentTime;
  }

  previousMag = magnitude;

  float angle;
  byte hue;
  float currentAngle;
  unsigned long currentTime;

  switch (currentState) {
    case HSL_ANIMATION:
      angle = atan2(event.acceleration.y, event.acceleration.x) * (180.0 / PI);
      if (angle < 0) angle += 360;
      hue = map(angle, 0, 340, 0, 240);
      fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255)); 
      break;

    case RAPID_CHANGE_ANIMATION:
      currentAngle = atan2(event.acceleration.y, event.acceleration.x) * (180.0 / PI);
      currentTime = millis();
      if (abs(currentAngle - previousAngle) > 120 && (currentTime - previousTime) <= 800) {
        fill_solid(leds, NUM_LEDS, CHSV(random8(), 255, 255)); 
      }
      previousAngle = currentAngle;
      previousTime = currentTime;
      break;
  }
  
  FastLED.show();
  delay(20);
}
