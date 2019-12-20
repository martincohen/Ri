(scope 27
  (decl
    (spec-var 'i64'
      (value-type
        (spec-type-number 'int64')
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
  (decl
    (spec-var 'u64'
      (value-type
        (spec-type-number 'uint64')
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
  (decl
    (spec-var 'u32'
      (value-type
        (spec-type-number 'uint32')
      )
    )
  )
  (decl
    (spec-var 'i16'
      (value-type
        (spec-type-number 'int16')
      )
    )
  )
  (decl
    (spec-var 'u16'
      (value-type
        (spec-type-number 'uint16')
      )
    )
  )
  (decl
    (spec-var 'i8'
      (value-type
        (spec-type-number 'int8')
      )
    )
  )
  (decl
    (spec-var 'u8'
      (value-type
        (spec-type-number 'uint8')
      )
    )
  )
  (decl
    (spec-var 'f32'
      (value-type
        (spec-type-number 'float32')
      )
    )
  )
  (decl
    (spec-var 'f64'
      (value-type
        (spec-type-number 'float64')
      )
    )
  )
  (code
    (decl
      (spec-var 'b'
        (value-type
          (spec-type-number 'bool')
        )
      )
    )
    (decl
      (spec-var 'i64'
        (value-type
          (spec-type-number 'int64')
        )
      )
    )
    (decl
      (spec-var 'u64'
        (value-type
          (spec-type-number 'uint64')
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
    (decl
      (spec-var 'u32'
        (value-type
          (spec-type-number 'uint32')
        )
      )
    )
    (decl
      (spec-var 'i16'
        (value-type
          (spec-type-number 'int16')
        )
      )
    )
    (decl
      (spec-var 'u16'
        (value-type
          (spec-type-number 'uint16')
        )
      )
    )
    (decl
      (spec-var 'i8'
        (value-type
          (spec-type-number 'int8')
        )
      )
    )
    (decl
      (spec-var 'u8'
        (value-type
          (spec-type-number 'uint8')
        )
      )
    )
    (decl
      (spec-var 'f32'
        (value-type
          (spec-type-number 'float32')
        )
      )
    )
    (decl
      (spec-var 'f64'
        (value-type
          (spec-type-number 'float64')
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i64'
          (value-type
            (spec-type-number 'int64')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'int64')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'u64'
          (value-type
            (spec-type-number 'uint64')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'uint64')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
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
      (expr-cast
        (arguments
          (spec-type-number 'int32')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'u32'
          (value-type
            (spec-type-number 'uint32')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'uint32')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i16'
          (value-type
            (spec-type-number 'int16')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'int16')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'u16'
          (value-type
            (spec-type-number 'uint16')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'uint16')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'i8'
          (value-type
            (spec-type-number 'int8')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'int8')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'u8'
          (value-type
            (spec-type-number 'uint8')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'uint8')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'f32'
          (value-type
            (spec-type-number 'float32')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'float32')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
    (st-assign
      (value-var
        (spec-var 'f64'
          (value-type
            (spec-type-number 'float64')
          )
        )
      )
      (expr-cast
        (arguments
          (spec-type-number 'float64')
          (value-var
            (spec-var 'b'
              (value-type
                (spec-type-number 'bool')
              )
            )
          )
        )
      )
    )
  )
)
