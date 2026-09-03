#include <Arduino.h>
// Pin definitions for Multi-Function Shield
const int LATCH_PIN = 4;
const int CLK_PIN   = 7;
const int DATA_PIN  = 8;
const int BUTTON_S1 = A1;
const int BUTTON_S2 = A2;

// Segment byte map for digits 0-9 (Active LOW)
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

// Digit select masks (Digit 0 = leftmost, Digit 3 = rightmost)
const byte DIGIT_SELECT[] = { 0xF1, 0xF2, 0xF4, 0xF8 };

// Stopwatch state variables
bool isRunning = true;
unsigned long elapsedCentis = 0; // increments every 10 ms (0.01s)
unsigned long previousMillis = 0;

// Button debounce tracking
bool lastButtonReading = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;


void handleButton() {
  int reading = digitalRead(BUTTON_S1);

  if (reading != lastButtonReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    static bool buttonState = HIGH;
    if (reading != buttonState) {
      buttonState = reading;

      // Trigger on button press (active LOW)
      if (buttonState == LOW) {
        if (isRunning) {
          isRunning = false; // Stop the timer
        } else {
          elapsedCentis = 0; // Reset
          isRunning = true;  // Start running again
        }
      }
    }
  }
  lastButtonReading = reading;
}

void updateTimer() {
  if (isRunning) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 10) { // 10ms = 0.01 seconds
      previousMillis = currentMillis;
      elapsedCentis++;
      if (elapsedCentis > 9999) {
        elapsedCentis = 0; // Roll over after 99.99s
      }
    }
  }
}

// Multiplexes each digit to eliminate flickering
void renderDisplay() {
  byte digits[4];
  digits[0] = (elapsedCentis / 1000) % 10; // Tens of seconds
  digits[1] = (elapsedCentis / 100) % 10;  // Units of seconds
  digits[2] = (elapsedCentis / 10) % 10;   // Tenths
  digits[3] = elapsedCentis % 10;          // Hundredths

  for (byte i = 0; i < 4; i++) {
    byte segData = SEGMENT_MAP[digits[i]];

    // Add decimal point after the second digit (XX.YY)
    if (i == 1) {
      segData &= 0x7F; // Clear bit 7 to turn on the decimal point
    }

    // Send data to 74HC595 shift registers
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, segData);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, DIGIT_SELECT[i]);
    digitalWrite(LATCH_PIN, HIGH);

    delayMicroseconds(500); // Brief persistence delay per digit
  }
}


void setup() {
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(BUTTON_S1, INPUT_PULLUP);
}

void loop() {
  handleButton();
  updateTimer();
  renderDisplay();
}

