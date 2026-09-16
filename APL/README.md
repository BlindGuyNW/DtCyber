# APL on NOS 2.8.7

Notes, listings, and tools collected while working out how the APL system on the
NOS 2.8.7 image works. Nothing here is part of the installer; it is a study directory.

## What the APL system is

* The interpreter is a single absolute program on the deadstart file: record `APL`,
  type ABS, product code `APL2`, dated 97/11/06, entry points `APL` and `APLUM`
  (see `CATALOG,SYSTEM` output). It identifies itself as `APL2.1.014`.
* It is CDC's productized version of APLUM, the University of Massachusetts APL.
  The `SOURCE` function in every public workspace still says "APL GROUP, UCC,
  GRAD. RES. CTR., U. OF MASS., AMHERST".
* Manuals (PDF plus OCR text in `C:\git\cdc-manuals`, outside this repo):
  * 60454000F, *APL Version 2 Reference Manual*, Nov 1980. The authoritative one.
  * *APLUM Reference Manual* (Tom Hunter scan). Version 2.12 of APLUM; same
    system, more public workspaces.
  * 60459680L, *NOS 2 Reference Set Volume 3, System Commands*, Dec 1988. For the
    permanent-file commands the interpreter sits on top of.

## Workspaces and libraries

* A workspace is an ordinary NOS permanent file in your catalog. `)SAVE name`
  makes an indirect access private file; `name/S`, `name/PU`, `name/DA` choose
  semiprivate, public, or direct access (60454000F section 8, "Workspaces").
  Names are 1 to 7 letters and digits, the NOS limit.
* A "library" is just another user's catalog. `)LOAD *APL1 CATALOG` means the
  file CATALOG in user APL1's catalog. `*APL0 CLEARWS` is a special case equal to
  `)CLEAR` (section 8, `⎕LOAD`).
* Section 11 says the release ships three workspaces under user APL1:
  `APLNEWS`, `FILESYS`, and `CATALOG`. This image actually has seven direct
  access public files there: those three plus `FILES2`, `WSFNS`, `TAPLWS`, and
  `TOTALWS` (all dated 23/11/08, so added by the image builder). Each has `ABSTRACT`, `DESCRIBE`, and
  `SOURCE` documentation objects and a `GRPDOC` group naming them. APLUM (page
  10-1) also shipped WSFNS, FILES2, PLOTFNS and STP1 to STP5; CDC dropped those.
* This image has users APL0 and APL1 (created by
  `NOS2.8.7/decks/update-zzsysgu.job`, passwords default to the user names).
  Neither is validated for interactive login, so look at them with batch jobs.

## Why `)LOAD *APL1 CATALOG` then `LIST APL1` prints nothing

The CATALOG workspace's driver (line 49 of `CATALOG` in
`workspaces/CATALOG.apl`) does `⎕LIB '*APL1'` and keeps the rows whose type is
`WS`. `⎕LIB` on another user's catalog sees exactly what NOS `CATLIST,UN=APL1`
sees, and on this image that is "EMPTY CATALOG." even though
`ATTACH,CATALOG/UN=APL1` works from GUEST.

The reason is the NOS *alternate CATLIST permission*. From 60459680L, CATLIST
command: "the only files listed are those that the owner has explicitly
permitted to be listed (using the AC parameter on a CHANGE, DEFINE, or SAVE
command)". `AC=ac` defaults to N (CHANGE command, page 10-15). The APL1
workspaces were saved with the default, so they are loadable but invisible to
listings. The repo's own `NOS2.8.7/decks/modopl.job` does
`CHANGE,OPL871/CT=PU,M=R,AC=Y` for the same reason.

Fix: run `jobs/apl1fix.job` (as user APL1: `CHANGE,name/AC=Y` for all seven
workspaces). `tools/apl-fix.js` runs it through the operator port.

Until then, the parts of CATALOG that do not depend on a listing still work:
`ABSTRACT APL1 APLNEWS` copies `ABSTRACT` out of the named workspace with
`⎕COPY`, and `)LOAD *APL1 FILESYS` and `)LOAD *APL1 APLNEWS` load fine.

