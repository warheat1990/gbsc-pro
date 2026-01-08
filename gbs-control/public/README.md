# GBS-Control Pro Web Interface

Web UI for GBSC-Pro. Part of the [GBSC-Pro custom firmware](../../README.md) by Brisma.

## Features

- 36 named profile slots (A-Z, 0-9)
- Slots save current filters state
- Slots filter state can be toggled between local/global
- Backup / Restore system
- Option to enable / disable developer options (hidden by default)
- Integrated WiFi management in system menu
- **Pro Features**:
  - Input source selection (RGBs, RGsB, VGA, YPbPr, S-Video, Composite)
  - Composite/S-Video options (video format, 2X line multiplication, smooth scaling)

## Production Build

### Setup
```bash
npm install
```

### Building for Production
1. Make your changes to the source files
2. Run `npm run build` to generate the necessary `webui_html.h` file
3. Build the ESP8266 firmware: `cd .. && pio run`

**Important**: Before every push, run `npm run build` to ensure bin files are updated to the latest version.

---

## Development Environment

For local development and testing without the physical device:

### Install Dependencies
```bash
npm install
```

### Start Development Server
```bash
npm run dev
```

This will start:
- **HTTP Server** on `http://localhost:8080` - serves static files and mocked REST APIs
- **WebSocket Server** on `ws://localhost:81` - simulates the GBS-Control Pro device

Open `http://localhost:8080` in your browser to test the interface.

### How the Development Server Works

The development server (`dev-server.js`) provides:

#### 1. REST API Mocks
- `/slot/save` - preset saving
- `/slot/set` - slot setting
- `/wifi/status` - WiFi status
- `/bin/slots.bin` - binary slot data
- `/pro` - Pro features (input source selection, video format, 2X, smooth scaling)
- Other necessary endpoints

#### 2. WebSocket Simulation
The WebSocket server periodically sends status messages in the format:
```
#[presetID][slotID][optionByte0][optionByte1][optionByte2]
```

For example: `#1A\x01\x02\x00` means:
- Preset ID: 1
- Slot: A
- Option Byte 0: 0x01 (adcAutoGain active)
- Option Byte 1: 0x02 (frameTimeLock active)
- Option Byte 2: 0x00

Pro status messages: `$[inputType][format][2x][smooth][sharpness]` where:
- inputType is 1-6:
  - 1: RGBs
  - 2: RGsB
  - 3: VGA
  - 4: YPbPr
  - 5: S-Video
  - 6: Composite
- format is 0-9, A, B (representing formats 0-11):
  - 0: Auto
  - 1: PAL
  - 2: NTSC-M
  - 3: PAL-60
  - 4: NTSC443
  - 5: NTSC-J
  - 6: PAL-N w/ p
  - 7: PAL-M w/o p
  - 8: PAL-M
  - 9: PAL Cmb -N
  - A: PAL Cmb -N w/ p
  - B: SECAM
- 2x is 0 or 1 (0=off, 1=on)
- smooth is 0 or 1 (0=off, 1=on, only works when 2x=1)
- sharpness is 0 or 1 (0=Normal, 1=Medium, when active Peaking is locked/disabled)

#### 3. Static Files
Serves all files from the `src/` folder:
- `index.html.tpl`
- `index.js`
- `style.css`
- Various assets

### TypeScript Development

To develop with TypeScript files:

#### Terminal 1 - TypeScript Compilation
```bash
npm start
# Or: tsc --watch ./src/index.ts
```

#### Terminal 2 - Development Server
```bash
npm run dev
```

This setup allows you to:
1. Edit `src/index.ts`
2. TypeScript automatically compiles to `src/index.js`
3. Reload the browser to see changes

### Development Notes

- The development server **does not modify** the webapp code
- All mocks are contained in `dev-server.js`
- For production, use the normal build: `npm run build`
- The WebSocket server simulates data from the real GBS-Control Pro device
- The `/pro` endpoint supports input source selection and Composite/S-Video options (format, 2X, smooth)
