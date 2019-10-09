(scope 22
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
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-bor
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
      (expr-band
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
      (expr-bxor
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
      (expr-bshr
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
      (expr-bshl
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
      (expr-bneg
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
