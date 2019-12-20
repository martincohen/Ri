(scope 27
  (code
    (decl
      (spec-var 'a'
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
              (spec-var 'r'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
        )
      )
    )
    (decl
      (spec-func 'main'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'a'
                (spec-type-func 'null'
                  (in
                    (decl
                      (spec-var 'x'
                        (id 'int32')
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
              )
            )
            (decl
              (spec-var 'b'
                (id 'int32')
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
        (scope 37
          (code
            (scope 54
              (code)
            )
          )
        )
      )
    )
  )
)
