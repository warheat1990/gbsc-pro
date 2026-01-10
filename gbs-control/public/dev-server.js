// dev-server.js - Development server for testing GBS-Control Pro web interface
// Usage: node dev-server.js
// Then open http://localhost:8080 in your browser

const http = require('http');
const fs = require('fs');
const path = require('path');
const { WebSocketServer } = require('ws');

const HTTP_PORT = 8080;
const HTTP_PORT_ALT = 80; // Alternative port for /sc and /uc
const WS_PORT = 81;

// Mock slot data - must match SlotMeta struct (128 bytes, 36 slots)
const createMockSlotsData = () => {
  // SlotMeta structure (128 bytes total):
  // --- ORIGINAL GBS-CONTROL (32 bytes) ---
  // char name[25];              // offset 0
  // uint8_t presetID;           // offset 25
  // uint8_t scanlines;          // offset 26
  // uint8_t scanlinesStrength;  // offset 27
  // uint8_t slot;               // offset 28
  // uint8_t wantVdsLineFilter;  // offset 29
  // uint8_t wantStepResponse;   // offset 30
  // uint8_t wantPeaking;        // offset 31
  // --- PRO: GBS Processing options (7 bytes) ---
  // uint8_t wantFullHeight;     // offset 32
  // uint8_t deintMode;          // offset 33
  // uint8_t enableFrameTimeLock;// offset 34
  // uint8_t frameTimeLockMethod;// offset 35
  // uint8_t PalForce60;         // offset 36
  // uint8_t adcGain;            // offset 37
  // uint8_t wantSharpness;      // offset 38
  // --- PRO: GBS Color balance (3 bytes) ---
  // uint8_t gbsColorR;          // offset 39
  // uint8_t gbsColorG;          // offset 40
  // uint8_t gbsColorB;          // offset 41
  // --- PRO: ADV7280 settings (6 bytes) ---
  // uint8_t advSmooth;          // offset 42
  // uint8_t advI2P;             // offset 43
  // uint8_t advBrightness;      // offset 44
  // uint8_t advContrast;        // offset 45
  // uint8_t advSaturation;      // offset 46
  // uint8_t advACE;             // offset 47
  // --- PRO: ADV7280 ACE Parameters (5 bytes) ---
  // uint8_t advACELumaGain;     // offset 48
  // uint8_t advACEChromaGain;   // offset 49
  // uint8_t advACEChromaMax;    // offset 50
  // uint8_t advACEGammaGain;    // offset 51
  // uint8_t advACEResponseSpeed;// offset 52
  // --- PRO: ADV7280 Video Filter Parameters (6 bytes) ---
  // uint8_t advFilterYShaping;  // offset 53
  // uint8_t advFilterCShaping;  // offset 54
  // uint8_t advFilterWYShaping; // offset 55
  // uint8_t advFilterWYOverride;// offset 56
  // uint8_t advFilterCombNTSC;  // offset 57
  // uint8_t advFilterCombPAL;   // offset 58
  // --- HDMI Limited Range ---
  // uint8_t hdmiLimitedRange;   // offset 59
  // --- Reserved (68 bytes) ---
  // uint8_t reserved[68];       // offset 60-127
  const slotSize = 128;  // Must match SlotMeta struct
  const numSlots = 36;   // SLOTS_TOTAL
  const buffer = Buffer.alloc(slotSize * numSlots);

  for (let i = 0; i < numSlots; i++) {
    const offset = i * slotSize;

    // Slot name (25 bytes, null-terminated or space-padded)
    const name = `Slot ${i}`;
    buffer.write(name, offset, 25, 'ascii');
    // Fill the rest with spaces
    for (let j = name.length; j < 25; j++) {
      buffer.writeUInt8(0x20, offset + j); // space
    }

    // --- Original GBS-Control fields ---
    buffer.writeUInt8(0x01, offset + 25); // presetID
    buffer.writeUInt8(0x00, offset + 26); // scanlines (0 = off)
    buffer.writeUInt8(0x30, offset + 27); // scanlinesStrength
    buffer.writeUInt8(i, offset + 28);    // slot index
    buffer.writeUInt8(0x01, offset + 29); // wantVdsLineFilter
    buffer.writeUInt8(0x00, offset + 30); // wantStepResponse
    buffer.writeUInt8(0x00, offset + 31); // wantPeaking

    // --- PRO: GBS Processing options ---
    buffer.writeUInt8(0x01, offset + 32); // wantFullHeight (default 1)
    buffer.writeUInt8(0x00, offset + 33); // deintMode (default 0=Adaptive)
    buffer.writeUInt8(0x00, offset + 34); // enableFrameTimeLock (default 0)
    buffer.writeUInt8(0x00, offset + 35); // frameTimeLockMethod (default 0)
    buffer.writeUInt8(0x00, offset + 36); // PalForce60 (default 0)
    buffer.writeUInt8(0x7B, offset + 37); // adcGain (default 0x7B)
    buffer.writeUInt8(0x00, offset + 38); // wantSharpness (default 0)

    // --- PRO: GBS Color balance ---
    buffer.writeUInt8(0x80, offset + 39); // gbsColorR (default 128)
    buffer.writeUInt8(0x80, offset + 40); // gbsColorG (default 128)
    buffer.writeUInt8(0x80, offset + 41); // gbsColorB (default 128)

    // --- PRO: ADV7280 settings ---
    buffer.writeUInt8(0x00, offset + 42); // advSmooth (default 0)
    buffer.writeUInt8(0x00, offset + 43); // advI2P (default 0)
    buffer.writeUInt8(0x80, offset + 44); // advBrightness (default 128)
    buffer.writeUInt8(0x80, offset + 45); // advContrast (default 128)
    buffer.writeUInt8(0x80, offset + 46); // advSaturation (default 128)
    buffer.writeUInt8(0x00, offset + 47); // advACE (default 0)

    // --- PRO: ADV7280 ACE Parameters ---
    buffer.writeUInt8(13, offset + 48);   // advACELumaGain (default 13)
    buffer.writeUInt8(8, offset + 49);    // advACEChromaGain (default 8)
    buffer.writeUInt8(8, offset + 50);    // advACEChromaMax (default 8)
    buffer.writeUInt8(8, offset + 51);    // advACEGammaGain (default 8)
    buffer.writeUInt8(15, offset + 52);   // advACEResponseSpeed (default 15)

    // --- PRO: ADV7280 Video Filter Parameters ---
    buffer.writeUInt8(1, offset + 53);    // advFilterYShaping (default 1=AutoNarrow)
    buffer.writeUInt8(0, offset + 54);    // advFilterCShaping (default 0=Auto1.5M)
    buffer.writeUInt8(9, offset + 55);    // advFilterWYShaping (default 9=SVHS-8)
    buffer.writeUInt8(1, offset + 56);    // advFilterWYOverride (default 1=Manual)
    buffer.writeUInt8(0, offset + 57);    // advFilterCombNTSC (default 0=Narrow)
    buffer.writeUInt8(1, offset + 58);    // advFilterCombPAL (default 1=Medium)

    // --- HDMI Limited Range ---
    buffer.writeUInt8(1, offset + 59);    // hdmiLimitedRange (default 1=HD)

    // --- Reserved (68 bytes, offset 60-127) ---
    // Already zeroed by Buffer.alloc
  }

  return buffer;
};

