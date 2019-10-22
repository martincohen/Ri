(scope 27
  (code
    (decl
      (spec-var 'a'
        (id 'int32')
      )
    )
    (st-switch
      (pre)
      (condition
        (id 'a')
      )
      (scope 31
        (code
          (scope 34
            (code)
          )
        )
      )
    )
    (st-switch
      (pre
        (decl
          (spec-var 'b'
            (id 'int32')
          )
        )
      )
      (condition
        (id 'a')
      )
      (scope 36
        (code
          (scope 41
            (code)
          )
        )
      )
    )
  )
)
