# ZigbeeWeather — ESP32-C6 SHT20 Weather Node

Quick guide to wiring, building, enabling the ESP-IDF Zigbee stack, and pairing with a SmartThings hub.

Wiring
- SHT20 (SHT2x): VCC -> 3.3V, GND -> GND, SDA -> GPIO21, SCL -> GPIO22
- Optional external Zigbee radio (UART fallback): TX -> GPIO17, RX -> GPIO16 (baud 9600)

Build & run (PlatformIO)
- Clean then build the default environment (full path shown if `pio` not on PATH):
```powershell
C:\Users\gomond\.platformio\penv\Scripts\platformio.exe run -t clean -e esp32-c6-devkitc-1
C:\Users\gomond\.platformio\penv\Scripts\platformio.exe run -e esp32-c6-devkitc-1
```
- Serial monitor (to view logs):
```powershell
C:\Users\gomond\.platformio\penv\Scripts\platformio.exe device monitor -e esp32-c6-devkitc-1
```

Enable internal Zigbee stack (ESP-IDF)
- Recommended: enable Zigbee via `menuconfig` so the project builds the esp Zigbee stack components.
- Launch interactive menuconfig in a terminal that supports curses (Git Bash / WSL / Windows Terminal):
```powershell
C:\Users\gomond\.platformio\penv\Scripts\platformio.exe run -t menuconfig -e esp32-c6-devkitc-1
```
- In `menuconfig` navigate to: `Component config` → `Zigbee` (or `ZBOSS`) and enable the Zigbee/ZBOSS stack.
- Save and exit, then run the clean + build commands above.
- If your ESP-IDF package lacks Zigbee support, the code will compile and use the UART fallback transport instead.

SmartThings pairing (permit-join)
- Put your SmartThings hub in Zigbee pairing mode from the SmartThings app: Add Device → Scan for devices → Zigbee (follow on-screen instructions to open permit-join).
- Once the hub is accepting joins, power on the device (it will attempt to join if the internal Zigbee stack is enabled).
- Logs to watch for:
  - `zigbee_app_init` or `Using UART fallback while internal Zigbee integration remains unimplemented` — indicates whether internal stack or UART fallback is used.
  - `Reporting via UART Zigbee transport: T=... H=...` — shows sensor readings forwarded via UART/XBee frames.

Notes and next steps
- The project currently includes a `zigbee_app` component with a UART fallback. When the Espressif Zigbee stack is enabled, the `zigbee_app` component is the place to implement ZCL endpoint registration (Temperature 0x0402, Relative Humidity 0x0405) and attribute reporting.
- To change I2C or UART pins, edit `src/main.c` (I2C: `I2C_SDA_PIN`, `I2C_SCL_PIN`; UART: `ZIGBEE_TX_PIN`, `ZIGBEE_RX_PIN`).
- If you want, I can implement the full ZCL endpoint and reporting next — enable Zigbee in `menuconfig` and tell me to proceed.

Files of interest
- `src/main.c` — app main loop and I2C/SHT20 integration
- `components/sht20` — SHT20 driver (with CRC)
- `components/zigbee` — UART/XBee transport
- `components/zigbee_app` — internal Zigbee app (skeleton + UART fallback)
# ZigbeeWeather — SHT20 Sensor Example

This project demonstrates reading an SHT20 (SHT2x) temperature/humidity sensor from an ESP32-C6 using ESP-IDF via PlatformIO.

Wiring (typical SHT20 module):
- VCC -> 3.3V
- GND -> GND
- SDA -> GPIO21 (change in `src/main.c` if needed)
- SCL -> GPIO22 (change in `src/main.c` if needed)

Build and run (PlatformIO):
```bash
pio run -e esp32-c6-devkitc-1
pio run -t upload -e esp32-c6-devkitc-1
pio device monitor -e esp32-c6-devkitc-1
```

Notes:
- The driver implementation is in `lib/sht20/src/sht20.c` and the public header is `include/sht20.h`.
- If your board uses different I2C pins, edit `I2C_SDA_PIN` and `I2C_SCL_PIN` in `src/main.c`.
- This example uses the no-hold master commands and waits ~100 ms for conversions; adjust timings if necessary.
