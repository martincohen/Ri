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
      (expr-and
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-or
        (var 'x')
        (var 'y')
      )
    )
    (st-assign
      (var 'a')
      (expr-not
        (var 'x')
      )
    )
  )
)
