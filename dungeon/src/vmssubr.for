C Operating system dependent subroutines -- NOS 2 / FTN5 version.
C
C COPYRIGHT 1980, 1990, INFOCOM COMPUTERS AND COMMUNICATIONS, CAMBRIDGE MA.
C ALL RIGHTS RESERVED, COMMERCIAL USAGE STRICTLY PROHIBITED
C WRITTEN BY R. M. SUPNIK
C
C The VMS original used SYS$NUMTIM and RAN.  Here the FTN5 CLOCK and
C DATE subroutines supply the time and date as 10 character strings
C ("hh.mm.ss." and "yy/mm/dd.", possibly with a leading blank), and the
C RANF/RANSET intrinsics supply random numbers.
C
C ITIME - Return system time in component form
C
	SUBROUTINE ITIME (H, M, S)
	IMPLICIT INTEGER (A-Z)
	CHARACTER*10 T

	CALL CLOCK (T)
	CALL DIGIT3 (T, H, M, S)
	RETURN

	END
C
C IDATE - Return system date in component form (month, day, year)
C
	SUBROUTINE IDATE (M, D, Y)
	IMPLICIT INTEGER (A-Z)
	CHARACTER*10 T

	CALL DATE (T)
	CALL DIGIT3 (T, Y, M, D)
	RETURN

	END
C
C DIGIT3 - Pick the first three numbers out of a string
C
	SUBROUTINE DIGIT3 (STR, A, B, C)
	IMPLICIT INTEGER (A-Z)
	CHARACTER*(*) STR
	INTEGER V(3)

	V(1) = 0
	V(2) = 0
	V(3) = 0
	N = 0
	CUR = -1
	DO 100 I = 1, LEN(STR)+1
	  D = -1
	  IF (I .LE. LEN(STR)) D = INDEX('0123456789', STR(I:I)) - 1
	  IF (D .GE. 0) THEN
	    IF (CUR .LT. 0) CUR = 0
	    CUR = CUR*10 + D
	  ELSE IF (CUR .GE. 0) THEN
	    N = N + 1
	    IF (N .LE. 3) V(N) = CUR
	    CUR = -1
	  END IF
100	CONTINUE
	A = V(1)
	B = V(2)
	C = V(3)
	RETURN

	END
C
C RND - Return a random integer mod n
C
	INTEGER FUNCTION RND (N)
	IMPLICIT INTEGER (A-Z)

	RND = RANF()*FLOAT(N)
	RETURN

	END
C
C INIRND - Initialize random number seed
C
	SUBROUTINE INIRND (LOW, HIGH)
	IMPLICIT INTEGER (A-Z)

	RNSEED = OR(SHIFT(HIGH,16)+LOW, 1)
	CALL RANSET (RNSEED)
	RETURN

	END
