const fs = require("fs");
const html = fs.readFileSync("./../src/index.html.tpl", "utf-8");

// Parse GBS_FW_VERSION from gbs-control-pro.h so webapp always matches firmware.
const proHeader = fs.readFileSync("./../../pro/gbs-control-pro.h", "utf-8");
const versionMatch = proHeader.match(/GBS_FW_VERSION\s+"([^"]+)"/);
const firmwareVersion = versionMatch ? versionMatch[1] : "0.0.0";

const js = fs
  .readFileSync("./../src/index.js", "utf-8")
  .replace(/"__FW_VERSION__"/g, JSON.stringify(firmwareVersion));

const icon1024 = fs
  .readFileSync("./../assets/icons/icon-1024-maskable.png")
  .toString("base64");
const oswald = fs
  .readFileSync("./../assets/fonts/oswald.woff2")
  .toString("base64");
const material = fs
  .readFileSync("./../assets/fonts/material.woff2")
  .toString("base64");
const favicon = fs
  .readFileSync("./../assets/icons/gbsc-logo.png")
  .toString("base64");

const css = fs
  .readFileSync("./../src/style.css", "utf-8")
  .replace("${oswald}", oswald)
  .replace("${material}", material);

const manifest = fs
  .readFileSync("./../src/manifest.json", "utf-8")
  .replace(/\$\{icon1024\}/g, `data:image/png;base64,${icon1024}`);

fs.writeFileSync(
  "./../../webui.html",
  html
    .replace("${styles}", css)
    .replace("${js}", js)
    .replace("${favicon}", `data:image/png;base64,${favicon}`)
    .replace(
      "${manifest}",
      `data:application/json;base64,${Buffer.from(manifest).toString("base64")}`
    )
    .replace("${icon1024}", `data:image/png;base64,${icon1024}`),
  "utf8"
);

console.log("webui.html GENERATED");
