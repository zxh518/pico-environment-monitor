# Pico W Environmental Monitor

A temperature and humidity monitor built on a Raspberry Pi Pico W, written in C. Reads from a DHT22 sensor, displays live readings on a 16x2 LCD, and triggers an LED alert when thresholds are exceeded.

## What it does

- Reads temperature (°C) and humidity (%) from a DHT22 sensor
- Displays readings on a 16x2 I2C LCD screen in real time
- Lights up a red LED when temperature or humidity goes above a set threshold

## Hardware

- Raspberry Pi Pico W
- DHT22 temperature and humidity sensor
- 16x2 LCD with I2C adapter module
- LEDs and resistors for alerts
- Breadboard and jumper wires

## What I learned building this

This was my first embedded project, and most of the learning came from debugging rather than the initial build.

**The DHT22 timing problem.** The sensor kept returning 0°C and 0% humidity no matter what I tried. After a lot of testing (including buying a second sensor and a multimeter thinking the hardware was faulty), I found the real cause which was that the Pico's USB stack runs a background task every millisecond to keep the connection with the laptop alive. The DHT22 sends its data as a single timed pulse on one pin, and the USB interrupt was firing right in the middle of that read, causing my code to miss the response every time.

It failed every single time rather than occasionally because the original code waited exactly 20ms before listening, which lined up with the USB tick cycle, so the collision landed in the same spot on every attempt.

The fix was to disable interrupts for the few milliseconds while the sensor responds, then re-enable them once the data is captured. Shortening the start pulse from 20ms to 2ms also moved the timing away from that collision.

**The LCD contrast issue.** The LCD powered on but showed no text. So, I adjusted the potentiometer on the back of the I2C adapter with a screwdriver which fixed it immediately.

## How to build and flash

1. Install the Raspberry Pi Pico SDK and VS Code with the Pico extension
2. Clone this repo
3. Open in VS Code, the Pico extension handles the CMake toolchain setup
4. Build and flash to a Pico W via USB (hold BOOTSEL while plugging in)

## Wiring

- DHT22 data pin → GP2
- LCD SDA → GP4, SCL → GP5 (I2C0)
- Normal LED → GP15 through a 330Ω resistor
- Alert LED → GP14 through a 330Ω resistor
