#!/usr/bin/env node
// Convert DUNGEON's DEC tab-form VAX FORTRAN source (src/*.for) into a fixed
// column FORTRAN 77 UPDATE/MODIFY creation deck that CDC FORTRAN 5 (FTN5) on
// NOS 2 will accept (nos/dungeon.upd).
//
// Usage: node convert.js [srcdir] [outdir]
//
// What it does, mechanically:
//   - DEC tab form -> columns: label in 1-5, continuation digit in 6, body
//     from 7. A tab followed by a digit 1-9 marks a continuation line.
//   - strips "!" trailing comments (outside quoted strings), form feeds,
//     blank lines, and stray tabs
//   - DEC "D" debug lines become comments
//   - INCLUDE 'DPARAM.FOR[/NOLIST]' becomes *CALL DPARAM; dparam.for itself
//     becomes *COMDECK DPARAM and every other file a *DECK named after it
//   - DEC octal constants 'nnn'O become decimal integers
//   - the DEC TYPE statement becomes PRINT
//   - everything is uppercased (NOS display code has no lowercase)
//   - runs of one-name PARAMETER statements are packed into one statement
//     (dparam.for has ~480 of them, and it is expanded into every routine)
//   - statements with more than 19 continuation lines (FTN5's limit) are
//     repacked densely, and DATA (X(I),I=a,b)/.../ statements that are still
//     too long are split in two; anything else too long is reported
//   - bodies past column 72 are wrapped onto extra continuation lines
//
// Everything else that FTN5 objects to (OPEN keywords, mixed CHARACTER and
// numeric COMMON, .XOR., "$" and "+" carriage control, VMS system calls) is
// edited by hand in src/ so that "diff orig src" documents the port.
'use strict';
const fs = require('fs');
const path = require('path');

const here = path.dirname(__filename);
const srcDir = process.argv[2] || path.join(here, '..', 'src');
const outDir = process.argv[3] || path.join(here, '..', 'nos');

const MAXCONT = 19;    // FTN5 continuation line limit
const BODYMAX = 66;    // columns 7-72

let warnings = 0;
function warn(file, lineNo, msg) {
  console.log(`${file}:${lineNo}: ${msg}`);
  warnings++;
}

// Cut a "!" comment, honouring single-quoted strings ('' is an escaped quote,
// which toggles twice and so needs no special case).
function stripBang(s) {
  let inQuote = false;
  for (let i = 0; i < s.length; i++) {
    const c = s[i];
    if (c === "'") inQuote = !inQuote;
    else if (c === '!' && !inQuote) return s.slice(0, i);
  }
  return s;
}

function octalToDecimal(s) {
  return s.replace(/'([0-7]+)'O\b/g, (m, digits) => String(parseInt(digits, 8)));
}

// Parse one DEC tab-form source line into {kind, ...}.
function parseLine(raw) {
  const line = raw.replace(/\f/g, '').replace(/\s+$/, '');
  if (line === '') return { kind: 'blank' };
  const c0 = line[0];
  if (c0 === 'C' || c0 === 'c' || c0 === '*' || c0 === '!') {
    return { kind: 'comment', text: line.slice(1) };
  }
  if (c0 === 'D' && /^D\t|^D\d{1,5}\t/.test(line)) {
    return { kind: 'comment', text: ' DEBUG: ' + line.slice(1).replace(/\t/g, ' ') };
  }
  let m;
  if ((m = /^\t([1-9])\t?(.*)$/.exec(line))) {
    return { kind: 'cont', body: m[2] };
  }
  if ((m = /^\t(.*)$/.exec(line))) {
    return { kind: 'stmt', label: '', body: m[1] };
  }
  if ((m = /^(\d{1,5})\t(.*)$/.exec(line))) {
    return { kind: 'stmt', label: m[1], body: m[2] };
  }
  if ((m = /^ {5}[^ 0](.*)$/.exec(line))) {
    return { kind: 'cont', body: m[1] };
  }
  if ((m = /^([ \d]{5}) (.*)$/.exec(line))) {
    return { kind: 'stmt', label: m[1].trim(), body: m[2] };
  }
  return { kind: 'unknown', text: line };
}

function cleanBody(body) {
  let s = stripBang(body).replace(/\t/g, ' ').replace(/\s+$/, '');
  s = octalToDecimal(s);
  // DEC TYPE statement, bare or as the object of a logical IF. A format
  // label always follows, which keeps OPEN's TYPE= keyword out of it.
  s = s.replace(/\bTYPE(?=\s+\d)/i, 'PRINT');
  return s.toUpperCase();
}

// Split a DATA value list on commas outside quotes.
function splitList(list) {
  const items = [];
  let cur = '';
  let inQuote = false;
  for (const c of list) {
    if (c === "'") inQuote = !inQuote;
    if (c === ',' && !inQuote) {
      items.push(cur.trim());
      cur = '';
    } else {
      cur += c;
    }
  }
  items.push(cur.trim());
  return items;
}

// Chunk a body into lines of at most BODYMAX columns. A split may fall inside
// a quoted string; that is fine in fixed form, since columns 7-72 of a
// continuation line follow on exactly.
function chunks(body) {
  const out = [];
  let rest = body;
  do {
    out.push(rest.slice(0, BODYMAX));
    rest = rest.slice(BODYMAX);
  } while (rest.length > 0);
  return out;
}

// A statement is {label, bodies: [first, cont1, cont2, ...], line, file}.
// Turn it into output lines, dealing with the continuation limit.
function emitStatement(out, stmt) {
  const lines = [];
  stmt.bodies.forEach(b => lines.push(...chunks(b)));
  if (lines.length - 1 > MAXCONT) {
    return emitLong(out, stmt);
  }
  lines.forEach((l, i) => {
    if (i === 0) out.push(stmt.label.padStart(5) + ' ' + l);
    else out.push('     ' + String(((i - 1) % 9) + 1) + l);
  });
}

