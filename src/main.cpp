#include <Arduino.h>

// Pin definitions for Multi-Function Shield
const int LATCH_PIN = 4;
const int CLK_PIN   = 7;
const int DATA_PIN  = 8;
const int BUTTON_S1 = A1; // Stop & Reset
const int BUTTON_S2 = A2; // Pause / Resume (Continue)

// Segment byte map (Active LOW: 0 = ON, 1 = OFF)
const byte SEGMENT_MAP[] = {
  0xC0, // 0
  0xF9, // 1
  0xA4, // 2
  0xB0, // 3
  0x99, // 4
  0x92, // 5
  0x82, // 6
  0xF8, // 7
  0x80, // 8
  0x90  // 9
};

// Active-high digit select masks
const byte DIGIT_SELECT[] = { 0x01, 0x02, 0x04, 0x08 };

// Stopwatch state variables
bool isRunning = true;
unsigned long elapsedCentis = 0; // Increments every 10 ms (0.01s)
unsigned long previousMillis = 0;

// Debounce state tracking
const unsigned long DEBOUNCE_DELAY = 50;

bool lastReadingS1 = HIGH;
bool buttonStateS1 = HIGH;
unsigned long lastDebounceTimeS1 = 0;

bool lastReadingS2 = HIGH;
bool buttonStateS2 = HIGH;
unsigned long lastDebounceTimeS2 = 0;

void handleButtons() {
  unsigned long now = millis();

  // --- Handle Button S1 (Reset & Stop) ---
  int readingS1 = digitalRead(BUTTON_S1);
  if (readingS1 != lastReadingS1) {
    lastDebounceTimeS1 = now;
  }
  if ((now - lastDebounceTimeS1) > DEBOUNCE_DELAY) {
    if (readingS1 != buttonStateS1) {
      buttonStateS1 = readingS1;
      if (buttonStateS1 == LOW) {
        isRunning = false;
        elapsedCentis = 0; // Reset counter
      }
    }
  }
  lastReadingS1 = readingS1;

  // --- Handle Button S2 (Pause / Continue) ---
  int readingS2 = digitalRead(BUTTON_S2);
  if (readingS2 != lastReadingS2) {
    lastDebounceTimeS2 = now;
  }
  if ((now - lastDebounceTimeS2) > DEBOUNCE_DELAY) {
    if (readingS2 != buttonStateS2) {
      buttonStateS2 = readingS2;
      if (buttonStateS2 == LOW) {
        isRunning = !isRunning; // Toggle run/pause state
      }
    }
  }
  lastReadingS2 = readingS2;
}

void updateTimer() {
  if (isRunning) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 10) {
      previousMillis = currentMillis;
      elapsedCentis++;
      if (elapsedCentis > 9999) {
        elapsedCentis = 0; // Rollover at 99.99s
      }
    }
  } else {
    // Keep baseline synchronized while paused to avoid jumps on resume
    previousMillis = millis();
  }
}

void renderDisplay() {
  byte digits[4];
  digits[0] = (elapsedCentis / 1000) % 10;
  digits[1] = (elapsedCentis / 100) % 10;
  digits[2] = (elapsedCentis / 10) % 10;
  digits[3] = elapsedCentis % 10;

  for (byte i = 0; i < 4; i++) {
    byte segData = SEGMENT_MAP[digits[i]];

    // Decimal point on digit index 1 (format: XX.YY)
    if (i == 1) {
      segData &= 0x7F;
    }

    // Blank the display before shifting to eliminate segment ghosting
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, 0xFF);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, 0x00);
    digitalWrite(LATCH_PIN, HIGH);

    // Send active digit data
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, segData);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, DIGIT_SELECT[i]);
    digitalWrite(LATCH_PIN, HIGH);

    delayMicroseconds(500);
  }
}

void setup() {
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  pinMode(BUTTON_S1, INPUT_PULLUP);
  pinMode(BUTTON_S2, INPUT_PULLUP);
}

void loop() {
  handleButtons();
  updateTimer();
  renderDisplay();
}