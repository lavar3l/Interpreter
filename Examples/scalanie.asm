WEKTORA     DS      5*INTEGER
WEKTORB     DS      5*INTEGER
WEKTORC     DS      10*INTEGER
JEDEN       DC      INTEGER(1)
CZTERY      DC      INTEGER(4)
DZIESIEC    DC      INTEGER(5)
ZERO        DC      INTEGER(0)
WCZYTAJA    READ    WEKTORA(5)
WCZYTAJB    READ    WEKTORB(5)
            LA      1, WEKTORA
            LA      2, WEKTORB
            L       3, 0(1)
            L       4, 0(2)
            L       5, ZERO
            L       6, ZERO
            LA      7, WEKTORC
POROWNAJ    CR      3, 4
            JP      MNIEJ.W.B
            JZ      MNIEJ.W.A
            JN      MNIEJ.W.A
MNIEJ.W.A   ST      3, 0(7)
            A       7, CZTERY
            A       1, CZTERY
            L       3, 0(1)
            A       5, JEDEN
            C       5, DZIESIEC
            JN      POROWNAJ
            JZ      KONIECA
MNIEJ.W.B   ST      4, 0(7)
            A       7, CZTERY
            A       2, CZTERY
            L       4, 0(2)
            A       6, JEDEN
            C       6, DZIESIEC
            JN      POROWNAJ
            JZ      KONIECB
KONIECA     C       6, DZIESIEC
            JZ      KONIEC
            ST      4, 0(7)
            A       7, CZTERY
            A       2, CZTERY
            A       6, JEDEN
            L       4, 0(2)
            J       KONIECA
KONIECB     C       5, DZIESIEC
            JZ      KONIEC
            ST      3, 0(7)
            A       7, CZTERY
            A       1, CZTERY
            A       5, JEDEN
            L       3, 0(1)
            J       KONIECB
KONIEC      WRITE   WEKTORC(10)