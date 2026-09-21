#!/usr/bin/env node
// Submit NOS2.8.7/opt/dungeon.job to the running NOS 2.8.7 system and wait
// for it to finish.
//
// The operator port takes one connection, and the user's `node start`
// normally holds it, so this is meant to be typed at their Operator> prompt:
//
//   !node C:\git\DtCyber\dungeon\tools\build-nos.js
//
// The "!" extension frees the port for the duration of the command. The
// job's output, including the FTN5 listing, goes to the printer file
// NOS2.8.7/LP5xx_C12_E5.
//
// This is the same job that `node install-product dungeon` runs, minus
// dungeon.post (which moves the built files to UN=LIBRARY) and the
// SYSEDIT of PROC/DUNGEON. The job's ${1} is the deadstart library
// number for games, taken from the product's "lib" in opt/products.json
// as install-product.js does.
'use strict';
const fs = require('fs');
const path = require('path');

const osDir = path.resolve(__dirname, '..', '..', 'NOS2.8.7');
process.chdir(osDir);
const DtCyber = require(path.join(osDir, '..', 'automation', 'DtCyber'));
const dtc = new DtCyber();

const products = JSON.parse(fs.readFileSync('opt/products.json', 'utf8'));
const product = products.flatMap(c => c.products).find(p => p.name === 'dungeon');
if (!product) {
  console.log('dungeon is not defined in opt/products.json');
  process.exit(1);
}

const t0 = Date.now();
dtc.connect()
.then(() => dtc.expect([ {re:/Operator> $/} ]))
.then(() => dtc.attachPrinter('LP5xx_C12_E5'))
.then(() => dtc.say('Connected; submitting opt/dungeon.job'))
.then(() => dtc.runJob(12, 4, 'opt/dungeon.job', [product.lib, 51]))
.then(() => dtc.say(`DUNGEON job complete in ${Math.round((Date.now() - t0) / 1000)} s; see LP5xx_C12_E5`))
.then(() => dtc.disconnect())
.then(() => process.exit(0))
.catch(err => {
  console.log(`${new Date().toLocaleTimeString()} ${err.message}`);
  console.log('See LP5xx_C12_E5 for the job output');
  try { dtc.disconnect(); } catch (e) { /* ignore */ }
  process.exit(1);
});
