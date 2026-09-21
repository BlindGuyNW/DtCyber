#!/usr/bin/env node
// Convert DUNGEON's DTEXT.DAT (VMS binary, 80-byte records, XOR-scrambled
// ASCII) into 80-column card images for NOS: I4 message group number followed
// by 76 characters of text, uppercased and restricted to the CDC 64-character
// display code set.
//
// Usage: node dtext.js [orig/dtext.dat] [nos/dtext.txt]
//
// Record layout (see game.for INITFL and subr.for RSPSB2):
//   bytes 0-3   little-endian int: record number of the first line of the
//               message this line belongs to (used to detect continuation)
//   bytes 4-79  text, each byte XORed with ((rec & 31) + i) for i = 1..76,
//               where rec is this record's 1-based index in the file
//               (TXCRYP in dungeon.for).
// The final record (group -1, "END OF DATA BASE FILE") is stored in the clear.
'use strict';
const fs = require('fs');
const path = require('path');

const here = path.dirname(__filename);
const inFile = process.argv[2] || path.join(here, '..', 'orig', 'dtext.dat');
const outFile = process.argv[3] || path.join(here, '..', 'nos', 'dtext.txt');

const RECLEN = 80;
const TEXLNT = 76;

// CDC display code (64-character set) has no lowercase and lacks a few ASCII
// glyphs. Everything not in this set is mapped or reported.
const DISPLAY = new Set(
  'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-*/()$= ,.#[]%"_!&\'?<>@\\^;:'.split(''));
const MAP = { '\t': ' ', '{': '(', '}': ')', '|': '!', '~': '-', '`': '\'' };

const buf = fs.readFileSync(inFile);
if (buf.length % RECLEN !== 0) {
  throw new Error(`${inFile}: length ${buf.length} is not a multiple of ${RECLEN}`);
}
const nrec = buf.length / RECLEN;
const lines = [];
const unmapped = new Map();
for (let r = 1; r <= nrec; r++) {
  const off = (r - 1) * RECLEN;
  const group = buf.readInt32LE(off);
  let text = '';
  for (let i = 1; i <= TEXLNT; i++) {
    const x = group < 0 ? 0 : (r & 31) + i;
    let c = String.fromCharCode(buf[off + 3 + i] ^ x);
    c = c.toUpperCase();
    if (MAP[c] !== undefined) c = MAP[c];
    if (!DISPLAY.has(c)) {
      unmapped.set(c, (unmapped.get(c) || 0) + 1);
      c = ' ';
    }
    text += c;
  }
  lines.push(String(group).padStart(4) + text.replace(/\s+$/, ''));
}
fs.writeFileSync(outFile, lines.join('\n') + '\n');
console.log(`${nrec} records -> ${outFile}`);
if (unmapped.size > 0) {
  console.log('characters replaced by blank:',
    [...unmapped].map(([c, n]) => `${JSON.stringify(c)} x${n}`).join(', '));
}
