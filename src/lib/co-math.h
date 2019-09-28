#pragma once

#include <math.h>
#include <float.h>

#define MATH_FLOAT_EPSILON FLT_EPSILON

#define mathf_sin   sinf
#define mathf_cos   cosf
#define mathf_tan   tanf
#define mathf_acos  acosf
#define mathf_abs   fabsf
#define mathf_mod   fmodf
#define mathf_sqrt  sqrtf
#define mathf_exp   expf
#define mathf_log   logf

// TODO: variants for flooring/ceiling directions?
#define mathf_floor floorf
#define mathf_ceil  ceilf
#define mathf_round roundf

#ifdef MATH_SSE
    #include <xmmintrin.h>
#endif

//
//
//

//
//
//

typedef union MathV2 {
    struct { float x, y; };
    struct { float w, h; };
    float f[2];
} MathV2;

typedef union MathV4 {
    struct { float x, y, z, w; };
    float f[4];
#ifdef MATH_SSE
    __m128 m;
#endif
} MathV4;

typedef union MathM4 {
    float f[16];
    float g[4][4];
    MathV4 f4[4];
#ifdef MATH_SSE
    __m128 m[4];
#endif
} MathM4;

typedef struct MathT2 {
    float scale;
    MathV2 translate;
} MathT2;

typedef union MathXYWH {
    struct { float x, y, w, h; };
    struct { MathV2 xy; MathV2 wh; };
    float f[4];
    MathV4 f4;
} MathXYWH;

typedef struct MathXYXY {
    struct { MathV2 min; MathV2 max; };
    float f[4];
    MathV4 f4;
} MathXYXY;

//
// Float
//

inline bool mathf_is_zero(float f) {
    return mathf_abs(f) < MATH_FLOAT_EPSILON;
}

// NOTE: We must not do 'num + 0.5' or 'num - 0.5' because they can cause precision loss.
// TODO: If we don't manage to get this from MSVC, then we should rename it to `roundf` for consistency.
inline float
mathf_ns_round(float num) {
    float integer = mathf_ceil(num);
    if (num > 0)
        return integer - num > 0.5f ? integer - 1.0f : integer;
    return integer - num >= 0.5f ? integer - 1.0f : integer;
}

inline float mathf_ns_pow(float a, float b) {
    return mathf_exp(b * mathf_log(a));
}

inline float mathf_lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}

//
// MathV2
//

inline int mathv2_is_zero(const MathV2 v) {
    return mathf_is_zero(v.x) && mathf_is_zero(v.y);
}

inline bool mathv2_is_equal(const MathV2 a, const MathV2 b) {
    return a.x == b.x && a.y == b.y;
}

inline MathV2 mathv2_add(const MathV2 a, const MathV2 b) {
    return (MathV2) { a.x + b.x, a.y + b.y };
}

inline MathV2 mathv2_sub(const MathV2 a, const MathV2 b) {
    return (MathV2) { a.x - b.x, a.y - b.y };
}

inline MathV2 mathv2_mul(const MathV2 a, const MathV2 b) {
    return (MathV2) { a.x * b.x, a.y * b.y };
}

inline MathV2 mathv2_mul_1(const MathV2 a, float x) {
    return (MathV2) { a.x * x, a.y * x };
}

inline MathV2 mathv2_div(const MathV2 a, const MathV2 b) {
    return (MathV2) { a.x / b.x, a.y / b.y };
}

inline MathV2 mathv2_div_1(const MathV2 a, float x) {
    return (MathV2) { a.x / x, a.y / x };
}

inline float mathv2_dot(const MathV2 a, const MathV2 b) {
    return a.x * b.x + a.y * b.y;
}

inline float mathv2_mag_s(const MathV2 a) {
    return a.x * a.x + a.y * a.y;
}

inline float mathv2_mag(const MathV2 a) {
    return mathf_sqrt(mathv2_mag_s(a));
}

inline float mathv2_mag_i(const MathV2 a, float d) {
    float m = (a.x * a.x) + (a.y * a.y);
    if (m > 0.0f) {
        return 1.0f / mathf_sqrt(m);
    }
    return m;
}

inline MathV2 mathv2_ceil(const MathV2 a) {
    return (MathV2) { mathf_ceil(a.x), mathf_ceil(a.y) };
}

inline MathV2 mathv2_floor(const MathV2 a) {
    return (MathV2) { mathf_floor(a.x), mathf_floor(a.y) };
}

inline MathV2 mathv2_round(const MathV2 a) {
    return (MathV2) { mathf_round(a.x), mathf_round(a.y) };
}

inline MathV2
mathv2_transform(const MathV2 point, const MathT2* transform) {
    return (MathV2) {
        // (point.x + transform->translate.x) * transform->scale,
        // (point.y + transform->translate.y) * transform->scale
        (point.x * transform->scale) + transform->translate.x,
        (point.y * transform->scale) + transform->translate.y
    };
}

