(scope 12
  (decl
    (decl-var 'x'
      (type int32)
    )
  )
  (code
    (ref 'x')
    (st-for
      (pre
        (st-assign
          (ref 'i')
          (var 'x')
        )
      )
      (condition
        (var 'i')
      )
      (post
        (st-assign
          (var 'i')
          (expr-plus
            (var 'i')
            (var 'x')
          )
        )
      )
      (scope 16
        (decl
          (decl-var 'i'
            (type int32)
          )
        )
        (code
          (scope 28
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'i')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre
        (st-assign
          (ref 'i')
          (var 'x')
        )
      )
      (condition)
      (post)
      (scope 33
        (decl
          (decl-var 'i'
            (type int32)
          )
        )
        (code
          (scope 39
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'i')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre
        (st-assign
          (ref 'i')
          (var 'x')
        )
      )
      (condition)
      (post
        (st-assign
          (var 'i')
          (expr-plus
            (var 'i')
            (var 'x')
          )
        )
      )
      (scope 44
        (decl
          (decl-var 'i'
            (type int32)
          )
        )
        (code
          (scope 55
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'i')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition)
      (post
        (st-expr
          (var 'x')
        )
      )
      (scope 60
        (decl)
        (code
          (scope 63
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'x')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition
        (var 'x')
      )
      (post
        (st-assign
          (var 'x')
          (expr-plus
            (var 'x')
            (var 'x')
          )
        )
      )
      (scope 68
        (decl)
        (code
          (scope 75
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'x')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition
        (var 'x')
      )
      (post)
      (scope 80
        (decl)
        (code
          (scope 82
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'x')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition)
      (post)
      (scope 87
        (decl)
        (code
          (scope 88
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'x')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition
        (st-expr
          (var 'x')
        )
      )
      (post)
      (scope 93
        (decl)
        (code
          (scope 96
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'x')
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition)
      (post)
      (scope 101
        (decl)
        (code
          (scope 102
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'x')
              )
            )
          )
        )
      )
    )
  )
)
