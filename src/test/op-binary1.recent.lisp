(scope 12
  (decl
    (decl-var 'a'
      (type int32)
    )
    (decl-var 'x'
      (type int32)
    )
    (decl-var 'y'
      (type int32)
    )
  )
  (code
    (ref 'x')
    (ref 'y')
    (ref 'a')
    (st-assign
      (var 'a')
      (expr-add
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-band
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-bxor
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-bshr
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-bshl
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-bneg
        (var 'x')
      )
    )
  )
)