inline MathV2
mathv2_transform_i(const MathV2 point, const MathT2* transform) {
    return (MathV2) {
        // (point.x / transform->scale) - transform->translate.x,
        // (point.y / transform->scale) - transform->translate.y
        (point.x - transform->translate.x) / transform->scale,
        (point.y - transform->translate.y) / transform->scale
    };
}

inline MathV2
mathv2_to_unit(MathV2 v, float* o_mag)
{
    float mag = mathv2_mag(v);
    if (mag < MATH_FLOAT_EPSILON) {
        if (o_mag) {
            *o_mag = 0.0f;
        }
        return v;
    }
    float mag_n = 1.0f / mag;
    v.x *= mag_n;
    v.y *= mag_n;

    if (o_mag) {
        *o_mag = mag;
    }

    return v;
}


//
// MathM4
//

inline MathM4
mathm4_make_ortho_2d(float left, float right, float bottom, float top)
{
    float e0  = 2.0 / (right - left);
    float e5  = 2.0 / (top - bottom);
    float e12 = -(right + left) / (right - left);
    float e13 = -(top + bottom) / (top - bottom);

    return (MathM4){ .f = {
        e0,  0,   0,    0,
        0,   e5,  0,    0,
        0,   0,   -1.0, 0,
        e12, e13, 0,    1.0
    }};
}

inline MathM4
mathm4_make_scale(float x, float y, float z)
{
    return (MathM4){ .f =
    {
        x,  0,  0,  0,
        0,  y,  0,  0,
        0,  0,  z,  0,
        0,  0,  0,  1
    }};
}

inline MathM4
mathm4_make_translate(float x, float y, float z)
{
    return (MathM4){ .f =
    {
        1,  0,  0,  0,
        0,  1,  0,  0,
        0,  0,  1,  0,
        x,  y,  z,  1,
    }};
}

inline MathM4
mathm4_make_rotate_x(float a)
{
    float c = mathf_cos(a);
    float s = mathf_sin(a);

    return (MathM4){ .f = {
        1,  0,  0,  0,
        0,  c,  s,  0,
        0, -s,  c,  0,
        0,  0,  0,  1
    }};
}

inline MathM4
mathm4_make_rotate_y(float a)
{
    float c = mathf_cos(a);
    float s = mathf_sin(a);

    return (MathM4){ .f = {
        c,  0, -s,  0,
        0,  1,  0,  0,
        s,  0,  c,  0,
        0,  0,  0,  1
    }};
}

inline MathM4
mathm4_make_rotate_z(float a)
{
    float c = mathf_cos(a);
    float s = mathf_sin(a);

    return (MathM4){ .f = {
         c,  s,  0,  0,
        -s,  c,  0,  0,
         0,  0,  1,  0,
         0,  0,  0,  1
    }};
}

MathM4
mathm4_mul(const MathM4 a, const MathM4 b)
{
#define MATHM4_MUL_(C, R) \
    (a.g[0][R] * b.g[C][0]) + \
    (a.g[1][R] * b.g[C][1]) + \
    (a.g[2][R] * b.g[C][2]) + \
    (a.g[3][R] * b.g[C][3])

    return (MathM4){ .f = {
        MATHM4_MUL_(0, 0), MATHM4_MUL_(0, 1), MATHM4_MUL_(0, 2), MATHM4_MUL_(0, 3),
        MATHM4_MUL_(1, 0), MATHM4_MUL_(1, 1), MATHM4_MUL_(1, 2), MATHM4_MUL_(1, 3),
        MATHM4_MUL_(2, 0), MATHM4_MUL_(2, 1), MATHM4_MUL_(2, 2), MATHM4_MUL_(2, 3),
        MATHM4_MUL_(3, 0), MATHM4_MUL_(3, 1), MATHM4_MUL_(3, 2), MATHM4_MUL_(3, 3),
    }};
#undef MATHM4_MUL_
}

//
#ifdef MATH_SSE
//

inline __m128
mathm4_sse_linear_combine(__m128 a, MathM4 b)
{
    __m128 m = {0};

    m = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x00), b.m[0]);
    m = _mm_add_ps(m, _mm_mul_ps(_mm_shuffle_ps(a, a, 0x55), b.m[1]));
    m = _mm_add_ps(m, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xaa), b.m[2]));
    m = _mm_add_ps(m, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xff), b.m[3]));

    return m;
}

inline MathM4
mathm4_sse_mul(MathM4 a, MathM4 b)
{
    _MM_TRANSPOSE4_PS(a.m[0], a.m[1], a.m[2], a.m[3]);
    _MM_TRANSPOSE4_PS(b.m[0], b.m[1], b.m[2], b.m[3]);

    MathM4 m;

    m.m[0] = mathm4_sse_linear_combine(a.m[0], b);
    m.m[1] = mathm4_sse_linear_combine(a.m[1], b);
    m.m[2] = mathm4_sse_linear_combine(a.m[2], b);
    m.m[3] = mathm4_sse_linear_combine(a.m[3], b);

    _MM_TRANSPOSE4_PS(m.m[0], m.m[1], m.m[2], m.m[3]);

    return m;
}

//
#endif // MATH_SSE
//

//
//
//



