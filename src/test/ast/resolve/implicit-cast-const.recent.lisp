(scope 27
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
        (const 1
          (spec-type-number 'int32')
        )
        (const 1
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
        (const 1
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
        (const 1
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
          (const 1
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
          (const 1
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
        (const 1
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
        (const 1
          (spec-type-number 'int32')
        )
        (expr-mul
          (const 1
            (spec-type-number 'int32')
          )
          (const 1
            (spec-type-number 'int32')
          )
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
