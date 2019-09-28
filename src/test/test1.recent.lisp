(scope 12
  (decl
    (decl-func 'main'
      (type-func 'null'
        (in
          (var 'a')
          (var 'b')
        )
        (out
          (var 'r')
        )
      )
      (scope 13
        (decl
          (decl-arg 'a'
            (type int32)
          )
          (decl-arg 'b'
            (type float32)
          )
          (decl-arg 'r'
            (type int32)
          )
        )
        (code
          (scope 21
            (decl
              (decl-var 'x'
                (type int32)
              )
              (decl-var 'y'
                (type int32)
              )
            )
            (code
              (ref 'x')
              (st-assign
                (ref 'y')
                (var 'x')
              )
              (st-assign
                (var 'x')
                (expr-plus
                  (var 'y')
                  (var 'y')
                )
              )
              (st-if
                (pre
                  (st-assign
                    (ref 'u')
                    (var 'x')
                  )
                )
                (condition
                  (expr-plus
                    (var 'u')
                    (var 'a')
                  )
                )
                (scope 35
                  (decl
                    (decl-var 'u'
                      (type int32)
                    )
                  )
                  (code
                    (scope 44
                      (decl)
                      (code
                        (st-assign
                          (var 'x')
                          (expr-plus
                            (var 'u')
                            (var 'b')
                          )
                        )
                      )
                    )
                    (st-if
                      (pre)
                      (condition
                        (st-expr
                          (var 'u')
                        )
                      )
                      (scope 50
                        (decl)
                        (code
                          (scope 53
                            (decl)
                            (code
                              (st-assign
                                (var 'x')
                                (var 'u')
                              )
                            )
                          )
                          (scope 57
                            (decl)
                            (code
                              (st-assign
                                (var 'y')
                                (var 'u')
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
                  (st-expr
                    (var 'a')
                  )
                )
                (scope 63
                  (decl)
                  (code
                    (scope 66
                      (decl)
                      (code
                        (st-assign
                          (var 'r')
                          (var 'a')
                        )
                        (st-return)
                      )
                    )
                  )
                )
              )
              (st-expr
                (expr-call
                  (func 'main')
                  (arguments
                    (var 'x')
                    (var 'y')
                  )
                )
              )
              (st-return
                (var 'x')
              )
            )
          )
        )
      )
    )
  )
  (code
    (ref 'main')
    (st-expr
      (expr-call
        (func 'main')
        (arguments)
      )
    )
  )
)
