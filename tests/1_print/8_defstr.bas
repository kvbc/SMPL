100 DEF FN F(S) = LCASE$(S)
110 LET X = FN F("TEST")
120 PRINT X         :REM single exp
130 PRINT X; X;     :REM exp twice (   trailing ;)
140 PRINT X; X      :REM exp twice (no trailing ;)