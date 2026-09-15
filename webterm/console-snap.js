// Print a one-shot text snapshot of the DtCyber console screens (left = A, right = B).
// Usage: node console-snap.js [seconds-to-listen] [port] [left|right|both]   (the console port takes one client at a time)
const path = require("path");
const WT = __dirname;
const CyberConsoleBase = require(WT + "/www/js/console-base");
const Machine = require(WT + "/textconsole/js/machine-tcp");
let view = "both";
const nums = [];
for (const a of process.argv.slice(2)) {
  if (/^(left|right|both|dual)$/i.test(a)) view = a.toLowerCase() === "dual" ? "both" : a.toLowerCase();
  else nums.push(a);
}
const secs = parseFloat(nums[0] || "4");
const port = parseInt(nums[1] || "16612", 10);
const screens = view === "both" ? [0, 1] : [view === "left" ? 0 : 1];
class Snap extends CyberConsoleBase {
  constructor() { super(); this.scr = 0; this.grid = [this.blank(), this.blank()]; this.last = null; }
  blank() { return Array.from({length: 52}, () => Array(66).fill(" ")); }
  setScreen(n) { this.scr = n ? 1 : 0; }
  drawChar(b) {
    const cx = Math.floor(this.x / 8), cy = Math.floor(this.y / 10);
    if (cy >= 0 && cy < 52 && cx >= 0 && cx < 66) this.grid[this.scr][cy][cx] = String.fromCharCode(b);
  }
  updateScreen() { this.last = this.grid.map(g => g.map(r => r.join("").replace(/\s+$/, ""))); this.grid = [this.blank(), this.blank()]; }
}
const snap = new Snap();
const m = new Machine("console", "127.0.0.1", port);
m.setReceivedDataHandler(d => snap.renderText(d));
m.setConnectListener(() => { m.send(new Uint8Array([0x80, 250, 0x81])); });
m.createConnection();
setTimeout(() => {
  if (!snap.last) { console.log("no frame received"); process.exit(1); }
  for (const i of screens) {
    const name = i === 0 ? "LEFT SCREEN" : "RIGHT SCREEN";
    console.log(`===== ${name} =====`);
    snap.last[i].forEach((l, n) => { if (l.length) console.log(String(n).padStart(2) + "| " + l); });
  }
  m.closeConnection(); process.exit(0);
}, secs * 1000);
