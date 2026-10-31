$ Bulk-only NAS fragment: no SOL/CEND/BEGIN required.
GRID         101              0.      0.      0.
RBE3,201,,101,123,1.0,123,102,103,+A
+A,104,105
RBE2         202     101  123456     102     103     104     105     106+B      
+B           107     108
$ Blank-first-field fixed continuation follows the next card.
SET1          10       1       2       3       4       5       6       7
               8       9      10
GRID	102		1.25-3	-.21+5	1.D+2
GRID     ,103,,1.0,2.0,3.0 $ delimiter in column 10
BEGIN SUPER=10
GRID,1,,0.,0.,0.
ENDDATA
