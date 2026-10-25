# SLF-EWN — Subterranean Landslide & Flood Early-Warning Network Dashboard

A native desktop console for monitoring a distributed subsurface sensor network. Buried nodes report soil moisture and 3-axis tilt over a USB-UART link from a surface tower receiver. The dashboard classifies each node as **SAFE**, **WARNING**, or **CRITICAL**, journals alerts, stores telemetry history locally, and supports both live operation and offline simulation.

---

## Table of Contents

- [Features](#features)
- [System Requirements](#system-requirements)
- [Build Instructions](#build-instructions)
  - [CMake](#cmake)
  - [qmake](#qmake)
- [First Run & Authentication](#first-run--authentication)
- [Using the Console](#using-the-console)
- [Serial Protocol](#serial-protocol)
  - [Binary Frame (12 bytes)](#binary-frame-12-bytes)
  - [ASCII Lines](#ascii-lines)
- [Local Data Store](#local-data-store)
- [Settings Reference](#settings-reference)
  - [THRESHOLDS](#thresholds)
  - [NODES](#nodes)
  - [LINK](#link)
  - [ACCESS](#access)
- [Project Layout](#project-layout)
- [Simulation Mode](#simulation-mode)
- [Troubleshooting](#troubleshooting)
- [License](#license)

---

## Features

- **Live Node Cards** — Real-time moisture, tilt (X/Y/Z), sparkline trends, and color-coded status for every connected sensor node.
- **Alert Log** — Centralized journal of warning, critical, and hardware-interrupt events with acknowledge, clear, and CSV export actions.
- **Zone Map** — Geographic placement of nodes across site zones: North Slope, South Berm, East Face, West Cut, and Access Road.
- **Per-Node Detail** — 24-hour, 7-day, and 30-day history charts with zoom and pan.
- **Dual Serial Protocol** — Auto-detects both compact binary frames and human-readable ASCII telemetry on the same port.
- **Auto-Connect** — Scans available USB-UART ports on launch, preferring CH340, CP210x, and FTDI bridges; connects automatically at 115200 8N1.
- **Simulation Mode** — Built-in packet generator for demo and testing without hardware; explicitly isolated from live data.
- **Local SQLite Persistence** — All telemetry, alerts, node labels, and operator accounts stored in a local database.
- **Role-Based Access Control** — Admin sessions with full configuration rights; guest sessions with read-only live view.
- **RX Diagnostics** — Real-time byte counters, frame decode stats, and dropped-byte analysis in Settings → LINK.

---

## System Requirements

- **Qt 6** with the following modules:
  - Core, Gui, Quick, QuickControls2, Qml, SerialPort, QuickDialogs2, Sql
- **C++17** compiler (MSVC, GCC, or Clang)
- **CMake 3.16+** or **qmake**

> **Windows Note:** Install [Qt 6](https://www.qt.io/download-qt-installer) with the **Serial Port** and **SQL (SQLite)** modules selected.

---

## Build Instructions

### CMake

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=<path-to-Qt6>
cmake --build build --config Release
```

The binary is named `SLF-EWN` (`SLF-EWN.exe` on Windows).

### qmake

```bash
qmake SlfEwnDashboard.pro
make        # Linux/macOS
nmake       # MSVC on Windows
mingw32-make # MinGW on Windows
```

---

## First Run & Authentication

The login screen is the session gate. The following default accounts are pre-seeded on first launch:

| Account | Password | Role |
|---------|----------|------|
| `admin` | `admin` | Full control (thresholds, serial config, acknowledge alerts, user management) |
| `guest` | `guest` | Live view only (read-only dashboard and map) |

Click **Continue as Guest** to sign in without entering a password.

> **Security Notice:** Change the default admin password immediately after first login. Settings → **ACCESS** will display a warning banner while the default password is still in use.

**Brute-force protection:** Five consecutive failed login attempts lock the form for 30 seconds.

---

## Using the Console

1. **Sign in** with admin or guest credentials.
2. **Connect the tower receiver** via USB. The dashboard auto-opens the preferred USB-UART port. The top status bar should display the port name (e.g., `COM5` or `/dev/ttyUSB0`), not `SIMULATION`.
3. **Select a port manually** from the serial bar in the sidebar if auto-connect does not match the receiver, or open Settings → **LINK** to pick the port and click **Connect**.
4. **Monitor node cards** on the dashboard grid. Each card shows live moisture, tilt magnitude, a mini sparkline, and a status badge.
5. **Click a node card** to open the **Detail Panel**, which provides:
   - Acknowledge active alerts for that node
   - Edit the zone label
   - View history charts (24 h / 7 d / 30 d)
6. **Open the Alert Log** (sidebar or top bar) to review all warning / critical / emergency events. Admins can acknowledge individual alerts or export the full log to CSV.
7. **Open the Map View** to see nodes positioned across the five site zones.
8. **Open Settings** (admin only for most tabs) to adjust thresholds, rename nodes, configure the serial link, or manage user accounts.

---

## Serial Protocol

Link parameters: **115200 baud, 8 data bits, no parity, 1 stop bit, no flow control.**

DTR and RTS are asserted on open so that USB-UART bridges (including Arduino-style boards) begin transmitting immediately.

The parser accepts two frame layouts on the same port simultaneously.

### Binary Frame (12 bytes, big-endian)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 2 bytes | `u16` | **Device ID** — unique node identifier |
| 2 | 2 bytes | `u16` | **Moisture raw** — `0…10000` maps to `0.00…100.00 %` |
| 4 | 2 bytes | `i16` | **Tilt X** — milli-g |
| 6 | 2 bytes | `i16` | **Tilt Y** — milli-g |
| 8 | 2 bytes | `i16` | **Tilt Z** — milli-g |
| 10 | 1 byte | `u8` | **Hardware status** — bit 7 (`0x80`) = emergency interrupt |
| 11 | 1 byte | `u8` | **CRC-8/SMBUS** — polynomial `0x07`, init `0x00`, over bytes 0–10 |

**Framing strategy:** The parser scans the incoming byte stream for valid CRC alignment. Bytes that fail CRC are discarded one at a time until a valid 12-byte frame is reconstructed. This provides automatic resynchronization after noise or cable disconnections.

### ASCII Lines

Newline-terminated (`\n` or `\r\n`). Fields may be separated by commas, semicolons, or whitespace.

**Example 1 — comma separated:**
```
A104,42.5,1.2,-0.4,0.8
```

**Example 2 — hex ID with spaces:**
```
0xB201 3850 120 -40 980 0
```

**Parsing rules:**
- Field 1: Device ID (hex, optional `0x` prefix)
- Field 2: Moisture (`≤ 100` treated as percent; otherwise treated as raw)
- Fields 3–5: Tilt X / Y / Z (degrees if values look like angles; otherwise milli-g)
- Field 6 (optional): Hardware status byte
- Lines starting with `#` are ignored as comments

---

## Local Data Store

All persistent data is kept in a single SQLite file:

```
<AppData>/SLF-EWN/SLF-EWN/slfewn.db
```

- **Windows:** `%APPDATA%\SLF-EWN\SLF-EWN\slfewn.db`
- **Linux:** `~/.local/share/SLF-EWN/SLF-EWN/slfewn.db`
- **macOS:** `~/Library/Application Support/SLF-EWN/SLF-EWN/slfewn.db`

**Buffered writes:** Telemetry samples are held in a memory queue and flushed to disk every 4 seconds, or immediately when the queue reaches its capacity limit. This minimizes disk I/O while preventing data loss on unexpected shutdown.

**Security:** Operator passwords are stored as **PBKDF2-HMAC-SHA256** hashes with per-user salts. Plaintext passwords are never written to disk.

---

## Settings Reference

Settings are organized into four tabs. Most tabs require an **admin** session.

### THRESHOLDS

Define the moisture and tilt levels that trigger status changes.

| Parameter | Default | Description |
|-----------|---------|-------------|
| Moisture Warning | 38 % | Elevated soil saturation |
| Moisture Critical | 58 % | Flood-level saturation |
| Tilt Warning | 8° | Noticeable ground shift |
| Tilt Critical | 16° | Severe structural deformation |

> The hardware emergency flag (`0x80` in the status byte) always raises a **CRITICAL** alert regardless of these thresholds.

### NODES

Assign human-readable zone labels to Device IDs. Labels appear on the dashboard cards and the Map View.

### LINK

Serial port configuration and diagnostics.

| Control | Description |
|---------|-------------|
| Port | Dropdown of detected COM/tty ports |
| Baud | Fixed at 115200 (editable for debug) |
| Connect / Disconnect | Manual port control |
| Simulation checkbox | Enables synthetic packet injection (pauses live port) |
| RX Bytes | Total bytes received since connect |
| Frames Decoded | Valid packets parsed |
| Dropped Bytes | Bytes discarded due to CRC mismatch or framing errors |
| Diagnosis | Auto-generated hint if the feed is silent or mis-framed |
| Store Path | SQLite database file location |

**Important:** Enabling **Simulation** explicitly pauses the live serial port so that synthetic packets cannot mix with real telemetry. Unchecking it reconnects the receiver automatically.

### ACCESS

User management and security (admin only).

- Change admin password
- Create / delete operator accounts
- View audit trail (login/logout events, setting changes)
- Default-password warning banner

---

## Project Layout

```
SLF-EWN/
├── src/
│   ├── core/          # Packet parser, CRC-8, shared types, constants
│   ├── model/         # Serial I/O, SQLite backend, simulation engine, list models
│   ├── controller/    # App session, authentication, node state machine
│   └── main.cpp
├── qml/
│   ├── pages/         # Login, Dashboard, Alerts, Map, Settings
│   └── components/    # Node cards, sparklines, charts, chrome, dialogs
├── CMakeLists.txt
├── SlfEwnDashboard.pro
└── resources/         # Icons, fonts, default config
```

**Key QML context properties:**
- `App` → `Slf::AppController` — global app state, navigation, theme
- `Auth` → `Slf::AuthController` — login/logout, session role, audit

---

## Simulation Mode

Simulation mode is intended for demonstrations, UI testing, and development without physical hardware.

**How to enable:**
1. Sign in as admin.
2. Open **Settings → LINK**.
3. Check the **Simulation** checkbox.
4. The top bar will switch from the port name to `SIMULATION`.
5. The dashboard will begin receiving synthetic packets from an internal timer.

**Behavior:**
- The live serial port is **paused** (not closed) to prevent data mixing.
- Synthetic packets follow the binary 12-byte frame format with randomized moisture and tilt values.
- Emergency interrupts are injected periodically to test alarm visualization.
- All simulated data is written to the local SQLite store exactly like real telemetry.

**To return to live mode:** Uncheck the Simulation checkbox. The port reconnects automatically.

---

## Troubleshooting

| Symptom | Likely Cause | Resolution |
|---------|--------------|------------|
| Top bar shows `SIMULATION` | Simulation mode is enabled | Uncheck in Settings → LINK |
| No data after connecting | Wrong port selected | Verify COM/tty in device manager; try manual selection |
| RX Bytes increasing, Frames Decoded = 0 | Baud mismatch or framing error | Confirm tower is set to 115200 8N1; check LINK diagnosis label |
| Dropped Bytes climbing | Noise on cable or incorrect CRC | Verify CRC polynomial (`0x07`, init `0x00`) in transmitter firmware |
| Admin settings grayed out | Logged in as guest | Sign out and log in as admin |
| Login form locked | Five failed attempts | Wait 30 seconds; retry with correct credentials |
| SQLite write errors | Missing write permissions on AppData folder | Run with standard user privileges; check antivirus quarantine |

---

## License

Private / academic project use unless otherwise specified.

---

*SLF-EWN Dashboard v1.0 — Qt 6 / QML / C++17*
