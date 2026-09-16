⍝ Workspace CATALOG from library *APL1 on NOS 2.8.7 (APL2.1.014 97/11/06)
⍝ Listed with )LOAD *APL1 CATALOG and ∇name[⎕]∇ under APL,TT=713; $xx mnemonics converted to glyphs.

⍝ )LOAD *APL1 CATALOG:
⍝   WHICH LIBRARY?
⍝   (TYPE ? FOR HELP)



      ∇CATALOG;A;B;C;⎕IO;ABSTRACT;⎕ENV
[1]    ⎕ENV←1
[2]    ⎕IO←0
[3]    ⎕TRAP L4
[4]   L0:'WHICH LIBRARY?'
[5]    '(TYPE ? FOR HELP)'
[6]    A←⍞
[7]    →(0=⍴A)/0
[8]    →('?'≠1↑A)/L3
[9]    ''
[10]   'THE FOLLOWING LIBRARIES ARE CURRENTLY DEFINED'
[11]   'LIBRARY'
[12]   'NUMBER       USE    ',(20⍴'⌫'),20⍴'_'
[13]   ''
[14]   'APL1     PUBLIC LIBRARY DIRECTORY AND FILE UTILITIES'
[15]   ''
[16]   'TO HAVE A LIST OF WORKSPACE NAMES IN A PARTICULAR'
[17]   'LIBRARY DISPLAYED, TYPE THE COMMAND ''LIST'' FOLLOWED'
[18]   'BY THE LIBRARY NAME. FOR EXAMPLE:'
[19]   ''
[20]   'LIST APL1'
[21]   ''
[22]   ''
[23]   'TO HAVE THE ABSTRACTS OF ALL THE WORKSPACES UNDER'
[24]   'ONE LIBRARY DISPLAYED, TYPE ''ABSTRACT'' FOLLOWED'
[25]   'BY THE LIBRARY NAME. FOR EXAMPLE:'
[26]   ''
[27]   'ABSTRACT APL1'
[28]   ''
[29]   ''
[30]   'TO DISPLAY THE ABSTRACT OF A PARTICULAR WORKSPACE,'
[31]   'ADD THE WORKSPACE NAME. FOR EXAMPLE:'
[32]   ''
[33]   'ABSTRACT APL1 APLNEWS'
[34]   ''
[35]   ''
[36]   'TO EXIT THE PROGRAM, JUST ENTER A CARRIAGE RETURN'
[37]   ''
[38]   →L0
[39]  L3:A←DEBLANK A
[40]   →(∧/'LIST'=4↑A)/L1
[41]   →(~∧/'ABSTRACT'=8↑A)/L2
[42]   A←8↓A
[43]  L2:→(' '∈A←DEBLANK A)/L5
[44]   →(∧/'DESCRIBE'=8↑A)/L7
[45]   CAT A
[46]   →L0
[47]  L1:A←DEBLANK 4↓A
[48]   ''
[49]   DEBLANK,' ',0 ¯10↓B←(B[;8 9]∧.='WS')⌿B←⎕LIB'*',A
[50]   ''
[51]   →L0
[52]  L5:→(0=''⍴⍴B←'ABSTRACT'⎕COPY'*',A)/L6
[53]   'WORKSPACE HAS NO ABSTRACT'
[54]   ''
[55]   →L0
[56]  L6:''
[57]   ABSTRACT
[58]   ''
[59]   B←⎕EX'ABSTRACT'
[60]   →L0
[61]  L7:DESCRIBE
[62]   →L0
[63]  L4:⎕TRAP L4
[64]   →(0≠1↓⎕EXTRACT ⎕ERR[0;])/L0
      ∇
            ∇CAT LIB;A;B;ABSTRACT;⎕IO;⎕ENV
[1]    ⎕IO←⎕ENV←1
[2]    ⎕TRAP L4
[3]    A←0 0⍴''
[4]    →('*'=1↑LIB)/L5
[5]    LIB←'*',LIB
[6]   L5:A←⎕LIB LIB
[7]   L02:A←(A[;9 10]∧.='WS')⌿A
[8]   L0:→(0=''⍴⍴A)/0
[9]    →(0≠''⍴⍴B←'ABSTRACT'⎕COPY LIB,' ',A[1;⍳7])/L1
[10]   A[,1;⍳7]
[11]   ''
[12]   ABSTRACT
[13]   B←⎕EX'ABSTRACT'
[14]   ''
[15]  L1:A←1 0↓A
[16]   →L0
[17]  L4:⎕TRAP L4
[18]   →(0≠1↓⎕EXTRACT ⎕ERR[1;])/L1
      ∇
            ∇ABSTRACT
[1]    'THE FUNCTION CATALOG GIVES THE NAMES OF WORKSPACES'
[2]    'IN THE APL PUBLIC LIBRARIES AND GIVES A SHORT'
[3]    'DESCRIPTION OF EACH.'
      ∇

      ∇DESCRIBE
[1]    'THE CATALOG WORKSPACE GIVES THE USER THE ABILITY TO'
[2]    'EASILY ENQUIRE ABOUT PUBLIC WORKSPACES.'
[3]    ''
      ∇

      ∇SOURCE
[1]    'APL GROUP'
[2]    'UCC'
[3]    'GRAD. RES. CTR.'
[4]    'U. OF MASS.'
[5]    'AMHERST, MA 01002'
      ∇

      ∇Z←DEBLANK A;B;C
[1]    Z←A,' '
[2]    A←' ',A
[3]    Z←1↓((Z=' ')⍲A=' ')/A
      ∇

⍝ )OFF:
⍝   UN=GUEST    LOG OFF  14.12.46. 
⍝   JSN=AADH     SRU-S=      1.162 
⍝   CHARACTERS=    3.774KCHS
⍝   IAF     CONNECT TIME 00.00.37.
⍝   LOGGED OUT.

