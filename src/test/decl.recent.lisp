(scope 22
  (decl
    (spec-var 'a'
      (value-type
        (spec-type-number 'int32')
      )
    )
  )
  (decl
    (spec-func 'test'
      (spec-type-func 'null'
        (in
          (decl
            (spec-var 'a'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
        (out
          (decl
            (spec-var 'r'
              (id 'int32')
            )
          )
        )
      )
      (scope 28
        (decl
          (spec-var 'a'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (code
          (scope 36
            (code
              (st-return
                (value-var
                  (spec-var 'a'
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
  (code
    (decl
      (spec-var 'a'
        (value-type
          (spec-type-number 'int32')
        )
      )
    )
    (st-expr
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
    )
    (decl
      (spec-func 'test'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'a'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
          (out
            (decl
              (spec-var 'r'
                (id 'int32')
              )
            )
          )
        )
        (scope 28
          (decl
            (spec-var 'a'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (code (recursive))
        )
      )
    )
    (st-expr
      (expr-call
        (value-func
          (spec-func 'test'
            (spec-type-func 'null'
              (in
                (decl
                  (spec-var 'a'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
              (out
                (decl
                  (spec-var 'r'
                    (id 'int32')
                  )
                )
              )
            )
            (scope 28
              (decl
                (spec-var 'a'
                  (value-type
                    (spec-type-number 'int32')
                  )
                )
              )
              (code (recursive))
            )
          )
        )
        (arguments
          (value-var
            (spec-var 'a'
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
