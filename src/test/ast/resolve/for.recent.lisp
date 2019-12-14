(scope 27
  (decl
    (spec-var 'x'
      (value-type
        (spec-type-number 'int32')
      )
    )
  )
  (code
    (decl
      (spec-var 'x'
        (value-type
          (spec-type-number 'int32')
        )
      )
    )
    (st-for
      (pre
        (st-assign
          (decl
            (spec-var 'i'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
      )
      (condition
        (expr-eq
          (value-var
            (spec-var 'i'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (const 0
            (spec-type-number 'untyped-int')
          )
        )
      )
      (post
        (st-assign
          (value-var
            (spec-var 'i'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (expr-add
            (value-var
              (spec-var 'i'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
            (value-var
              (spec-var 'x'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
        )
      )
      (scope 31
        (decl
          (spec-var 'i'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (code
          (scope 45
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'i'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-for
      (pre
        (st-assign
          (decl
            (spec-var 'i'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
      )
      (condition)
      (post)
      (scope 50
        (decl
          (spec-var 'i'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (code
          (scope 56
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'i'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-for
      (pre
        (st-assign
          (decl
            (spec-var 'i'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
      )
      (condition)
      (post
        (st-assign
          (value-var
            (spec-var 'i'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (expr-add
            (value-var
              (spec-var 'i'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
            (value-var
              (spec-var 'x'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
        )
      )
      (scope 61
        (decl
          (spec-var 'i'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (code
          (scope 72
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'i'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
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
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
      )
      (scope 77
        (code
          (scope 80
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition
        (expr-eq
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (const 1
            (spec-type-number 'untyped-int')
          )
        )
      )
      (post
        (st-assign
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (expr-add
            (value-var
              (spec-var 'x'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
            (value-var
              (spec-var 'x'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
        )
      )
      (scope 85
        (code
          (scope 94
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition
        (expr-eq
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (const 1
            (spec-type-number 'untyped-int')
          )
        )
      )
      (post)
      (scope 99
        (code
          (scope 103
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
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
      (scope 108
        (code
          (scope 109
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-for
      (pre)
      (condition
        (expr-eq
          (value-var
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (const 1
            (spec-type-number 'untyped-int')
          )
        )
      )
      (post)
      (scope 114
        (code
          (scope 119
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
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
      (scope 124
        (code
          (scope 125
            (code
              (st-assign
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
                (value-var
                  (spec-var 'x'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
  )
)
