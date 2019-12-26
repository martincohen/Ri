(scope 30
  (code
    (decl
      (spec-func 'f1'
        (spec-type-func 'null'
          (in)
          (out)
        )
        (scope 31
          (code
            (scope 33
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'f2'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
          (out)
        )
        (scope 36
          (code
            (scope 41
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'f3'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'float32')
                )
              )
            )
          )
          (out)
        )
        (scope 44
          (code
            (scope 52
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'f4'
        (spec-type-func 'null'
          (in)
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
        (scope 55
          (code
            (scope 60
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'f5'
        (spec-type-func 'null'
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
        (scope 63
          (code
            (scope 71
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-type-func 'F1'
        (in)
        (out)
      )
    )
    (decl
      (spec-type-func 'F2'
        (in
          (decl
            (spec-var '_'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
        (out)
      )
    )
    (decl
      (spec-type-func 'F3'
        (in
          (decl
            (spec-var '_'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
          (decl
            (spec-var '_'
              (value-type
                (spec-type-number 'int32')
              )
            )
          )
        )
        (out)
      )
    )
    (decl
      (spec-type-func 'F4'
        (in)
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
      (spec-type-func 'F5'
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
      (spec-var 'v_f1'
        (spec-type-func 'null'
          (in)
          (out)
        )
      )
    )
    (decl
      (spec-var 'v_f2'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
          (out)
        )
      )
    )
    (decl
      (spec-var 'v_f3'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
          (out)
        )
      )
    )
    (decl
      (spec-var 'v_f4'
        (spec-type-func 'null'
          (in)
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
    (decl
      (spec-var 'v_f5'
        (spec-type-func 'null'
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
    (decl
      (spec-func 'a_f1'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'f'
                (spec-type-func 'null'
                  (in)
                  (out)
                )
              )
            )
          )
          (out)
        )
        (scope 135
          (code
            (scope 140
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'a_f2'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'f'
                (spec-type-func 'null'
                  (in
                    (decl
                      (spec-var '_'
                        (value-type
                          (spec-type-number 'int32')
                        )
                      )
                    )
                  )
                  (out)
                )
              )
            )
          )
          (out)
        )
        (scope 143
          (code
            (scope 151
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'a_f3'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'f'
                (spec-type-func 'null'
                  (in
                    (decl
                      (spec-var '_'
                        (value-type
                          (spec-type-number 'int32')
                        )
                      )
                    )
                    (decl
                      (spec-var '_'
                        (value-type
                          (spec-type-number 'int32')
                        )
                      )
                    )
                  )
                  (out)
                )
              )
            )
          )
          (out)
        )
        (scope 154
          (code
            (scope 165
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'a_f4'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'f'
                (spec-type-func 'null'
                  (in
                    (decl
                      (spec-var '_'
                        (value-type
                          (spec-type-number 'int32')
                        )
                      )
                    )
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
          (out)
        )
        (scope 168
          (code
            (scope 182
              (code)
            )
          )
        )
      )
    )
    (decl
      (spec-func 'a_f5'
        (spec-type-func 'null'
          (in
            (decl
              (spec-var 'f'
                (spec-type-func 'null'
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
            (decl
              (spec-var '_'
                (value-type
                  (spec-type-number 'int32')
                )
              )
            )
          )
          (out)
        )
        (scope 185
          (code
            (scope 199
              (code)
            )
          )
        )
      )
    )
  )
)
