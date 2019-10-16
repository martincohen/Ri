func0:
    (t3 = t1 == 1)
    (if t3 != 0 then (goto _1) else (goto _2))
_1:
    (t1 = 2)
_2:
    (t3 = t1 == 1)
    (if t3 != 0 then (goto _3) else (goto _4))
_3:
    (t1 = 2)
_4:
    (ret t1)
