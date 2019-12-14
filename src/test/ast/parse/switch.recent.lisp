(scope 27
  (code
    (decl
      (spec-var 'a'
        (id 'int32')
      )
    )
    (st-switch
      (pre)
      (expr
        (id 'a')
      )
      (scope 31
        (code
          (scope 34
            (code)
          )
        )
      )
    )
    (st-switch
      (pre
        (decl
          (spec-var 'b'
            (id 'int32')
          )
        )
      )
      (expr
        (id 'a')
      )
      (scope 36
        (code
          (scope 41
            (code)
          )
        )
      )
    )
    (st-switch
      (pre)
      (expr
        (id 'a')
      )
      (scope 43
        (code
          (scope 46
            (code
              (st-switch-case
                (const 1
                  (spec-type-number 'untyped-int')
                )
              )
              (st-switch-case
                (const 2
                  (spec-type-number 'untyped-int')
                )
              )
              (st-switch-case
                (expr-add
                  (id 'a')
                  (const 1
                    (spec-type-number 'untyped-int')
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-switch
      (pre)
      (expr
        (id 'a')
      )
      (scope 56
        (code
          (scope 59
            (code
              (st-switch-case
                (const 1
                  (spec-type-number 'untyped-int')
                )
              )
              (st-break)
              (st-switch-case
                (const 2
                  (spec-type-number 'untyped-int')
                )
              )
              (st-break)
              (st-switch-default)
              (st-break)
            )
          )
        )
      )
    )
    (st-switch
      (pre)
      (expr
        (id 'a')
      )
      (scope 69
        (code
          (scope 71
            (code
              (st-switch-case
                (const 1
                  (spec-type-number 'untyped-int')
                )
              )
              (st-break)
            )
          )
        )
      )
    )
    (st-switch
      (pre
        (decl
          (spec-var 'b'
            (id 'int32')
          )
        )
      )
      (expr
        (id 'a')
      )
      (scope 76
        (code
          (scope 81
            (code
              (st-switch-case
                (const 1
                  (spec-type-number 'untyped-int')
                )
              )
              (st-break)
              (st-switch-default)
              (st-break)
            )
          )
        )
      )
    )
  )
)
