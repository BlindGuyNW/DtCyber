# Convert a NOS APL TT=713 session log into two listings: raw mnemonic text and Unicode APL glyphs.
# usage: python apl-glyphs.py session.log out-raw.txt out-glyph.txt
# Mnemonics per Table C-2 of the CDC APL Version 2 Reference Manual (60454000F).
import re, sys
M = {
 'AN':'∧','OR':'∨','ND':'⍲','NR':'⍱','LE':'≤','NE':'≠','GE':'≥','UG':'⍋','DG':'⍒','TA':'↑','DR':'↓',
 'IS':'←','GO':'→','LP':'⍝','LD':'⍫','DL':'∇','MN':'⌊','MX':'⌈','ML':'×','DV':'÷','DI':'¨','NG':'¯',
 'DT':'∆','RK':'⊣','LK':'⊢','CI':'○','LG':'⍟','RU':'⌽','RT':'⊖','TP':'⍉','DM':'⋄','DU':'⍙','IO':'⍳',
 'RO':'⍴','BV':'⊥','RP':'⊤','IB':'⌶','ID':'⊂','IN':'⊃','IX':'∩','UN':'∪','NL':'∘','QD':'⎕','QP':'⍞',
 'BT':'⍀','SM':'⌿','XD':'⌹','OM':'⍵','AL':'⍺','EP':'∈','EV':'⍎','FM':'⍕','CN':'⍪','BC':'⍫',
 'OB':'[','BS':chr(92),'CB':']','CX':'^','UL':'_','AC':'‾','TL':'~','LB':'{','MD':'|','RB':'}',
 'CL':':','SC':';','LT':'<','EQ':'=','GT':'>','QU':'?','AT':'@','BJ':'⌫',
}
for c in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
    M[c + c] = c.lower()          # $AA..$ZZ are the underscored (lowercase) letters
src, raw_out, gl_out = sys.argv[1:4]
lines = []
for line in open(src, encoding='utf-8', errors='replace'):
    line = re.sub(r'\x1b\[[0-9;]*[A-Za-z]', '', line)
    m = re.match(r'^.*? ([RS]) : (.*)$', line.rstrip('\n'))
    if not m: continue
    d, text = m.groups()
    text = text.replace('<0d><0a>', '').replace('<0d>', '').replace('<07>', '')
    if d == 'S': text = '> ' + text
    lines.append(text)
raw = '\n'.join(lines) + '\n'
open(raw_out, 'w', encoding='utf-8').write(raw)
gl = re.sub(r'\$([A-Z][A-Z0-9])', lambda m: M.get(m.group(1), m.group(0)), raw)
open(gl_out, 'w', encoding='utf-8').write(gl)
print(len(lines), 'lines')
