 1 REM BASIC logical operator tests
 2 REM OPERATOR: OR
10 IF 0 OR  0 THEN GOSUB 92 ELSE GOSUB 94
20 IF 0 OR  1 THEN GOSUB 92 ELSE GOSUB 94
30 IF 1 OR  0 THEN GOSUB 92 ELSE GOSUB 94
40 IF 1 OR  1 THEN GOSUB 92 ELSE GOSUB 94
50 PRINT "----------"
 3 REM OPERATOR: AND
60 IF 0 AND 0 THEN GOSUB 92 ELSE GOSUB 94
70 IF 0 AND 1 THEN GOSUB 92 ELSE GOSUB 94
80 IF 1 AND 0 THEN GOSUB 92 ELSE GOSUB 94
90 IF 1 AND 1 THEN GOSUB 92 ELSE GOSUB 94
91 END
 3 REM true
92 PRINT 1
93 RETURN
 4 REM false
94 PRINT 0
95 RETURN