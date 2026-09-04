# Arduino Uno Multi-Function Shield Dual-Mode Stopwatch & Slot-Machine Game

A dual-mode firmware built for the Arduino Uno and Multi-Function Shield (MFS) featuring a **Digital Stopwatch** mode and an interactive **Independent Spinning Letters (Slot Machine) Mode**.

## Features & Modes

- **Digital Mode**:
  - Idle / Reset Display: `00.00`
  - Counts `00.00` to `99.99` in 0.01s (centiseconds).
- **Letters Mode (Slot Machine Mini-Game)**:
  - Idle / Reset Display: `_ _ _ _` (4 underscores).
  - When running: Each of the 4 digit positions spins through letters **A to Z** independently at high speed!
  - **Mini-Game Goal**: Try to press **S2** at the exact right moment to freeze the spinning letters on a real 4-letter word!
- **Button Controls**:
  - **Button S1 (`A1`)**: **Reset** — Resets to default mode state (`00.00` or `_ _ _ _`) and stops.
  - **Button S2 (`A2`)**: **Start / Stop / Resume** — Starts running from idle state, or pauses/unfreezes the display.
  - **Button S3 (`A3`)**: **Mode Switcher** — Toggles between Digital Mode (`00.00`) and Letters Mode (`_ _ _ _`).

---

## Hardware Configuration & Pinout

| Pin Name | Arduino Pin | Description |
| :--- | :---: | :--- |
| **LATCH_PIN** | `D4` | 74HC595 RCLK (Latch) |
| **CLK_PIN** | `D7` | 74HC595 SRCLK (Shift Clock) |
| **DATA_PIN** | `D8` | 74HC595 SER (Serial Data) |
| **BUTTON_S1** | `A1` | Reset to Idle State |
| **BUTTON_S2** | `A2` | Start / Stop / Resume |
| **BUTTON_S3** | `A3` | Mode Switcher (Digital <-> Letters) |

---

## Hardware Troubleshooting Note

If digits `5` and `6` render as `9` and `8`, check for a physical short circuit between the bottom solder pins of the shield and the top metal housing of the Arduino Uno USB Type-B port. Place electrical tape over the top of the metal USB box to insulate it.

---

## Building and Flashing

```bash
# Build firmware
pio run

# Upload to Arduino Uno
pio run --target upload
```
