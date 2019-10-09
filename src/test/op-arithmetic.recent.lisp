(scope 24
  (decl
    (spec-var 'a'
      (value-type
        (spec-type-number 'int32')
      )
    )
  )
  (decl
    (spec-var 'x'
      (value-type
        (spec-type-number 'int32')
      )
    )
  )
  (decl
    (spec-var 'f'
      (value-type
        (spec-type-number 'float32')
      )
    )
  )
  (decl
    (spec-var 'y'
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
    (decl
      (spec-var 'y'
        (value-type
          (spec-type-number 'int32')
        )
      )
    )
    (decl
      (spec-var 'a'
        (value-type
          (spec-type-number 'int32')
        )
      )
    )
    (decl
      (spec-var 'f'
        (value-type
          (spec-type-number 'float32')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
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
        (expr-cast
          (arguments
            (spec-type-number 'int32')
            (value-var
              (spec-var 'f'
                (value-type
                  (spec-type-number 'float32')
                )
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
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
          (spec-var 'y'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-sub
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (value-var
          (spec-var 'y'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-mul
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (value-var
          (spec-var 'y'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-div
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (value-var
          (spec-var 'y'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-mod
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (value-var
          (spec-var 'y'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-negative
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-negative
        (value-var
          (spec-var 'y'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
  )
)
