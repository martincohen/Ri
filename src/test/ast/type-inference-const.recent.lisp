(scope 24
  (decl
    (spec-var 'i32'
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
  (code
    (decl
      (spec-var 'a'
        (value-type
          (spec-type-number 'int32')
        )
      )
    )
    (decl
      (spec-var 'i32'
        (value-type
          (spec-type-number 'int32')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i32'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-add
        (const-integer 1
          (spec-type-number 'int32')
        )
        (const-integer 1
          (spec-type-number 'int32')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i32'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-add
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (const-integer 1
          (spec-type-number 'int32')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i32'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-add
        (const-integer 1
          (spec-type-number 'int32')
        )
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i32'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-add
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (expr-negative
          (const-integer 1
            (spec-type-number 'int32')
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i32'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-add
        (expr-negative
          (const-integer 1
            (spec-type-number 'int32')
          )
        )
        (value-var
          (spec-var 'a'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i32'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (expr-negative
        (const-integer 1
          (spec-type-number 'int32')
        )
      )
    )
  )
)
