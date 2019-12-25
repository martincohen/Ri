(scope 30
  (decl
    (spec-type-func 'MyFunc'
      (in
        (decl
          (spec-var '_'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
      (out
        (decl
          (spec-var 'null'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
  )
  (decl
    (spec-func 'pow2'
      (spec-type-func 'null'
        (in
          (decl
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
        (out
          (decl
            (spec-var 'null'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
      )
      (scope 31
        (decl
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (code
          (scope 39
            (code
              (st-return
                (expr-mul
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
  (decl
    (spec-var 'a'
      (value-type
        (spec-type-func 'MyFunc'
          (in
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
          (out
            (decl
              (spec-var 'null'
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
  (code
    (decl
      (spec-func 'pow2'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'x'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
          (out
            (decl
              (spec-var 'null'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
        )
        (scope 31
          (decl
            (spec-var 'x'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (code (recursive))
        )
      )
    )
    (decl
      (spec-type-func 'MyFunc'
        (in
          (decl
            (spec-var '_'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
        (out
          (decl
            (spec-var 'null'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
      )
    )
    (st-assign
      (decl
        (spec-var 'a'
          (value-type
            (spec-type-func 'MyFunc'
              (in
                (decl
                  (spec-var '_'
                    (value-type
                      (spec-type-number 'int32')
                    )
                  )
                )
              )
              (out
                (decl
                  (spec-var 'null'
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
      (value-func
        (spec-func 'pow2'
          (spec-type-func 'null'
            (in
              (decl
                (spec-var 'x'
                  (value-type
                    (spec-type-number 'int32')
                  )
                )
              )
            )
            (out
              (decl
                (spec-var 'null'
                  (value-type
                    (spec-type-number 'int32')
                  )
                )
              )
            )
          )
          (scope 31
            (decl
              (spec-var 'x'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
            (code (recursive))
          )
        )
      )
    )
    (st-expr
      (expr-call
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-func 'MyFunc'
                (in
                  (decl
                    (spec-var '_'
                      (value-type
                        (spec-type-number 'int32')
                      )
                    )
                  )
                )
                (out
                  (decl
                    (spec-var 'null'
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
        (arguments
          (const 4
            (spec-type-number 'int32')
          )
        )
      )
    )
  )
)
