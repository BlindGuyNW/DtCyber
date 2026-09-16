⍝ Workspace APLNEWS from library *APL1 on NOS 2.8.7 (APL2.1.014 97/11/06)
⍝ Listed with )LOAD *APL1 APLNEWS and ∇name[⎕]∇ under APL,TT=713; $xx mnemonics converted to glyphs.

⍝ )LOAD *APL1 APLNEWS:
⍝   LAST NEWS ENTERED ON ARCHIVE 
⍝   WHAT IS YOUR DESIRE?
⍝   (TYPE ? FOR HELP)

⍝ )GRPS:
⍝   GRPDOC

⍝ GRPDOC:
⍝   ABSTRACT
⍝   DESCRIBE
⍝   SOURCE

⍝ ABSTRACT:
⍝   NEWS ABOUT THE APL SYSTEM. 

⍝ NEWSF:
⍝   *APL0 NEWSF

      ∇APLNEWS;A;B;C;D;E;F;⎕IO
[1]    ⎕IO←1
[2]    ⎕TRAP L20
[3]    (NEWSF,'/RM')FTIE 1
[4]    A←FREAD 1 0
[5]    B←''⍴⍴A
[6]    ''
[7]    'LAST NEWS ENTERED ON ',A[B;]
[8]   L12:⎕TRAP L12
[9]   L10:''
[10]   'WHAT IS YOUR DESIRE?'
[11]   '(TYPE ? FOR HELP)'
[12]   C←(' '≠C)/C←⍞
[13]   ''
[14]   →('?'≠1↑C)/L9
[15]   'AVAILABLE OPTIONS ARE:'
[16]   '----------------------'
[17]   'LAST        FOR LATEST NEWS ITEM'
[18]   'DIRECTORY   FOR DIRECTORY OF NEWS ITEMS'
[19]   'ALL         FOR ALL NEWS ITEMS'
[20]   'NNNNNNNN    WHERE NNNNNNNN IS AN ITEM NAME'
[21]   '              IN THE DIRECTORY LISTING SUCH AS '
[22]   '              ',A[B;]
[23]   'TERMINALS   LIST OF TERMINAL BRANDS AND LOGIN METHODS.'
[24]   'ENTER CARRIAGE RETURN ONLY TO EXIT PROGRAM'
[25]   →L10
[26]  L9:→(0=⍴C)/L3
[27]   →(∧/'LAS'=3↑C)/L11
[28]   →(∧/'ALL'=3↑C)/L4
[29]   →(~∧/'DIR'=3↑C)/L2
[30]   'ITEM     / DESCRIPTION',(22⍴'⌫'),22⍴'_'
[31]   B←''⍴⍴A
[32]  L0:→(0≥B)/L10
[33]   →(∧/' '=A[B;])/L00
[34]   D←FREAD 1,B
[35]   (1,⍴D)⍴D←A[B;],' / ',⍎,((1=⍴⍴D),2=⍴⍴D)⌿2 5⍴'60↑D D[1;]'
[36]  L00:B←B-1
[37]   →L0
[38]  L2:C←(A∧.=8↑C)/⍳''⍴⍴A
[39]   →(0=⍴C)/L10
[40]   FREAD 1,1↑C
[41]   →L1
[42]  L3:FUNTIE 1
[43]   →0
[44]  L4:B←''⍴⍴A
[45]  L5:→(0≥B)/L3
[46]   ''
[47]   A[,B;]
[48]   ''
[49]   FREAD 1,B
[50]   B←B-1
[51]   →L5
[52]  L11:FREAD 1,''⍴⍴A
[53]   →L12
[54]  L20:⎕ERR
[55]   →L3
      ∇

      ∇Z←DATE;A;B
[1]    A←¯1900 0 0+3↑⎕TS
[2]    A←100⊥A
[3]    Z←1 1 0 1 1 0 1 1\6 0⍕A
[4]    Z[3 6]←'/'
      ∇

      ∇DD
[1]    'NEWSF:NEWSF'FTIE 1
[2]    B←FREAD 1 0
[3]    C←(B∧.=8↑A)/⍳''⍴⍴B
[4]    →(0=⍴C)/L1
[5]    B←((¯1+C)↑B),C↓B
[6]    B FWRITE 1 0
[7]    FDELETE 1,C
[8]   L0:FUNTIE 1
[9]    →0
[10]  L1:'NOT FOUND'
[11]   →L0
      ∇

      ∇DELETE A;B;C;⎕IO;I;NAME
