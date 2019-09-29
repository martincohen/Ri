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
      (expr-sub
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-mul
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-div
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-mod
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-negative
        (var 'x')
      )
    )
    (st-assign
      (var 'a')
      (expr-negative
        (var 'y')
      )
    )
  )
)
