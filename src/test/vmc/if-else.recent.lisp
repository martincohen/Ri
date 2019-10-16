func0:
    (t3 = t1 == 1)
    (if t3 != 0 then (goto _1) else (goto _2))
_1:
    (t1 = 3)
    (goto _3)
_2:
    (t1 = 4)
_3:
    (t3 = t1 == 1)
    (if t3 != 0 then (goto _4) else (goto _5))
_4:
    (t1 = 3)
    (goto _6)
_5:
    (t3 = t1 == 2)
    (if t3 != 0 then (goto _7) else (goto _8))
_7:
    (t1 = 4)
_6:
_8:
    (t3 = t1 == 1)
    (if t3 != 0 then (goto _9) else (goto _10))
_9:
    (t1 = 3)
    (goto _11)
_10:
    (t3 = t1 == 2)
    (if t3 != 0 then (goto _12) else (goto _13))
_12:
    (t1 = 4)
    (goto _14)
_13:
    (t1 = 5)
_11:
_14:
    (ret t1)