[1]    ⎕IO←1
[2]    NEWSF FTIE I←1+⌈/0,FNUMS
[3]    B←FREAD I,0
[4]    C←(B∧.=8↑A)/⍳''⍴⍴B
[5]    →(0=⍴C)/L1
[6]    B←(C≠⍳''⍴⍴B)/[1]B ⍝ UPDATE INDEX
[7]    B FWRITE I,0
[8]    FRDEL 1,C ⍝  DELETE INFO RECORD
[9]   L2:⍞←'ENTER NEW FILE NAME: '
[10]   →(0=1↑⍴NAME←(∨\NAME≠' ')/NAME←⍞)/L3 ⍝  IF CARRIAGE RETURN
[11]   →(7<1↓⍴NAME)/L3 ⍝ IF FILENAME GREATER THAN 7 CHARS.
[12]   →(∧/'NEWSF'=5↑NAME)/L3
[13]   FUNTIE I
[14]   NEWSF FCOPY NAME,'/DA PU'
[15]   'TYPE THE FOLLOWING NOW:'
[16]   '      )SYSTEM'
[17]   'PURGE,NEWSF'
[18]   'CHANGE,NEWSF=',NAME,'/M=RM'
[19]   'APL,TT=...,WS=APLNEWS,UN=1QCB'
[20]   →0
[21]  L3:'ILLEGAL FILE NAME'
[22]   →L2
[23]  L0:FUNTIE I
[24]   →0
[25]  L1:'NOT FOUND'
[26]   →L0
      ∇

      ∇DESCRIBE
[1]    'DESCRIBE ACCESSES A FILE WHICH CONTAINS'
[2]    'ITEMS OF INTEREST TO USERS OF APL. TYPE:'
[3]    '      APLNEWS'
[4]    'TO ENTER A DIALOG WITH THE APLNEWS WORKSPACE.'
[5]    ''
[6]    'INCLUDED ARE FUNCTIONS FOR UPDATING THE NEWS'
[7]    'ITEMS. THESE FUNCTIONS WORK ONLY FOR THOSE WITH'
[8]    'WRITE ACCESS TO THE NEWSF FILE. THE FUNCTIONS ARE:'
[9]    '      GET ''NNNNNNNN''    TO GET A NEWS ITEM INTO THE MAT ARRAY'
[10]   '      POST ''NNNNNNNN''   TO ADD OR REPLACE A NEWS ITEM USING THE MAT ARRAY'
[11]   '      EDIT              TO EDIT THE MAT ARRAY'
[12]   ''
      ∇

      ∇EDIT;O;I;C;V;S;T;D;L;X;W;Y;B;N;E;⎕ENV;⎕IO;⎕ERR;⎕CT
