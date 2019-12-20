(scope 27
  (decl
    (spec-var 'a'
      (value-type
        (spec-type-number 'bool')
      )
    )
  )
  (decl
    (spec-var 'x'
      (value-type
        (spec-type-number 'bool')
      )
    )
  )
  (decl
    (spec-var 'y'
      (value-type
        (spec-type-number 'bool')
      )
    )
  )
  (code
    (decl
      (spec-var 'x'
        (value-type
          (spec-type-number 'bool')
        )
      )
    )
    (decl
      (spec-var 'y'
        (value-type
          (spec-type-number 'bool')
        )
      )
    )
    (decl
      (spec-var 'a'
        (value-type
          (spec-type-number 'bool')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-and
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
        (value-var
          (spec-var 'y'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-or
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
        (value-var
          (spec-var 'y'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'a'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-not
        (value-var
          (spec-var 'x'
            (value-type
              (spec-type-number 'bool')
            )
          )
        )
      )
    )
  )
)
