# PMS5003 on Raspberry Pi (WiringPi)

Reads PMS5003 frames via the Pi’s primary UART using WiringPi’s `wiringSerial`. The sensor streams a 32‑byte frame roughly once per second; no host commands are required.

## Prerequisites
- Enable UART and disable the serial login shell:
  - `sudo raspi-config` → Interface Options → Serial
  - “Login shell over serial?” → No
  - “Serial port hardware enabled?” → Yes
- WiringPi installed (self‑compiled if using an early Mk1 Pi).
- Ensure `/dev/serial0` points to the active UART you intend to use.

## Wiring
- PMS5003 `VCC` → Pi `5V` (physical pin 2 or 4)
- PMS5003 `GND` → Pi `GND` (physical pin 6)
- PMS5003 `TXD` → Pi `RXD` (BCM 15, wPi 16, physical pin 10)
- Optional PMS5003 `RXD` → Pi `TXD` (BCM 14, wPi 15, physical pin 8) — only needed for commands.
- PMS5003 `SET` (enable):
  - Tie to `3.3V` to keep sensor running, or
  - Drive via `BCM 4` (wPi 7, physical pin 7) through a level‑safe transistor. This code sets wPi `7` HIGH.

## Build
```bash
gcc -O2 -o pms5003 pms5003.rpi.c -lwiringPi