## Terminals and typing APL symbols

* `APL` alone from a Telnet session assumes `TT=ASCAPL` (60454000F, "APL command",
  `TT` option). In that mode the `$xx` mnemonics are not recognized.
* `APL,TT=713` treats the terminal as plain ASCII and accepts and prints APL
  symbols as `$` plus two letters, per Table C-2 in Appendix C: `$DL` ∇,
  `$QD` ⎕, `$IS` ←, `$GO` →, `$RO` ⍴, `$IO` ⍳, `$TA` ↑, `$DR` ↓, `$EP` ∈,
  `$AN` ∧, `$OR` ∨, `$NE` ≠, `$SM` ⌿, `$NG` ¯, `$TL` ~, `$AA`..`$ZZ` the
  underscored letters. `tools/apl-glyphs.py` holds the full map.
* So `∇CATALOG[⎕]∇` is typed as `$DLCATALOG[$QD]$DL`. `)WSID` is not
  implemented ("USE SYSTEM VARIABLES").
* The browser terminal in `webterm` uses `APL,TT=TYPE` with its own keyboard
  mapping instead.

## Gotchas met along the way

* A session abandoned inside APL (for example at CATALOG's "WHICH LIBRARY?"
  prompt) is kept by NOS as a recoverable job. The next login for that user gets
  "RECOVERABLE JOB(S) ... ENTER GO TO CONTINUE CURRENT JOB, RELIST, OR DESIRED
  JSN:" instead of a `/` prompt. Type `GO` for a fresh session; the old ones time
  out after 30 minutes. End APL sessions with `)OFF` (logs out) or `)SYSTEM`
  (back to NOS) to avoid leaving them.
* `CATLIST,UN=user` listing empty does not mean the catalog is empty; see above.
* Batch output for the jobs in `jobs/` lands in `NOS2.8.7/LP5xx_C12_E5`.

## Contents

* `workspaces/*.apl`: the three public workspaces as displayed with `∇name[⎕]∇`,
  converted to Unicode APL glyphs. FILESYS's 18 file primitives are locked and
  cannot be displayed; `FIHOW` documents them as wrappers around `⎕FI`.
* `workspaces/transcripts/*.713.txt`: the raw `TT=713` session transcripts.
  `SURVEY.713.txt` has `)FNS`, `)VARS`, `ABSTRACT` and `DESCRIBE` for the four
  workspaces not yet listed in full:
  * `FILES2` (79/06/19): the FILESYS primitives plus convenience functions
    written on top of them (CLIST, FCOPY, CMAP, FMAP, LIB, USERNO, VERIFY, each
    with a HOW function). The APL manual's file chapter uses it for examples.
  * `WSFNS` (81/08/18): general utilities: IF, OVER, MATRIFY, ALPHSORT, MATIOTA,
    COST, a Gauss-Jordan DOMINO, and ⎕AV selector vectors (group GRPAV).
  * `TAPLWS` (78/01/21): file primitives plus CREATE, GENQUAL, ROWNAMES,
    TESTFTOBCD; no ABSTRACT or DESCRIBE, so probably a scratch or test workspace.
  * `TOTALWS` (81/09/16): an interface to the TOTAL database system (SINON,
    READM, WRITD, and so on), with a long DESCRIBE.
* `jobs/`: `apl0cat.job` and `apl1cat.job` (full CATLIST as those users),
  `apl1fix.job` (set AC=Y), `tapecat.job` (catalog the three nos287 release
  tapes mounted on units 1 to 3).
* `tools/nos-session.js USER PASS delayMs "line"...`: log in on port 23 and send
  lines with a fixed pause, no prompt matching. Answer `GO` first if a recoverable
  job exists. Runs without touching the operator port.
* `tools/apl-probe.js`, `tools/apl-fix.js`: run from the `Operator>` prompt as
  `!node <path>` while `node start` holds the port.
* `tools/apl-glyphs.py`, `tools/ws-extract.py`: turn a session log into a
  glyph listing and then into a clean `.apl` file.
