# Turn a glyph-converted session transcript (from apl-glyphs.py) into a clean workspace listing.
# usage: python ws-extract.py WSNAME transcript-glyph.txt out.apl
import sys, re
ws, src, dst = sys.argv[1:4]
lines = open(src, encoding='utf-8').read().split('\n')
# group into (command, response lines)
groups, cmd, resp = [], None, []
for l in lines:
    if l.startswith('> '):
        if cmd is not None: groups.append((cmd, resp))
        cmd, resp = l[2:], []
    elif cmd is not None:
        resp.append(l)
groups.append((cmd, resp))
out = [f'⍝ Workspace {ws} from library *APL1 on NOS 2.8.7 (APL2.1.014 97/11/06)',
       '⍝ Listed with )LOAD *APL1 %s and ∇name[⎕]∇ under APL,TT=713; $xx mnemonics converted to glyphs.' % ws, '']
locked = []
for cmd, resp in groups:
    resp = [r for r in resp if r.strip() != '' and r.strip() != cmd.strip()]
    m = re.match(r'^∇(\w+)\[⎕\]∇$', cmd)
    if m:
        if any('DEFN ERROR' in r for r in resp) and not any(r.startswith('      ∇') for r in resp):
            locked.append(m.group(1)); continue
        out.extend(resp); out.append('')
    elif cmd.startswith(')') or cmd in ('GRPDOC', 'GRPPRIM', 'GRPCONV', 'ABSTRACT', 'DESCRIBE', 'NEWSF'):
        out.append('⍝ ' + cmd + ':')
        out.extend('⍝   ' + r for r in resp)
        out.append('')
if locked:
    out.append('⍝ Locked functions (12: DEFN ERROR on display): ' + ' '.join(locked))
open(dst, 'w', encoding='utf-8').write('\n'.join(out) + '\n')
print(dst, len(out), 'lines; locked:', len(locked))
