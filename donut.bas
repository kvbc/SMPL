11 LET A = 0.07          
12 LET B = 0.02          
13 LET C = 75/2       
14 LET D = 180/2
15 LET E = 1           
16 LET F = 2           
17 LET G = 5               
18 LET H = C*G*3/(8*(E+F)) 
19 LET I = 1
20 LET J = 0
26 DIM O(C,D)    
27 DIM P(C,D)
 1 REM ------------------------------------- render_frame (I, J)
22 LET K = COS(I)    
23 LET L = SIN(I)    
24 LET M = COS(J)   
25 LET N = SIN(J)   
28 FOR c=1 TO C
29     FOR d=1 TO D
30         O(c,d)=" "
31         P(c,d)=0
32     NEXT d
33 NEXT c
34 FOR a=0 TO 2*PI()-1 STEP A
35      LET b = COS(a)
36      LET c = SIN(a)
37      FOR d=0 TO 2*PI()-1 STEP B
38          LET e = COS(d)
39          LET f = SIN(d)
40          LET g = F+E*b
41          LET h = E*c
42          LET i = g*(M*e+L*N*f)-h*K*N
43          LET j = g*(N*e-L*M*f)+h*K*M
44          LET k = G+K*g*f+h*L
45          LET l = 1/k
46          LET m = INT(C/2+H*l*i) + 1
47          LET n = INT(D/2-H*l*j) + 1
48          LET o = e*b*N-K*b*f-L*c+M*(K*c-b*L*f)
49          IF o <= 0 THEN GOTO 53
50              IF l <= P(m,n) THEN GOTO 53
51                  P(m,n) = l
52                  O(m,n) = AT(".,-~:;=!*#$@", o*8)
53      NEXT d
54 NEXT a
55 FOR c=1 TO C
56     FOR d=1 TO D
57         PRINT O(c,d);
58     NEXT d
59     PRINT
60 NEXT c
 2 REM ------------------------------------- rotation
61 I = I + 0.2
62 GOTO 22