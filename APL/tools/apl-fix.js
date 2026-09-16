// Run from the NOS 2.8.7 Operator> prompt as:  !node <this file>
// Runs APL1FIX: sets AC=Y on the APL1 public-library workspaces so that
// CATLIST,UN=APL1 and APL's )LIB *APL1 / quad-LIB can list them.
const path = require("path");
process.chdir(path.join(__dirname, "../../NOS2.8.7"));
const DtCyber = require(path.join(__dirname, "../../automation/DtCyber.js"));
const decks = path.join(__dirname, "../jobs").split(path.sep).join("/");
const dtc = new DtCyber();
dtc.connect()
.then(() => dtc.expect([{ re: /Operator> $/ }]))
.then(() => dtc.attachPrinter("LP5xx_C12_E5"))
.then(() => dtc.say("Running APL1FIX ..."))
.then(() => dtc.runJob(12, 4, `${decks}/apl1fix.job`))
.then(() => dtc.say("APL1FIX complete"))
.then(() => process.exit(0))
.catch(err => { console.log(err); process.exit(1); });
