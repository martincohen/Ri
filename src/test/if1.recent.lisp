(scope 12
  (decl
    (decl-var 'a'
      (type int32)
    )
    (decl-var 'b'
      (type int32)
    )
  )
  (code
    (ref 'a')
    (ref 'b')
    (st-if
      (pre
        (ref 'x')
      )
      (condition
        (var 'a')
      )
      (scope 19
        (decl
          (decl-var 'x'
            (type int32)
          )
        )
        (code
          (scope 24
            (decl)
            (code
              (st-assign
                (var 'x')
                (var 'a')
              )
            )
          )
        )
      )
    )
    (st-if
      (pre)
      (condition
        (st-expr
          (var 'a')
        )
      )
      (scope 29
        (decl)
        (code
          (scope 32
            (decl)
            (code
              (st-assign
                (var 'b')
                (var 'a')
              )
            )
          )
        )
      )
    )
    (st-if
      (pre)
      (condition
        (st-expr
          (var 'a')
        )
      )
      (scope 37
        (decl)
        (code
          (scope 40
            (decl)
            (code
              (st-assign
                (var 'b')
                (var 'a')
              )
            )
          )
          (scope 44
            (decl)
            (code
              (st-assign
                (var 'b')
                (var 'a')
              )
            )
          )
        )
      )
    )
    (st-if
      (pre)
      (condition
        (st-expr
          (var 'a')
        )
      )
      (scope 49
        (decl)
        (code
          (scope 52
            (decl)
            (code
              (st-assign
                (var 'b')
                (var 'a')
              )
            )
          )
          (st-if
            (pre)
            (condition
              (st-expr
                (var 'a')
              )
            )
            (scope 56
              (decl)
              (code
                (scope 59
                  (decl)
                  (code
                    (st-assign
                      (var 'b')
                      (var 'a')
                    )
                  )
                )
                (scope 63
                  (decl)
                  (code
                    (st-assign
                      (var 'b')
                      (var 'a')
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
