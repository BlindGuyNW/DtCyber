// Run from the NOS 2.8.7 Operator> prompt as:  !node <this file>
// Mounts the three release tapes, runs CATLIST jobs under APL0 and APL1,
// and catalogs the tapes. All output goes to the LP5xx_C12_E5 printer file.
const path = require("path");
process.chdir(path.join(__dirname, "../../NOS2.8.7"));
const DtCyber = require(path.join(__dirname, "../../automation/DtCyber.js"));
const decks = path.join(__dirname, "../jobs").split(path.sep).join("/");
const dtc = new DtCyber();

const job = name => () => dtc.say(`Running ${name} ...`)
  .then(() => dtc.runJob(12, 4, `${decks}/${name}.job`))
  .then(() => dtc.say(`${name} complete`))
  .catch(err => dtc.say(`${name}: ${err.message}`));

dtc.connect()
.then(() => dtc.expect([{ re: /Operator> $/ }]))
.then(() => dtc.attachPrinter("LP5xx_C12_E5"))
.then(job("apl0cat"))
.then(job("apl1cat"))
.then(() => dtc.mount(13, 0, 1, "tapes/nos287-1.tap"))
.then(() => dtc.mount(13, 0, 2, "tapes/nos287-2.tap"))
.then(() => dtc.mount(13, 0, 3, "tapes/nos287-3.tap"))
.then(job("tapecat"))
.then(() => dtc.unmount(13, 0, 1))
.then(() => dtc.unmount(13, 0, 2))
.then(() => dtc.unmount(13, 0, 3))
.then(() => dtc.say("Done"))
.then(() => process.exit(0))
.catch(err => { console.log(err); process.exit(1); });
