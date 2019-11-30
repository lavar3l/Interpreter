WSP_A               DS      INTEGER
WSP_B               DS      INTEGER
WSP_C               DS      INTEGER
WYNIK               DS      INTEGER
ZERO                DC      INTEGER(0)
JEDEN               DC      INTEGER(1)
CZTERY              DC      INTEGER(4)
                    READ    WSP_A
                    READ    WSP_B
                    READ    WSP_C
                    L       1, WSP_B
                    MR      1, 1
                    L       2, CZTERY
                    M       2, WSP_A
                    M       2, WSP_C
                    SR      1, 2
                    JP      DWA_ROZWIAZANIA
                    JZ      JEDNO_ROZWIAZANIE
                    JN      ZERO_ROZWIAZAN
DWA_ROZWIAZANIA     L       3, JEDEN
                    AR      3, 3
                    ST      3, WYNIK
                    J       KONIEC_PROGRAMU
JEDNO_ROZWIAZANIE   L       3, JEDEN
                    ST      3, WYNIK
                    J       KONIEC_PROGRAMU
ZERO_ROZWIAZAN      L       3, ZERO
                    ST      3, WYNIK
                    J       KONIEC_PROGRAMU
KONIEC_PROGRAMU     C       1, 1