WEKTOR	DC		10*INTEGER(10)
OSTEL	DC		INTEGER(8)
JEDEN	DC		INTEGER(1)
ZERO	DC		INTEGER(0)
CZT		DC		INTEGER(4)
WYNIK	DS		INTEGER
		READ	WEKTOR(10)
START	LA		1, WEKTOR
		SR		2, 2
		L		3, 0(1)
		L		6, ZERO
		CR		3, 6
		JP		FALSZ
		JN		FALSZ
		A		1, CZT
		L		3, 0(1)
		L		6, JEDEN
		CR		3, 6
		JP		FALSZ
		JN		FALSZ
		A		1, CZT
PETLA	LR		7, 1
		L		3, 0(1)
		S		7, CZT
		L		4, 0(7)
		S		7, CZT
		L		5, 0(7)
		LR		8, 4
		AR		8, 5
		CR		3, 8
		JP		FALSZ
		JN		FALSZ
NAST	A		1, CZT
		A		2, JEDEN
		C		2, OSTEL
		JN		PETLA
PRAWDA	L		1, JEDEN
		J		KONIEC
FALSZ	L		1, ZERO
KONIEC	ST		1, WYNIK