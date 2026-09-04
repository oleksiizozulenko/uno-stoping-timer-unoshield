# Arduino Uno Multi-Function Shield Stopwatch & Display

A high-precision stopwatch timer and custom 4-letter word display firmware built for the Arduino Uno using the standard Multi-Function Shield (MFS) with dual 74HC595 shift registers.

## Features

- **Stopwatch Timer**: 0.01s resolution timer (00.00 to 99.99s).
- **4-Letter Word Modes**: Displays custom words (`HELL`, `HELP`, `COOL`, `OPEN`, `STOP`, `FIRE`, `GOOD`).
- **Debounced Button Controls**: Reliable input handling for S1, S2, and S3 buttons.
- **Ghost-free Display Multiplexing**: Output pre-blanking between scan passes to eliminate display ghosting.

---

## Hardware Configuration & Pinout

The Multi-Function Shield drives the 4-digit 7-segment display via two daisy-chained 74HC595 shift registers (active-LOW segment mapping, active-HIGH digit selection).

| Pin Name | Arduino Pin | Description |
| :--- | :---: | :--- |
| **LATCH_PIN** | `D4` | 74HC595 RCLK (Latch) |
| **CLK_PIN** | `D7` | 74HC595 SRCLK (Shift Clock) |
| **DATA_PIN** | `D8` | 74HC595 SER (Serial Data) |
| **BUTTON_S1** | `A1` | Button S1 (Active LOW) |
| **BUTTON_S2** | `A2` | Button S2 (Active LOW) |
| **BUTTON_S3** | `A3` | Button S3 (Active LOW) |

---

## Button Controls

- **Button S1 (`A1`)**: **Reset & Start/Stop**
  - Resets the elapsed time to `00.00` and toggles between start and stop.
- **Button S2 (`A2`)**: **Pause / Resume**
  - Toggles pause and resume without clearing the elapsed time.
- **Button S3 (`A3`)**: **Display Mode Switcher**
  - Cycles between Stopwatch Digits mode (`XX.YY`) and 4-letter word modes (`HELL` -> `HELP` -> `COOL` -> `OPEN` -> `STOP` -> `FIRE` -> `GOOD` -> Stopwatch).

---

## Hardware Troubleshooting & Notes

### Issue: Digit 5 displays as 9, and 6 displays as 8

- **Root Cause**: The bottom solder pins of the Multi-Function Shield sit directly above the metal shell of the Arduino Uno USB Type-B port. On some shields, long component pins touch the USB housing, causing a direct short to **GND**. Because the display uses active-LOW logic, grounding **Segment B** keeps it permanently ON, transforming `5` into `9` and `6` into `8`.
- **Fix**: Place a strip of electrical tape or Kapton tape over the top of the metal USB connector box on the Arduino Uno board before attaching the shield.

---

## Building and Flashing

Built with PlatformIO:

```bash
# Build firmware
pio run

# Upload to Arduino Uno
pio run --target upload
```