[1]   ONE:C←⎕ENV←⎕IO←E←D←0
[2]    I←1
[3]    V←⍳C←''⍴⍴MAT
[4]    ⎕CT←5E¯11
[5]   L45:⎕TRAP L28
[6]    ⎕ERR←3 100⍴' '
[7]    ⍞←(S←6⌈⍴S)↑S←'[',(⍕C),'] '
[8]    O←S↓⍞
[9]   L55:→(T∧⎕IO=⍴O←(-T←∨/'∇⍫'=S←¯1↑O)↓O)/L26
[10]   D←D∨T
[11]  L56:B←(S←((~≠\''''=O)⍲'⋄'=O)⍳⎕IO)↑O
[12]   O←(S+1)↓O
[13]   →(∨/' '≠B)/L7
[14]   MAT←(S←V≠C)⌿MAT
[15]   V←S/V
[16]   →E←L10
[17]  L65:→(⎕IO=⍴B)/L5
[18]  L7:→('[/'=ONE↑B←((B≠' ')⍳ONE)↓B)/L12,L23
[19]   MAT←(S[E←ONE]↑B)⍪(S←(⎕IO,⍴B)⌈⍴MAT)↑MAT
[20]   V←C,V
[21]  L10:C←C+I
[22]  L5:→(0≠⍴O)/L56
[23]   →D/L26
[24]   ⍞←(S←6⌈⍴S)↑S←'[',(⍕C),'] '
[25]   O←S↓⍞
[26]   →L55
[27]  L12:→('⎕'=ONE↑B←ONE↓B)/L16
[28]   →(2≠⍴S←|⎕EXTRACT B)/L27
[29]   C←(⌊0.5+10000×S[ONE])÷10000
[30]   I←0.0001 0.001 0.01 0.1 1[(1E¯8<0.001 0.01 0.1 1|C)⍳ONE]
[31]   I←I+I=⎕IO
[32]   →(']'=ONE↑B←S[⎕IO]↓B)/L6
[33]   →('⎕'≠ONE↑B)/L27
[34]   S←|⎕EXTRACT B←ONE↓B
[35]   →(']'≠ONE↑B←S[⎕IO]↓B)/L27
[36]   →(2 1=⍴S)/L13,L15
[37]   →L27
[38]  L13:→((⍴V)≤T←V⍳C)/L5
[39]   X←ONE≠B←(-((⊖B)≠' ')⍳ONE)↓B←((6⌈⍴X)↑X←'[',(⍕C),'] '),MAT[T;]
[40]   →(⎕IO=S[ONE])/L14
[41]   B
[42]   ⍞←(⎕PW|S[ONE]-ONE)⍴' '
[43]   B←(X←'/'≠S←(⍴B)↑⍞)/B
[44]   X←(X<18)×(5×(X-9)×~S)+X×S←10>X←'0123456789ABCDEFGH'⍳S←X/S
[45]   B←(X←(⍳S++/X)∈(⍳S←⍴X)++\X)\B
[46]  L14:⍞←B,((⍴B)-X⍳⎕IO)⍴'⌫'
[47]   B←(B,S↑X)[(⍳S)+S×' '=B←(S←(⍴B)⌈⍴X←⍞)↑B]
[48]   →L7
[49]  L15:→((⍴V)≤S←V⍳C)/L6
[50]   (ONE,⍴T)⍴T←((6⌈⍴T)↑T←'[',(⍕C),'] '),MAT[S;]
[51]   →L6
[52]  L16:C←(I←ONE)+⌈⌈/V
[53]   →(2 1=⍴T←|⎕EXTRACT B←ONE↓B)/L18,L17
[54]   →L27
[55]  L17:X←⎕IO
[56]   →L19
[57]  L18:X←T[ONE]
[58]  L19:→(']'≠ONE↑B←T[⎕IO]↓B)/L27
[59]   →(⎕IO=E)/L20
[60]   MAT←MAT[S[T←⍋V←V[S←((⍳⍴V)=V⍳V)/⍳⍴V]];]
[61]   V←V[T]
[62]  L20:S←¯1⌈¯1+(V≥X)⍳ONE
[63]  L21:→((⍴V)≤S←S+ONE)/L22
[64]   (ONE,⍴T)⍴T←((6⌈⍴T)↑T←'[',(⍕V[S]),'] '),MAT[S;]
[65]   →L21
[66]  L22:'      ∇'
[67]  L6:B←ONE↓B
[68]   →L65
[69]  L23:→((⍴V)≤S←V⍳C)/L5
[70]   T←ONE↑ONE↓W←B
[71]   B←(-((⊖B)≠' ')⍳ONE)↓B←((6⌈⍴X)↑X←'[',(⍕C),']'),B←MAT[S;]
[72]   →(⎕IO=(⍴W←S↑W)+⍴Y←(Y⍳T)↑Y←(ONE+S←W⍳T)↓W←2↓W)/L24
[73]   →(S<T←((S←⍴B)×⎕IO=⍴W)⌈T←(∧⌿(⍳⍴W)⊖(W∘.=B),~ONE)⍳ONE)/L25
[74]   ⎕←B←(T↑B),Y,(T+⍴W)↓B
[75]   →L7
[76]  L24:⍞←B
[77]   B←B,(⍴B)↓⍞
[78]   →L7
[79]  L25:'13: PHRASE NOT FOUND'
[80]   →L5
[81]  L26:→(⎕IO=E)/0
[82]   V←V[T←⍋V←V[S←((⍳⍴V)=V⍳V)/⍳⍴V]]
[83]   MAT←MAT[S[T];]
[84]   →0
[85]  L27:'12: DEFN ERROR'
[86]   B←(D←⎕IO)⍴⎕←B
[87]   →L5×××/⍴MAT
[88]  L29:C←(I←ONE)+⌈⌈/V
[89]   →L45
[90]  L28:→(10≠ONE↓⎕EXTRACT,⎕←⎕ERR[,⎕IO;])/L29
      ∇

      ∇D ERRATA C;A;B;E;F;MAT
[1]    GET'ERRATA'
[2]    MAT←(1 1 0,(¯2+''⍴⍴MAT)⍴1 1)⍀MAT
[3]    C←(5 0 6 0⍕D),'      ',C
[4]    MAT←((⍴MAT)⌈(0,⍴C))↑MAT
[5]    C←(1↓⍴MAT)↑C
[6]    MAT[3;]←C
[7]    MAT[,3;]
[8]    'OK?'
[9]    →(0≠⍴⍞)/0
[10]   POST'ERRATA'
[11]   'DONE'
      ∇

      ∇D FIX A;B;C;E;F;MAT
[1]    →(0≠⎕NC'D')/L00
[2]    D←DATE
[3]   L00:GET'BUGLIST'
[4]    A←2 0⍕A
[5]    B←(MAT[;2 3]∧.=A)/⍳''⍴⍴MAT
[6]    →(0≠⍴B)/L1
[7]    'HUH!'
[8]    →0
[9]   L1:MAT[B;22+⍳8]←D
[10]   MAT[,B;]
[11]   'OK?'
[12]   →(0≠⍴⍞)/0
[13]   POST'BUGLIST'
[14]   'DONE'
      ∇

      ∇GET A;B;C;D;⎕IO
[1]    ⎕IO←1
[2]    (NEWSF,'/RM') FTIE 1
[3]    B←FREAD 1 0
[4]    C←(B∧.=8↑A)/⍳''⍴⍴B
[5]    →(0=⍴C)/L1
[6]    MAT←FREAD 1,C
[7]   L0:FUNTIE 1
[8]    →0
[9]   L1:'NOT FOUND'
[10]   →L0
      ∇

      ∇NEW
[1]    MAT←0 0⍴''
[2]    EDIT
      ∇

      ∇POST A;B;C;D;⎕IO
[1]    ⎕IO←1
[2]    NEWSF FTIE I←1+⌈/0,FNUMS
[3]    B←FREAD I,0
[4]    C←(B∧.=8↑A)/⍳''⍴⍴B
[5]    →(0=⍴C)/L1
[6]    MAT FWRITE I,C
[7]   L0:FUNTIE I
[8]    →0
[9]   L1:'NOT FOUND IN FILE'
[10]   'DO YOU WISH TO ADD?'
[11]   'ANSWER YES OR NO'
[12]   D←(' '≠D)/D←⍞
[13]   →(~∧/'YES'=3↑D)/L0
[14]   B←B⍪8↑A
[15]   B FWRITE I,0
[16]   MAT FWRITE I,''⍴⍴B
[17]   →L0
      ∇

      ∇RELEASE
[1]    ⎕←NEWSF←'*APL0 NEWSF'
[2]    ⎕SAVE'APLNEWS'
      ∇

      ∇D REPORT C;A;B;E;F;MAT
[1]    GET'BUGLIST'
[2]    →(0≠⎕NC'D')/L1
[3]    D←DATE
[4]   L1:MAT←(1 1 0,(¯2+''⍴⍴MAT)⍴1 1)⍀MAT
[5]    C←(3 0⍕1+1↓A←⎕EXTRACT B←MAT[4;2 3]),'     ',D,(18⍴' '),C
[6]    MAT←((⍴MAT)⌈(0,⍴C))↑MAT
[7]    C←(1↓⍴MAT)↑C
[8]    MAT[3;]←C
[9]    MAT[,3;]
[10]   'OK?'
[11]   →(0≠⍴⍞)/0
[12]   POST'BUGLIST'
[13]   'DONE'
      ∇

      ∇SOURCE
      ∇

      ∇TEST
[1]    ⎕←NEWSF←'*0QCB NEWSF'
[2]    0 ⎕SAVE'APLNEWS'
      ∇

⍝ )OFF:
⍝   UN=GUEST    LOG OFF  14.15.15. 
⍝   JSN=AADJ     SRU-S=      1.302 
⍝   CHARACTERS=   10.385KCHS
⍝   IAF     CONNECT TIME 00.01.15.
⍝   LOGGED OUT.

