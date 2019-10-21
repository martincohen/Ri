(scope 27
  (decl
    (spec-var 'a0'
      (value-type
        (spec-type-number 'int32')
      )
    )
  )
  (code
    (st-assign
      (decl
        (spec-var 'a0'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
      (const 1
        (spec-type-number 'int32')
      )
    )
    (st-assign
      (decl
        (spec-var 'a1'
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
      (decl
        (spec-var 'c0'
          (spec-type-number 'int32')
        )
      )
      (value-var
        (spec-var 'a0'
          (value-type
            (spec-type-number 'int32')
          )
        )
      )
    )
    (st-assign
      (decl
        (spec-var 'c1'
          (spec-type-number 'int32')
        )
      )
      (expr-add
        (value-var
          (spec-var 'a0'
            (value-type
              (spec-type-number 'int32')
            )
          )
        )
        (const 2
          (spec-type-number 'int32')
        )
      )
    )
    (st-assign
      (decl
        (spec-var 'd'
          (spec-type-number 'int64')
        )
      )
      (const 1
        (spec-type-number 'int64')
      )
    )
  )
)
