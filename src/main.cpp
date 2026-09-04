#include <Arduino.h>

const int LATCH_PIN = 4;
const int CLK_PIN   = 7;
const int DATA_PIN  = 8;

const int BUTTON_S1 = A1; // Stop & Start with reset
const int BUTTON_S2 = A2; // Pause / Resume
const int BUTTON_S3 = A3; // Mode switch: Timer / Words

// 7-segment encoding (Active LOW)
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

struct WordPattern {
  const char* label;
  byte segs[4];
};

const WordPattern WORDS[] = {
  { "HELL", { 0x89, 0x86, 0xC7, 0xC7 } },
  { "HELP", { 0x89, 0x86, 0xC7, 0x8C } },
  { "COOL", { 0xC6, 0xC0, 0xC0, 0xC7 } },
  { "OPEN", { 0xC0, 0x8C, 0x86, 0xAB } },
  { "STOP", { 0x92, 0x87, 0xC0, 0x8C } },
  { "FIRE", { 0x8E, 0xF9, 0xAF, 0x86 } },
  { "GOOD", { 0x90, 0xC0, 0xC0, 0xA1 } }
};
const byte NUM_WORDS = sizeof(WORDS) / sizeof(WORDS[0]);

const byte DIGIT_SELECT[] = { 0x01, 0x02, 0x04, 0x08 };

byte displayMode = 0; // 0 = timer, 1..N = words

bool isRunning = true;
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

  // S1: Reset & Start/Stop
  int readingS1 = digitalRead(BUTTON_S1);
  if (readingS1 != lastReadingS1) lastDebounceTimeS1 = now;
  if ((now - lastDebounceTimeS1) > DEBOUNCE_MS) {
    if (readingS1 != buttonStateS1) {
      buttonStateS1 = readingS1;
      if (buttonStateS1 == LOW) {
        elapsedCentis = 0;
        isRunning = !isRunning;
      }
    }
  }
  lastReadingS1 = readingS1;

  // S2: Pause / Resume
  int readingS2 = digitalRead(BUTTON_S2);
  if (readingS2 != lastReadingS2) lastDebounceTimeS2 = now;
  if ((now - lastDebounceTimeS2) > DEBOUNCE_MS) {
    if (readingS2 != buttonStateS2) {
      buttonStateS2 = readingS2;
      if (buttonStateS2 == LOW) {
        isRunning = !isRunning;
      }
    }
  }
  lastReadingS2 = readingS2;

  // S3: Cycle modes (Timer -> Word 1 -> Word 2 ... -> Timer)
  int readingS3 = digitalRead(BUTTON_S3);
  if (readingS3 != lastReadingS3) lastDebounceTimeS3 = now;
  if ((now - lastDebounceTimeS3) > DEBOUNCE_MS) {
    if (readingS3 != buttonStateS3) {
      buttonStateS3 = readingS3;
      if (buttonStateS3 == LOW) {
        displayMode = (displayMode + 1) % (NUM_WORDS + 1);
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
  // Clear outputs before latching new data to avoid ghosting
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

void renderStopwatch() {
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

void renderWord(const byte segs[4]) {
  for (byte i = 0; i < 4; i++) {
    sendFrame(segs[i], DIGIT_SELECT[i]);
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
    renderStopwatch();
  } else {
    renderWord(WORDS[displayMode - 1].segs);
  }
}