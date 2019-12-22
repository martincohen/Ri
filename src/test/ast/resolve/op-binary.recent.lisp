(scope 30
  (decl
    (spec-var 'i32'
      (value-type
        (spec-type-number 'int32')
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
      (spec-var 'i32'
        (value-type
          (spec-type-number 'int32')
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
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-and
        (const true
          (spec-type-number 'bool')
        )
        (const true
          (spec-type-number 'bool')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-or
        (const true
          (spec-type-number 'bool')
        )
        (const true
          (spec-type-number 'bool')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-lt
        (value-var
          (spec-var 'i32'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (const 1
          (spec-type-number 'int32')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-lt
        (const 1
          (spec-type-number 'int64')
        )
        (const 1
          (spec-type-number 'int64')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-lt-eq
        (const 1
          (spec-type-number 'int64')
        )
        (const 1
          (spec-type-number 'int64')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-gt
        (const 1
          (spec-type-number 'int64')
        )
        (const 1
          (spec-type-number 'int64')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-gt-eq
        (const 1
          (spec-type-number 'int64')
        )
        (const 1
          (spec-type-number 'int64')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-eq
        (const 1
          (spec-type-number 'int64')
        )
        (const 1
          (spec-type-number 'int64')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'b'
          (value-type
            (spec-type-number 'bool')
          )
        )
      )
      (expr-not-eq
        (const 1
          (spec-type-number 'int64')
        )
        (const 1
          (spec-type-number 'int64')
        )
      )
    )
  )
)
