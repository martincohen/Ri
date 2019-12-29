(scope 30
  (decl
    (spec-module 'lib')
  )
  (decl
    (spec-var 'a'
      (spec-type-infer)
    )
  )
  (decl
    (spec-var 'b'
      (spec-type-infer)
    )
  )
  (code
    (decl
      (spec-module 'lib')
    )
    (st-assign
      (decl
        (spec-var 'a'
          (spec-type-infer)
        )
      )
      (expr-select
        (id 'lib')
        (id 'variable')
      )
    )
    (st-assign
      (decl
        (spec-var 'b'
          (spec-type-infer)
        )
      )
      (expr-call
        (expr-select
          (id 'lib')
          (id 'function')
        )
        (arguments)
      )
    )
  )
)
