<!-- Purpose: concise, actionable guidance for AI coding agents working on this PlatformIO project -->
# Copilot instructions — Zigbee Weather (PlatformIO)

Purpose
- Help AI coding agents be immediately productive in this repository: a PlatformIO firmware project targeting ESP32-C6 (ESP-IDF via PlatformIO).

Quick repo snapshot
- **Platform/Env**: see [platformio.ini](platformio.ini) — environment `esp32-c6-devkitc-1` using `platform = espressif32` and `framework = espidf`.
- **Source layout**: `src/` (firmware sources — currently empty), `include/` (public headers), `lib/` (local libraries), `test/` (PlatformIO unit tests), `.pio/` (build artifacts).

Big picture / architecture to assume
- This is a single-board firmware project (one PlatformIO environment). Code lives in `src/` and is compiled with ESP-IDF via PlatformIO.
- Local libraries belong under `lib/<name>/src` with optional `library.json` for custom build options (PlatformIO LDF is used).
- No application code is present yet; when adding features, place platform-agnostic headers in `include/` and implementation in `src/` or `lib/` as appropriate.

Build / test / debug workflows (explicit)
- Build the default env: `pio run` (PlatformIO reads `platformio.ini`).
- Build a specific env: `pio run -e esp32-c6-devkitc-1`.
- Upload to a connected board: `pio run -t upload -e esp32-c6-devkitc-1`.
- Open device serial monitor: `pio device monitor -e esp32-c6-devkitc-1`.
- Run unit tests (if tests added to `test/`): `pio test -e esp32-c6-devkitc-1`.

Project-specific conventions (discoverable)
- Place firmware entrypoint in `src/` as `main.c` / `main.cpp` or an ESP-IDF-style component; PlatformIO will build whatever is in `src/`.
- Use header files in `include/` for interfaces shared across modules (standard C/C++ include rules apply).
- Local libs under `lib/` should follow PlatformIO layout (see `lib/README` in repo).
- Prefer PlatformIO's Library Dependency Finder (LDF) instead of hard-coding include paths; add `library.json` only when custom build flags are required.

Integration points & external dependencies
- Hardware: project targets an ESP32-C6 dev board — any Zigbee radio or sensor integrations will be through additional libraries or drivers placed under `lib/` or added as submodules/dependencies.
- Platform/Framework: `platform = espressif32`, `framework = espidf` — use ESP-IDF APIs and build idioms where applicable.

What to look for when making changes
- If adding hardware drivers or Zigbee logic, put drivers in `lib/<driver>/src` and headers in `include/`.
- Add unit tests under `test/` following PlatformIO Test Runner conventions.
- Update `platformio.ini` only when adding additional build environments, board targets, or custom build flags.

Examples (concrete)
- To add a sensor driver: create `lib/sensor_x/src/sensor_x.c` and `lib/sensor_x/src/sensor_x.h`, then `#include <sensor_x.h>` from `src/main.c`.
- To run a quick build locally (Windows PowerShell):
```
pio run
pio device monitor -e esp32-c6-devkitc-1
```

When merging or editing this file
- If an existing `.github/copilot-instructions.md` exists, preserve any repository-specific instructions and merge the build/test commands and env names from `platformio.ini`.

Limitations / assumptions
- Repo currently has no firmware source in `src/`; do not assume internal code structure beyond PlatformIO defaults.
- No CI config discovered; do not assume CI scripts or matrix builds exist.

If anything is unclear or you want more detail (for example, expected Zigbee stack, target hardware UART pins, or test scenarios), ask the maintainer for hardware schematics or intended runtime behavior.

---
Please review and tell me which sections need more detail or examples for the Zigbee/weather integration.
