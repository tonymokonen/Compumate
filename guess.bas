1 Prt"GUESS"
2 Prt
3 Prt"GUESS THE   NUMBER"
4 Prt"BETWEEN 1   AND 100"
5 LetB=99999
6 LetN=Rnd[99],N=N+1,G=1
7 Prt"GUESS",G
8 InpA
9 IfA=NThenGoto14
10 IfA<NThenPrt"TOO LOW"
11 IfA>NThenPrt"TOO HIGH"
12 LetG=G+1
13 Goto7
14 Prt"YOU WIN"
15 IfG<BThenGoto20
16 Prt"BEST SCORE",B,"BY",Q$
17 Inp"PLAY AGAIN  1=YES",A
18 IfA=1ThenGoto6
19 Goto23
20 Inp"NEW BEST    ENTER NAME",Q$
21 LetB=G
22 Goto17
23 Prt"BYE"
