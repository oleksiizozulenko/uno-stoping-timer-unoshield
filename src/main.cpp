#include <Arduino.h>

const int LATCH_PIN = 4;
const int CLK_PIN   = 7;
const int DATA_PIN  = 8;

const int BUTTON_S1 = A1; // Reset to default mode (_ _ _ _ or 00.00)
const int BUTTON_S2 = A2; // Start / Stop / Resume
const int BUTTON_S3 = A3; // Mode switch: Digital <-> Letters

// 7-segment encoding for digits 0-9 (Active LOW)
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

// 7-segment encoding for letters A-Z (Active LOW)
const byte ALPHA_MAP[26] = {
  0x88, // A
  0x83, // B (b)
  0xC6, // C
  0xA1, // D (d)
  0x86, // E
  0x8E, // F
  0xC2, // G
  0x89, // H
  0xCF, // I
  0xE1, // J
  0x8A, // K
  0xC7, // L
  0xC8, // M
  0xAB, // N (n)
  0xC0, // O
  0x8C, // P
  0x98, // Q
  0xAF, // R (r)
  0x92, // S
  0x87, // T (t)
  0xC1, // U
  0xE3, // V
  0x81, // W
  0x89, // X
  0x91, // Y
  0xA4  // Z
};

const byte DIGIT_SELECT[] = { 0x01, 0x02, 0x04, 0x08 };

// Underscore pattern '_ _ _ _' (segment D active LOW)
const byte UNDERSCORES[4] = { 0xF7, 0xF7, 0xF7, 0xF7 };

// State variables
byte displayMode = 0;     // 0 = Digital Mode, 1 = Letters Mode
bool isRunning = false;   // Running state
bool isResetState = true; // True when showing default idle state (00.00 or _ _ _ _)

unsigned long elapsedCentis = 0;
unsigned long previousMillis = 0;

const unsigned long DEBOUNCE_MS = 50;

bool lastReadingS1 = HIGH, buttonStateS1 = HIGH;
unsigned long lastDebounceTimeS1 = 0;

bool lastReadingS2 = HIGH, buttonStateS2 = HIGH;
unsigned long lastDebounceTimeS2 = 0;

bool lastReadingS3 = HIGH, buttonStateS3 = HIGH;
unsigned long lastDebounceTimeS3 = 0;

void handleButtons() {
  unsigned long now = millis();

  // S1: Reset to default mode (_ _ _ _ or 00.00)
  int readingS1 = digitalRead(BUTTON_S1);
  if (readingS1 != lastReadingS1) lastDebounceTimeS1 = now;
  if ((now - lastDebounceTimeS1) > DEBOUNCE_MS) {
    if (readingS1 != buttonStateS1) {
      buttonStateS1 = readingS1;
      if (buttonStateS1 == LOW) {
        elapsedCentis = 0;
        isRunning = false;
        isResetState = true;
      }
    }
  }
  lastReadingS1 = readingS1;

  // S2: Start / Stop / Resume
  int readingS2 = digitalRead(BUTTON_S2);
  if (readingS2 != lastReadingS2) lastDebounceTimeS2 = now;
  if ((now - lastDebounceTimeS2) > DEBOUNCE_MS) {
    if (readingS2 != buttonStateS2) {
      buttonStateS2 = readingS2;
      if (buttonStateS2 == LOW) {
        if (isResetState) {
          isResetState = false;
          isRunning = true;
        } else {
          isRunning = !isRunning; // Toggle stop / resume
        }
      }
    }
  }
  lastReadingS2 = readingS2;

  // S3: Switch Mode (Digital <-> Letters)
  int readingS3 = digitalRead(BUTTON_S3);
  if (readingS3 != lastReadingS3) lastDebounceTimeS3 = now;
  if ((now - lastDebounceTimeS3) > DEBOUNCE_MS) {
    if (readingS3 != buttonStateS3) {
      buttonStateS3 = readingS3;
      if (buttonStateS3 == LOW) {
        displayMode = (displayMode == 0) ? 1 : 0;
        elapsedCentis = 0;
        isRunning = false;
        isResetState = true; // Reset to default mode display
      }
    }
  }
  lastReadingS3 = readingS3;
}

void updateTimer() {
  if (isRunning) {
    unsigned long now = millis();
    if (now - previousMillis >= 10) {
      previousMillis = now;
      elapsedCentis++;
      if (elapsedCentis > 9999) {
        elapsedCentis = 0;
      }
    }
  } else {
    previousMillis = millis();
  }
}

void sendFrame(byte segData, byte digitMask) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, 0xFF);
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, 0x00);
  digitalWrite(LATCH_PIN, HIGH);

  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, segData);
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, digitMask);
  digitalWrite(LATCH_PIN, HIGH);

  delayMicroseconds(500);
}

void renderStopwatchDigits() {
  if (isResetState) {
    // Idle Digital Mode: 00.00
    for (byte i = 0; i < 4; i++) {
      byte segData = SEGMENT_MAP[0];
      if (i == 1) segData &= 0x7F; // decimal point on digit 2
      sendFrame(segData, DIGIT_SELECT[i]);
    }
    return;
  }

  byte digits[4] = {
    (byte)((elapsedCentis / 1000) % 10),
    (byte)((elapsedCentis / 100) % 10),
    (byte)((elapsedCentis / 10) % 10),
    (byte)(elapsedCentis % 10)
  };

  for (byte i = 0; i < 4; i++) {
    byte segData = SEGMENT_MAP[digits[i]];
    if (i == 1) segData &= 0x7F; // decimal point on digit 2
    sendFrame(segData, DIGIT_SELECT[i]);
  }
}

void renderStopwatchLetters() {
  if (isResetState) {
    // Idle Letters Mode: _ _ _ _
    for (byte i = 0; i < 4; i++) {
      sendFrame(UNDERSCORES[i], DIGIT_SELECT[i]);
    }
    return;
  }

  // Independent A-Z spinning for each of the 4 sectors
  byte letterIndices[4] = {
    (byte)((elapsedCentis * 7) % 26),
    (byte)((elapsedCentis * 11 + 5) % 26),
    (byte)((elapsedCentis * 13 + 12) % 26),
    (byte)((elapsedCentis * 17 + 19) % 26)
  };

  for (byte i = 0; i < 4; i++) {
    sendFrame(ALPHA_MAP[letterIndices[i]], DIGIT_SELECT[i]);
  }
}

void setup() {
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);

  pinMode(BUTTON_S1, INPUT_PULLUP);
  pinMode(BUTTON_S2, INPUT_PULLUP);
  pinMode(BUTTON_S3, INPUT_PULLUP);
}

void loop() {
  handleButtons();
  updateTimer();

  if (displayMode == 0) {
    renderStopwatchDigits();
  } else {
    renderStopwatchLetters();
  }
}