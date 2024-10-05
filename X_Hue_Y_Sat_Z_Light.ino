#include <SPI.h>
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

#define SMOOTHING 5
float xHistory[SMOOTHING], yHistory[SMOOTHING], zHistory[SMOOTHING];
int readIndex = 0;

void setup() {
  Serial.begin(115200);

  // Initialize FastLED
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

 
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  if (!accel.begin()) {
    Serial.println("Failed to initialize accelerometer!");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_16_G);  // Set accelerometer range to ±16G
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  // Correct for offsets
  float correctedX = event.acceleration.x ;
  float correctedY = event.acceleration.y ;
  float correctedZ = event.acceleration.z ;

  // Store readings in history arrays
  xHistory[readIndex] = correctedX;
  yHistory[readIndex] = correctedY;
  zHistory[readIndex] = correctedZ;

  // Calculate average
  float avgX = average(xHistory);
  float avgY = average(yHistory);
  float avgZ = average(zHistory);

  // Map the averaged acceleration values to LED color values
  int redVal = map(avgX, -5, 5, 30, 255);
  int greenVal = map(avgY, -5, 5, 30, 255);
  int blueVal = map(avgZ, -5, 5, 30, 255);

  fill_solid(leds, NUM_LEDS, CRGB(redVal, greenVal, blueVal));
  FastLED.show();

  // Increment read index and wrap if necessary
  readIndex++;
  if (readIndex >= SMOOTHING) readIndex = 0;

  delay(10);
}

float average(float *arr) {
  float sum = 0;
  for (int i = 0; i < SMOOTHING; i++) {
    sum += arr[i];
  }
  return sum / SMOOTHING;
}