function emitLong(out, stmt) {
  // Dense repack: blanks outside strings are insignificant and a string may
  // continue across a line boundary, so concatenating the bodies exactly and
  // re-chunking preserves meaning.
  const joined = stmt.bodies.join('');
  let lines = chunks(joined);
  if (lines.length - 1 <= MAXCONT) {
    return emitStatement(out, { ...stmt, bodies: [joined] });
  }
  const m = /^\s*DATA\s*\((\w+)\((\w+)\),(\w+)=(\d+),(\d+)\)\s*\/(.*)\/\s*$/.exec(joined);
  if (m && m[2] === m[3]) {
    const [, arr, idx, , lo, hi, list] = m;
    const items = splitList(list);
    const n = Number(hi) - Number(lo) + 1;
    if (items.length === n && !items.some(t => /^[^']*\*/.test(t))) {
      const half = Math.ceil(items.length / 2);
      const mid = Number(lo) + half - 1;
      emitLong(out, { ...stmt, bodies: [`DATA (${arr}(${idx}),${idx}=${lo},${mid}) /${items.slice(0, half).join(',')}/`] });
      emitLong(out, { ...stmt, label: '', bodies: [`DATA (${arr}(${idx}),${idx}=${mid + 1},${hi}) /${items.slice(half).join(',')}/`] });
      return;
    }
  }
  warn(stmt.file, stmt.line, `statement has ${lines.length - 1} continuation lines after repacking (limit ${MAXCONT})`);
  emitStatement(out, { ...stmt, bodies: [joined] });
}

// Merge runs of "PARAMETER (NAME=value)" statements into one statement.
function packParameters(stmts) {
  const out = [];
  let run = [];
  const flush = () => {
    while (run.length > 0) {
      let body = 'PARAMETER (';
      let n = 0;
      while (n < run.length && (n === 0 || body.length + run[n].length + 2 <= BODYMAX)) {
        body += (n > 0 ? ',' : '') + run[n];
        n++;
      }
      out.push({ kind: 'stmt', label: '', bodies: [body + ')'], line: run.line, file: run.file });
      const { line, file } = run; run = run.slice(n); run.line = line; run.file = file;
    }
  };
  for (const s of stmts) {
    const m = s.kind === 'stmt' && s.label === '' && s.bodies.length === 1
      && /^\s*PARAMETER\s*\((\w+=[^,()]+)\)\s*$/.exec(s.bodies[0]);
    if (m) {
      if (run.length === 0) { run.line = s.line; run.file = s.file; }
      run.push(m[1]);
    } else {
      flush();
      out.push(s);
    }
  }
  flush();
  return out;
}

// Read a source file into a list of {kind: 'comment'|'stmt'|'call', ...}.
function readFile(name) {
  const file = path.join(srcDir, name);
  const src = fs.readFileSync(file, 'utf8').replace(/\r\n?/g, '\n').split('\n');
  const items = [];
  let cur = null;
  src.forEach((raw, idx) => {
    const lineNo = idx + 1;
    const p = parseLine(raw);
    switch (p.kind) {
    case 'blank':
      break;
    case 'comment':
      items.push({ kind: 'comment', text: ('C' + p.text.replace(/\t/g, ' ').replace(/\s+$/, '')).toUpperCase().slice(0, 72) });
      break;
    case 'stmt': {
      const body = cleanBody(p.body);
      if (body === '') break;
      if (/^INCLUDE\s+'DPARAM\.FOR/i.test(body)) {
        cur = null;
        items.push({ kind: 'call', text: '*CALL DPARAM' });
        break;
      }
      cur = { kind: 'stmt', label: p.label, bodies: [body], line: lineNo, file: name };
      items.push(cur);
      break;
    }
    case 'cont': {
      const body = cleanBody(p.body);
      if (body === '') break;
      if (!cur) {
        warn(name, lineNo, 'continuation line with no statement');
        break;
      }
      cur.bodies.push(body);
      break;
    }
    case 'unknown':
      warn(name, lineNo, `unrecognised line form: ${JSON.stringify(p.text)}`);
      break;
    }
  });
  return items;
}

function emitItems(items) {
  const out = [];
  for (const it of items) {
    if (it.kind === 'stmt') emitStatement(out, it);
    else out.push(it.text);
  }
  return out;
}

const deck = [];
const dparam = emitItems(packParameters(readFile('dparam.for')));
deck.push('*COMDECK DPARAM', ...dparam);
console.log(`dparam.for -> *COMDECK DPARAM (${dparam.length} lines)`);
const files = fs.readdirSync(srcDir).filter(f => /\.for$/i.test(f) && f !== 'dparam.for').sort();
for (const f of files) {
  const name = f.replace(/\.for$/i, '').toUpperCase();
  const lines = emitItems(readFile(f));
  deck.push(`*DECK ${name}`, ...lines);
  console.log(`${f} -> *DECK ${name} (${lines.length} lines)`);
}
fs.mkdirSync(outDir, { recursive: true });
const outFile = path.join(outDir, 'dungeon.upd');
fs.writeFileSync(outFile, deck.join('\n') + '\n');
const bad = deck.filter(l => l.length > 72 && !l.startsWith('*'));
if (bad.length > 0) warn('output', 0, `${bad.length} line(s) exceed 72 columns`);
console.log(`${deck.length} lines -> ${outFile}, ${warnings} warning(s)`);
process.exit(warnings > 0 ? 1 : 0);
