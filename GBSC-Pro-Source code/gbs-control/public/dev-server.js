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

// Mock slot data
const createMockSlotsData = () => {
  // Slot structure from code:
  // { name: "name", type: "string", size: 25 },
  // { name: "presetID", type: "byte", size: 1 },
  // { name: "scanlines", type: "byte", size: 1 },
  // { name: "scanlinesStrength", type: "byte", size: 1 },
  // { name: "slot", type: "byte", size: 1 },
  // { name: "wantVdsLineFilter", type: "byte", size: 1 },
  // { name: "wantStepResponse", type: "byte", size: 1 },
  // { name: "wantPeaking", type: "byte", size: 1 },
  const slotSize = 25 + 1 + 1 + 1 + 1 + 1 + 1 + 1; // = 32 bytes
  const numSlots = 72;
  const buffer = Buffer.alloc(slotSize * numSlots);

  for (let i = 0; i < numSlots; i++) {
    const offset = i * slotSize;

    // Slot name (25 bytes, null-terminated or space-padded)
    const name = `Slot ${i}`;
    buffer.write(name, offset, 25, 'ascii');
    // Fill the rest with spaces (optional, but clean)
    for (let j = name.length; j < 25; j++) {
      buffer.writeUInt8(0x20, offset + j); // space
    }

    // Slot data
    buffer.writeUInt8(0x01, offset + 25); // presetID
    buffer.writeUInt8(0x00, offset + 26); // scanlines (0 = off)
    buffer.writeUInt8(0x30, offset + 27); // scanlinesStrength
    buffer.writeUInt8(i, offset + 28);     // slot index
    buffer.writeUInt8(0x01, offset + 29); // wantVdsLineFilter
    buffer.writeUInt8(0x00, offset + 30); // wantStepResponse
    buffer.writeUInt8(0x00, offset + 31); // wantPeaking
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
    const command = url.searchParams.toString().split('&')[0];
    console.log(`  ├─ 🎮 Serial Command: ${command}`);
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('OK');
    return;
  }

  if (url.pathname.startsWith('/uc')) {
    const command = url.searchParams.toString().split('&')[0];
    console.log(`  ├─ ⚙️  User Command: ${command}`);
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

      // Pro status: 5 bytes - $[inputType][format][2x][smooth]
      const inputNames = ['', 'RGBs', 'RGsB', 'VGA', 'YPbPr', 'S-Video', 'Composite'];
      const formatNames = ['Auto', 'PAL', 'NTSC-M', 'PAL-60', 'NTSC443', 'NTSC-J', 'PAL-N w/ p', 'PAL-M w/o p', 'PAL-M', 'PAL Cmb -N', 'PAL Cmb -N w/ p', 'SECAM'];

      // Format encoding: 0-9 = '0'-'9', 10 = 'A', 11 = 'B'
      let formatChar;
      if (currentFormat <= 9) {
        formatChar = String.fromCharCode(48 + currentFormat); // '0' + offset
      } else if (currentFormat === 10) {
        formatChar = 'A';
      } else {
        formatChar = 'B';
      }

      console.log(`[${ts}] 📤 WS → Pro: Input=${inputNames[currentInputType]} (${currentInputType}), Format=${formatNames[currentFormat]} (${currentFormat}), 2X=${current2X ? 'ON' : 'OFF'}, Smooth=${currentSmooth ? 'ON' : 'OFF'}`);
      ws.send(`$${currentInputType}${formatChar}${current2X}${currentSmooth}`);
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