// MIME types
const mimeTypes = {
  '.html': 'text/html',
  '.js': 'application/javascript',
  '.css': 'text/css',
  '.json': 'application/json',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.gif': 'image/gif',
  '.svg': 'image/svg+xml',
  '.ico': 'image/x-icon'
};

// Shared request handler
const handleRequest = (req, res) => {
  const timestamp = new Date().toLocaleTimeString();
  console.log(`\n[${timestamp}] 📥 ${req.method} ${req.url}`);

  // Add CORS headers to allow cross-origin requests
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  // Handle OPTIONS requests (preflight)
  if (req.method === 'OPTIONS') {
    res.writeHead(200);
    res.end();
    return;
  }

  const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);

  // Mock API endpoints
  if (url.pathname.startsWith('/slot/save')) {
    const params = Object.fromEntries(url.searchParams);
    console.log(`  ├─ 💾 Save Slot: index=${params.index}, name=${params.name}`);
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ status: 'ok' }));
    return;
  }

  if (url.pathname.startsWith('/slot/set')) {
    const params = Object.fromEntries(url.searchParams);
    console.log(`  ├─ 🎯 Set Slot: ${params.slot}`);
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ status: 'ok' }));
    return;
  }

  if (url.pathname.startsWith('/slot/remove')) {
    console.log(`  ├─ 🗑️  Remove Slot: ${url.search}`);
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ status: 'ok' }));
    return;
  }

  if (url.pathname === '/wifi/status') {
    console.log(`  ├─ 📡 WiFi Status Request`);
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({
      mode: 'ap',
      ssid: 'gbscontrol-dev'
    }));
    return;
  }

  if (url.pathname === '/wifi/list') {
    console.log(`  ├─ 📶 WiFi List Request`);
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('80,1,TestNetwork1\n70,0,TestNetwork2\n60,1,TestNetwork3');
    return;
  }

  if (url.pathname === '/wifi/connect' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => {
      body += chunk.toString();
    });
    req.on('end', () => {
      const params = new URLSearchParams(body);
      const ssid = params.get('n');
      const password = params.get('p');
      console.log(`  ├─ 🔗 WiFi Connect: SSID="${ssid}", Password="${password ? '***' : 'none'}"`);
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ status: 'ok' }));
    });
    return;
  }

  if (url.pathname.startsWith('/sc')) {
    const commandFull = url.searchParams.toString().split('&')[0];
    const command = commandFull.split('=')[0]; // Extract just the command letter
    console.log(`  ├─ 🎮 Serial Command: ${command}`);

    if (command === 'f') {
      // Peaking toggle - only works if Sharpness is not active
      if (!currentSharpness) {
        optionByte0 ^= 0x08; // Toggle bit 3 for Peaking
        console.log(`  ├─ 🎮 Peaking toggled to ${(optionByte0 & 0x08) ? 'ON' : 'OFF'}`);
        broadcastStatus();
      } else {
        console.log(`  ├─ 🎮 Peaking cannot be toggled (locked by Sharpness)`);
      }
    } else if (command === 'm') {
      optionByte0 ^= 0x04; // Toggle bit 2 for VDS Line Filter
      console.log(`  ├─ 🎮 Line Filter toggled to ${(optionByte0 & 0x04) ? 'ON' : 'OFF'}`);
      broadcastStatus();
    } else if (command === '7') {
      // Toggle scanlines: requires logic for scanlinesStrength but simple toggle here
      optionByte0 ^= 0x02;
      console.log(`  ├─ 🎮 Scanlines toggled to ${(optionByte0 & 0x02) ? 'ON' : 'OFF'}`);
      broadcastStatus();
    }

    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('OK');
    return;
  }

  if (url.pathname.startsWith('/uc')) {
    const commandFull = url.searchParams.toString().split('&')[0];
    const command = decodeURIComponent(commandFull.split('=')[0]); // Extract and decode command
    console.log(`  ├─ ⚙️  User Command: ${command}`);

    if (command === 'W') {
      // Toggle Sharpness
      currentSharpness = currentSharpness ? 0 : 1;
      console.log(`  ├─ ⚙️  Sharpness toggled to ${currentSharpness ? 'Medium' : 'Normal'}`);

      // When Sharpness is active, Peaking is locked (disabled)
      if (currentSharpness) {
        console.log(`  ├─ ⚙️  Peaking is now locked (disabled by Sharpness)`);
      }
      broadcastStatus();
    }

    if (command === '%') {
      // Cycle HDMI Limited Range: 0=Off, 1=HD, 2=SD, 3=All
      currentHdmiLimitedRange = (currentHdmiLimitedRange + 1) % 4;
      const hdmiLimitedRangeNames = ['OFF', 'HD', 'SD', 'ALL'];
      console.log(`  ├─ ⚙️  HDMI Limited Range: ${hdmiLimitedRangeNames[currentHdmiLimitedRange]}`);
      broadcastStatus();
    }

    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('OK');
    return;
  }

  if (url.pathname === '/bin/slots.bin') {
    const buffer = createMockSlotsData();
    console.log(`  ├─ 📦 Slots Binary: ${buffer.length} bytes`);
    res.writeHead(200, {
      'Content-Type': 'application/octet-stream',
      'Content-Length': buffer.length,
      'Cache-Control': 'no-cache'
    });
    res.end(buffer);
    return;
  }

  if (url.pathname === '/pro' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => {
      body += chunk.toString();
    });
    req.on('end', () => {
      const params = new URLSearchParams(body);
      const i = parseInt(params.get('i'));
      const f = parseInt(params.get('f'));

      // Handle input source selection
      if (!isNaN(i)) {
        const inputNames = ['', 'RGBs', 'RGsB', 'VGA', 'YPbPr', 'S-Video', 'Composite'];

        if (i === 0) {
          console.log(`  ├─ ⚡ Pro: Get current input = ${currentInputType}`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(String(currentInputType));
        } else if (i >= 1 && i <= 6) {
          currentInputType = i;
          console.log(`  ├─ ⚡ Pro: Set input to ${inputNames[i]} (${i})`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle video format selection
      if (!isNaN(f)) {
        const formatNames = ['Auto', 'PAL', 'NTSC-M', 'PAL-60', 'NTSC443', 'NTSC-J', 'PAL-N w/ p', 'PAL-M w/o p', 'PAL-M', 'PAL Cmb -N', 'PAL Cmb -N w/ p', 'SECAM'];

        if (f >= 0 && f <= 11) {
          currentFormat = f;
          console.log(`  ├─ ⚡ Pro: Set format to ${formatNames[f]} (${f})`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle 2X toggle
      const x = parseInt(params.get('x'));
      if (!isNaN(x)) {
        if (x >= 0 && x <= 1) {
          current2X = x;
          console.log(`  ├─ ⚡ Pro: Set 2X to ${x === 1 ? 'ON' : 'OFF'} (${x})`);

          // If 2X is disabled, also disable Smooth (smooth only works with 2X)
          if (!current2X && currentSmooth) {
            currentSmooth = 0;
            console.log(`  ├─ ⚡ Pro: Smooth disabled (requires 2X)`);
          }

          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle Smooth toggle
      const s = parseInt(params.get('s'));
      if (!isNaN(s)) {
        if (s >= 0 && s <= 1) {
          // Force smooth off if 2X is not enabled
          if (!current2X && s === 1) {
            currentSmooth = 0;
            console.log(`  ├─ ⚡ Pro: Smooth not enabled (requires 2X)`);
          } else {
            currentSmooth = s;
            console.log(`  ├─ ⚡ Pro: Set Smooth to ${s === 1 ? 'ON' : 'OFF'} (${s})`);
          }

          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle ACE toggle
      const a = parseInt(params.get('a'));
      if (!isNaN(a)) {
        if (a >= 0 && a <= 1) {
          currentACE = a;
          console.log(`  ├─ ⚡ Pro: Set ACE to ${a === 1 ? 'ON' : 'OFF'} (${a})`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
          broadcastStatus();
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle ACE Luma Gain (al parameter)
      const al = parseInt(params.get('al'));
      if (!isNaN(al)) {
        if (al >= 0 && al <= 31) {
          currentACELumaGain = al;
          console.log(`  ├─ ⚡ Pro: Set ACE Luma Gain to ${al}`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
          broadcastStatus();
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle ACE Chroma Gain (ac parameter)
      const ac = parseInt(params.get('ac'));
      if (!isNaN(ac)) {
        if (ac >= 0 && ac <= 15) {
          currentACEChromaGain = ac;
          console.log(`  ├─ ⚡ Pro: Set ACE Chroma Gain to ${ac}`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
          broadcastStatus();
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle ACE Chroma Max (am parameter)
      const am = parseInt(params.get('am'));
      if (!isNaN(am)) {
        if (am >= 0 && am <= 15) {
          currentACEChromaMax = am;
          console.log(`  ├─ ⚡ Pro: Set ACE Chroma Max to ${am}`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
          broadcastStatus();
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle ACE Gamma Gain (ag parameter)
      const ag = parseInt(params.get('ag'));
      if (!isNaN(ag)) {
        if (ag >= 0 && ag <= 15) {
          currentACEGammaGain = ag;
          console.log(`  ├─ ⚡ Pro: Set ACE Gamma Gain to ${ag}`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
          broadcastStatus();
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle ACE Response Speed (ar parameter)
      const ar = parseInt(params.get('ar'));
      if (!isNaN(ar)) {
        if (ar >= 0 && ar <= 15) {
          currentACEResponseSpeed = ar;
          console.log(`  ├─ ⚡ Pro: Set ACE Response Speed to ${ar}`);
          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
          broadcastStatus();
        } else {
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // Handle ACE Defaults (ad parameter)
      const ad = parseInt(params.get('ad'));
      if (!isNaN(ad) && ad === 1) {
        currentACELumaGain = 13;
        currentACEChromaGain = 8;
        currentACEChromaMax = 8;
        currentACEGammaGain = 8;
        currentACEResponseSpeed = 15;
        console.log(`  ├─ ⚡ Pro: Reset ACE parameters to defaults`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end('true');
        broadcastStatus();
        return;
      }

      // =====================================================================
      // Video Filters API - fy, fc, fo, fb, fd
      // =====================================================================

      // Handle Video Filter Y Shaping (fy parameter)
      const fy = parseInt(params.get('fy'));
      if (!isNaN(fy) && fy >= 0 && fy <= 30) {
        if (currentInputType === 5) {
          currentFilterWY = fy;  // S-Video
          console.log(`  ├─ ⚡ Pro: Set WY Filter to ${fy}`);
        } else {
          currentFilterY = fy;   // Composite
          console.log(`  ├─ ⚡ Pro: Set Y Filter to ${fy}`);
        }
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end('true');
        broadcastStatus();
        return;
      }

      // Handle Video Filter C Shaping (fc parameter)
      const fc = parseInt(params.get('fc'));
      if (!isNaN(fc) && fc >= 0 && fc <= 7) {
        currentFilterC = fc;
        console.log(`  ├─ ⚡ Pro: Set C Filter to ${fc}`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end('true');
        broadcastStatus();
        return;
      }

      // Handle Video Filter Override (fo parameter)
      const fo = parseInt(params.get('fo'));
      if (!isNaN(fo) && (fo === 0 || fo === 1)) {
        currentFilterWYOverride = fo;
        console.log(`  ├─ ⚡ Pro: Set WY Override to ${fo ? 'Manual' : 'Auto'}`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end('true');
        broadcastStatus();
        return;
      }

      // Handle Video Filter Comb (fb parameter)
      const fb = parseInt(params.get('fb'));
      if (!isNaN(fb) && fb >= 0 && fb <= 3) {
        currentFilterComb = fb;
        console.log(`  ├─ ⚡ Pro: Set Comb Filter to ${fb}`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end('true');
        broadcastStatus();
        return;
      }

      // Handle Video Filter Defaults (fd parameter)
      const fd = parseInt(params.get('fd'));
      if (!isNaN(fd) && fd === 1) {
        currentFilterY = 1;
        currentFilterC = 0;
        currentFilterWY = 9;
        currentFilterWYOverride = 1;
        currentFilterComb = 1;
        console.log(`  ├─ ⚡ Pro: Reset Video Filters to defaults`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end('true');
        broadcastStatus();
        return;
      }

      // Handle Sync Stripper toggle (ss parameter)
      const ss = parseInt(params.get('ss'));
      if (!isNaN(ss) && (ss === 0 || ss === 1)) {
        currentSyncStripper = ss;
        console.log(`  ├─ ⚡ Pro: Set Sync Stripper to ${ss === 1 ? 'ON' : 'OFF'}`);
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end('true');
        broadcastStatus();
        return;
      }

      // Handle ADV Controller - Custom I2C command
      const c = params.get('c');
      if (c) {
        // Parse comma-separated hex values (e.g., "42,0E,00,56,17,02")
        const hexValues = c.split(',').map(v => v.trim());
        const bytes = hexValues.map(v => parseInt(v, 16));

        // Validate: must be multiples of 3 (addr, reg, val triplets)
        if (bytes.length > 0 && bytes.length % 3 === 0 && bytes.every(b => !isNaN(b) && b >= 0 && b <= 255)) {
          const tripletCount = bytes.length / 3;
          console.log(`  ├─ ⚡ Pro: ADV Controller - Custom I2C - ${tripletCount} triplet(s)`);

          // Log each triplet
          for (let t = 0; t < tripletCount; t++) {
            const addr = bytes[t * 3];
            const reg = bytes[t * 3 + 1];
            const val = bytes[t * 3 + 2];
            console.log(`  │   [${t + 1}] I2C Addr=0x${addr.toString(16).padStart(2, '0')}, Reg=0x${reg.toString(16).padStart(2, '0')}, Val=0x${val.toString(16).padStart(2, '0')}`);
          }

          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end('true');
        } else {
          console.log(`  ├─ ⚡ Pro: ADV Controller - Custom I2C ERROR - invalid data: ${c}`);
          res.writeHead(400, { 'Content-Type': 'application/json' });
          res.end('false');
        }
        return;
      }

      // No valid parameter provided
      res.writeHead(400, { 'Content-Type': 'application/json' });
      res.end('false');
    });
    return;
  }

  if (url.pathname.startsWith('/gbs/')) {
    console.log(`  ├─ 🔧 GBS Command: ${url.pathname}`);
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ status: 'ok' }));
    return;
  }

  if (url.pathname === '/filesystem/dir') {
    console.log(`  ├─ 📁 Filesystem Directory List`);
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify([
      { name: 'config.json', size: 1024, type: 'file' },
      { name: 'presets.bin', size: 2048, type: 'file' }
    ]));
    return;
  }

  if (url.pathname === '/filesystem/download') {
    const filename = url.searchParams.get('file');
    console.log(`  ├─ 📥 Filesystem Download: ${filename}`);
    res.writeHead(200, {
      'Content-Type': 'application/octet-stream',
      'Content-Disposition': `attachment; filename="${filename}"`
    });
    res.end(Buffer.from('Mock file content'));
    return;
  }

  if (url.pathname === '/filesystem/upload' && req.method === 'POST') {
    let body = '';
    req.on('data', chunk => {
      body += chunk.toString();
    });
    req.on('end', () => {
      console.log(`  ├─ 📤 Filesystem Upload: ${body.length} bytes`);
      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ status: 'ok' }));
    });
    return;
  }

  // Handle index.html with template interpolation
  if (url.pathname === '/') {
    console.log(`  ├─ 🏠 Serving index.html`);

    const htmlTemplate = fs.readFileSync(path.join(__dirname, 'src', 'index.html.tpl'), 'utf-8');
    const js = fs.readFileSync(path.join(__dirname, 'src', 'index.js'), 'utf-8');
    const css = fs.readFileSync(path.join(__dirname, 'src', 'style.css'), 'utf-8');

    // Read assets as base64
    const icon1024 = fs.readFileSync(path.join(__dirname, 'assets/icons/icon-1024-maskable.png')).toString('base64');
    const favicon = fs.readFileSync(path.join(__dirname, 'assets/icons/gbsc-logo.png')).toString('base64');
    const oswald = fs.readFileSync(path.join(__dirname, 'assets/fonts/oswald.woff2')).toString('base64');
    const material = fs.readFileSync(path.join(__dirname, 'assets/fonts/material.woff2')).toString('base64');

    // Process CSS with fonts
    const cssProcessed = css
      .replace('${oswald}', oswald)
      .replace('${material}', material);

    // Process manifest
    const manifestContent = fs.readFileSync(path.join(__dirname, 'src', 'manifest.json'), 'utf-8')
      .replace(/\$\{icon1024\}/g, `data:image/png;base64,${icon1024}`);

    // Generate final HTML with all substitutions
    const htmlFinal = htmlTemplate
      .replace('${styles}', cssProcessed)
      .replace('${js}', js)
      .replace('${favicon}', `data:image/png;base64,${favicon}`)
      .replace('${manifest}', `data:application/json;base64,${Buffer.from(manifestContent).toString('base64')}`)
      .replace('${icon1024}', `data:image/png;base64,${icon1024}`);

    res.writeHead(200, { 'Content-Type': 'text/html' });
    res.end(htmlFinal);
    return;
  }

  // Serve static files
  let filePath = path.join(__dirname, 'src', url.pathname);

  fs.readFile(filePath, (err, data) => {
    if (err) {
      console.log(`  └─ ❌ 404 Not Found: ${url.pathname}`);
      res.writeHead(404);
      res.end('Not found');
      return;
    }

    const ext = path.extname(filePath);
    const contentType = mimeTypes[ext] || 'text/plain';
    console.log(`  └─ 📄 Static file: ${url.pathname}`);

    res.writeHead(200, { 'Content-Type': contentType });
    res.end(data);
  });
};

// Main HTTP server (port 8080 for web interface)
const server = http.createServer(handleRequest);

// Alternative HTTP server (port 80 for /sc and /uc)
const serverAlt = http.createServer(handleRequest);

// WebSocket server to simulate GBS-Control Pro device
const wss = new WebSocketServer({ port: WS_PORT });

let currentPreset = 1;
let currentSlot = 'A';
let optionByte0 = 0x01; // adcAutoGain active
let optionByte1 = 0x02; // frameTimeLock active
let optionByte2 = 0x00;
let currentInputType = 1; // 1=RGBs, 2=RGsB, 3=VGA, 4=YPbPr, 5=S-Video, 6=Composite
let currentFormat = 0; // 0-11: Auto, PAL, NTSC-M, PAL-60, NTSC443, NTSC-J, PAL-N w/ p, PAL-M w/o p, PAL-M, PAL Cmb -N, PAL Cmb -N w/ p, SECAM
let current2X = 0; // 0=off, 1=on
let currentSmooth = 0; // 0=off, 1=on
let currentSharpness = 0; // 0=off, 1=on
let currentACE = 0; // 0=off, 1=on
let currentACELumaGain = 13; // 0-31, default 13 (0x0D)
let currentACEChromaGain = 8; // 0-15, default 8
let currentACEChromaMax = 8; // 0-15, default 8
let currentACEGammaGain = 8; // 0-15, default 8
let currentACEResponseSpeed = 15; // 0-15, default 15

// Video Filter state
let currentFilterY = 1;           // AV default (AutoNarrow)
let currentFilterC = 0;           // AV default (Auto1.5M)
let currentFilterWY = 9;          // SV default (SVHS-8)
let currentFilterWYOverride = 1;  // SV default (Manual)
let currentFilterComb = 1;        // Default (Medium)

// HDMI Limited Range state
let currentHdmiLimitedRange = 1;  // 0=Off, 1=HD, 2=SD, 3=All (default 1=HD)

// Sync Stripper state
let currentSyncStripper = 1;  // 0=Off, 1=On (default 1=On)

// Helper to convert value 0-31 to hex char (0-9, A-V)
const toHexChar32 = (val) => {
  if (val < 10) return String.fromCharCode(48 + val); // '0'-'9'
  return String.fromCharCode(65 + val - 10); // 'A'-'V'
};

// Helper to convert value 0-15 to hex char (0-9, A-F)
const toHexChar16 = (val) => {
  if (val < 10) return String.fromCharCode(48 + val); // '0'-'9'
  return String.fromCharCode(65 + val - 10); // 'A'-'F'
};

// Build Pro status message (19 chars)
// Format: $[input][format][2x][smooth][sharpness][ace][lumaGain][chromaGain][chromaMax][gammaGain][responseSpeed]
//         [yFilter][cFilter][wyFilter][wyOverride][comb][hdmiLimitedRange][syncStripper]
const buildProStatusMessage = () => {
  let formatChar;
  if (currentFormat <= 9) {
    formatChar = String.fromCharCode(48 + currentFormat);
  } else if (currentFormat === 10) {
    formatChar = 'A';
  } else {
    formatChar = 'B';
  }

  // Base message (12 chars) + 5 filter chars + 1 hdmiLimitedRange + 1 syncStripper = 19 chars
  return `$${currentInputType}${formatChar}${current2X}${currentSmooth}${currentSharpness}${currentACE}` +
         `${toHexChar32(currentACELumaGain)}${toHexChar16(currentACEChromaGain)}` +
         `${toHexChar16(currentACEChromaMax)}${toHexChar16(currentACEGammaGain)}` +
         `${toHexChar16(currentACEResponseSpeed)}` +
         `${toHexChar32(currentFilterY)}${toHexChar16(currentFilterC)}` +
         `${toHexChar16(currentFilterWY)}${currentFilterWYOverride}${toHexChar16(currentFilterComb)}` +
         `${currentHdmiLimitedRange}${currentSyncStripper}`;
};

// Broadcast status to all connected WebSocket clients
const broadcastStatus = () => {
  wss.clients.forEach((client) => {
    if (client.readyState === 1) { // OPEN
      const statusMessage = `#${currentPreset}${currentSlot}${String.fromCharCode(optionByte0)}${String.fromCharCode(optionByte1)}${String.fromCharCode(optionByte2)}`;
      client.send(statusMessage);
      client.send(buildProStatusMessage());
    }
  });
};

wss.on('connection', (ws) => {
  const timestamp = new Date().toLocaleTimeString();
  console.log(`\n[${timestamp}] 🔌 WebSocket client connected`);

  // Send initial status message
  const sendStatus = () => {
    if (ws.readyState === 1) { // OPEN
      const statusMessage = `#${currentPreset}${currentSlot}${String.fromCharCode(optionByte0)}${String.fromCharCode(optionByte1)}${String.fromCharCode(optionByte2)}`;
      const ts = new Date().toLocaleTimeString();
      console.log(`[${ts}] 📤 WS → Status: Preset=${currentPreset}, Slot=${currentSlot}, Options=[${optionByte0.toString(16)},${optionByte1.toString(16)},${optionByte2.toString(16)}]`);
      ws.send(statusMessage);

      // Pro status: 12 chars - $[input][format][2x][smooth][sharpness][ace][luma][chroma][chromamax][gamma][response]
      const inputNames = ['', 'RGBs', 'RGsB', 'VGA', 'YPbPr', 'S-Video', 'Composite'];
      const formatNames = ['Auto', 'PAL', 'NTSC-M', 'PAL-60', 'NTSC443', 'NTSC-J', 'PAL-N w/ p', 'PAL-M w/o p', 'PAL-M', 'PAL Cmb -N', 'PAL Cmb -N w/ p', 'SECAM'];

      console.log(`[${ts}] 📤 WS → Pro: Input=${inputNames[currentInputType]} (${currentInputType}), Format=${formatNames[currentFormat]} (${currentFormat}), 2X=${current2X ? 'ON' : 'OFF'}, Smooth=${currentSmooth ? 'ON' : 'OFF'}, Sharpness=${currentSharpness ? 'ON' : 'OFF'}, ACE=${currentACE ? 'ON' : 'OFF'}, LumaGain=${currentACELumaGain}, ChromaGain=${currentACEChromaGain}, ChromaMax=${currentACEChromaMax}, GammaGain=${currentACEGammaGain}, ResponseSpeed=${currentACEResponseSpeed}`);
      ws.send(buildProStatusMessage());
    }
  };

  // Send status every 2 seconds
  const statusInterval = setInterval(sendStatus, 2000);

  // Also send simulated log messages
  const logInterval = setInterval(() => {
    if (ws.readyState === 1) {
      const ts = new Date().toLocaleTimeString();
      console.log(`[${ts}] 📝 WS → Log messages`);
      ws.send('GBS-Control Pro ready\n');
      ws.send(`Preset: ${currentPreset}, Slot: ${currentSlot}\n`);
    }
  }, 15000);

  // Send initial status right after connection
  setTimeout(sendStatus, 100);

  ws.on('message', (message) => {
    const ts = new Date().toLocaleTimeString();
    const msg = message.toString();
    console.log(`[${ts}] 📨 WS ← Client message: "${msg}"`);
  });

  ws.on('close', () => {
    clearInterval(statusInterval);
    clearInterval(logInterval);
    const ts = new Date().toLocaleTimeString();
    console.log(`[${ts}] 🔌 WebSocket client disconnected`);
  });

  ws.on('error', (error) => {
    const ts = new Date().toLocaleTimeString();
    console.error(`[${ts}] ❌ WebSocket error:`, error);
  });
});

// Start servers
server.listen(HTTP_PORT, () => {
  console.log('╔════════════════════════════════════════════════════════╗');
  console.log('║  GBS-Control Pro Development Server                    ║');
  console.log('╠════════════════════════════════════════════════════════╣');
  console.log(`║  HTTP Server:      http://localhost:${HTTP_PORT}/           ║`);
  console.log(`║  HTTP Server Alt:  http://localhost:${HTTP_PORT_ALT}/             ║`);
  console.log(`║  WebSocket Server: ws://localhost:${WS_PORT}/              ║`);
  console.log('╠════════════════════════════════════════════════════════╣');
  console.log('║  Open http://localhost:8080/ in your browser           ║');
  console.log('╚════════════════════════════════════════════════════════╝');
});

// Start alternative server for /sc and /uc (requires sudo on Linux/Mac)
serverAlt.listen(HTTP_PORT_ALT, (err) => {
  if (err) {
    console.log('\n⚠️  Note: Server on port 80 was not started.');
    console.log('   On Linux/Mac you may need to run with sudo:');
    console.log('   sudo node dev-server.js');
    console.log('   Alternatively, /sc and /uc commands may not work.\n');
  }
});

wss.on('listening', () => {
  console.log('WebSocket server listening on port', WS_PORT);
});

wss.on('error', (error) => {
  console.error('WebSocket server error:', error);
  if (error.code === 'EADDRINUSE') {
    console.error(`Port ${WS_PORT} is already in use. Please close other applications using this port.`);
    process.exit(1);
  }
});

// Handle shutdown
process.on('SIGINT', () => {
  console.log('\nShutting down servers...');
  wss.close();
  server.close();
  process.exit(0);
});
