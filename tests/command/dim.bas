10 LET X = 3
20 LET Y = 5
30 LET Z = 7
40 DIM A(X,Y,Z)
50 LET C = 1
60 FOR I=1 TO X
70     FOR J=1 TO Y
80         FOR K=1 TO Z
90             A(I,J,K) = C
91             C = C + 1
92         NEXT K
93     NEXT J
94 NEXT I
11 FOR I=1 TO X
12     FOR J=1 TO Y
13         FOR K=1 TO Z
14             PRINT A(I,J,K)
15         NEXT K
16     NEXT J
17 NEXT I