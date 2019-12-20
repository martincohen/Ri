(scope 27
  (decl
    (spec-var 'a'
      (value-type
        (spec-type-number 'bool')
      )
    )
  )
  (decl
    (spec-var 'b'
      (value-type
        (spec-type-number 'bool')
      )
    )
  )
  (code
    (decl
      (spec-var 'a'
        (value-type
          (spec-type-number 'bool')
        )
      )
    )
    (decl
      (spec-var 'b'
        (value-type
          (spec-type-number 'bool')
        )
      )
    )
    (st-if
      (pre
        (decl
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
      (condition
        (expr-eq
          (value-var
            (spec-var 'a'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
      (scope 34
        (decl
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (code
          (scope 41
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
                  (spec-var 'a'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-if
      (pre)
      (condition
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
      )
      (scope 46
        (code
          (scope 49
            (code
              (st-assign
                (value-var
                  (spec-var 'b'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
                (value-var
                  (spec-var 'a'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-if
      (pre)
      (condition
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
      )
      (scope 54
        (code
          (scope 57
            (code
              (st-assign
                (value-var
                  (spec-var 'b'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
                (value-var
                  (spec-var 'a'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
              )
            )
          )
          (scope 61
            (code
              (st-assign
                (value-var
                  (spec-var 'b'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
                (value-var
                  (spec-var 'a'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
              )
            )
          )
        )
      )
    )
    (st-if
      (pre)
      (condition
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
      )
      (scope 66
        (code
          (scope 69
            (code
              (st-assign
                (value-var
                  (spec-var 'b'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
                (value-var
                  (spec-var 'a'
                    (value-type
                      (spec-type-number 'bool')
                    )
                  )
                )
              )
            )
          )
          (st-if
            (pre)
            (condition
              (value-var
                (spec-var 'a'
                  (value-type
                    (spec-type-number 'bool')
                  )
                )
              )
            )
            (scope 73
              (code
                (scope 76
                  (code
                    (st-assign
                      (value-var
                        (spec-var 'b'
                          (value-type
                            (spec-type-number 'bool')
                          )
                        )
                      )
                      (value-var
                        (spec-var 'a'
                          (value-type
                            (spec-type-number 'bool')
                          )
                        )
                      )
                    )
                  )
                )
                (scope 80
                  (code
                    (st-assign
                      (value-var
                        (spec-var 'b'
                          (value-type
                            (spec-type-number 'bool')
                          )
                        )
                      )
                      (value-var
                        (spec-var 'a'
                          (value-type
                            (spec-type-number 'bool')
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
    )
  )
)
