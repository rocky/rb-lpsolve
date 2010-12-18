/* Objective function */
Maximize: +3 money +3 C3 +3 C4;

speed: +4 money +C3 +2 C4 >= 3;
+2 money +4 C3 +2 C4 = 8;

/* Variable bounds */
money >= 2;
C3 <= 5.3;
