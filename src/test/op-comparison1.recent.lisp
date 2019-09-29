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
      (expr-gt
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-gt-eq
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-lt
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-lt-eq
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-eq
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-not-eq
        (var 'x')
        (var 'y')
      )
    )
  )
)
