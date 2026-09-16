// usage: node nos-session.js USER PASS delayMs "line" ...
// Like apl-session.js but never waits for a prompt after login; just paces lines by delay.
const Terminal = require(require("path").join(__dirname, "../../automation/Terminal.js"));
const [user, pass, delay, ...lines] = process.argv.slice(2);
const term = new Terminal.AnsiTerminal();
term.setTracer();
term.setDefaultTimeout(30, () => "timeout");
let p = term.connect(23)
  .then(() => term.expect([{ re: /FAMILY:/ }]))
  .then(() => term.sleep(1000))
  .then(() => term.send(`,${user},${pass}\r`))
  .then(() => term.sleep(Number(delay) * 2));
for (const l of lines) p = p.then(() => term.send(l + "\r")).then(() => term.sleep(Number(delay)));
p.then(() => process.exit(0)).catch(err => { console.log(err); process.exit(1); });
