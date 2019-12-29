#ifndef CO_LIB_H_INCLUDED
#define CO_LIB_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include <uchar.h>
#include <string.h>
#include <wchar.h>
#include <stddef.h>

#include <stb_printf.h>

#include <co-math.h>

//
// System
//

#if defined(_WIN32)
    #define SYSTEM_WINDOWS
    #include <windows.h>
#elif defined(__EMSCRIPTEN__)
    #define SYSTEM_EMSC
#else
    #define SYSTEM_LINUX
#endif

#if defined(_MSC_VER)
    #define COMPILER_MSVC
#elif defined(__GNUC__)
    #define COMPILER_GCC
#endif

#ifdef BUILD_MINIMAL
    #ifndef SYSTEM_WINDOWS
        #error "minimal build is only for Windows"
    #endif
#endif
//
//
//

#include <id-error.h>

#define __DIR__ __FILE__ "\\.."

//
// DLL
//

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

#if 1
    #define RUNT_API
#else
    #if defined(RUNT_APP)
    #define DLL_IMPORT
    #else
    #define DLL_EXPORT
    #endif
#endif


//
// Platform
//

// TODO Set by platform.
#define PAGE_SIZE 4096

//
// Types
//

typedef  intptr_t iptr;
typedef uintptr_t uptr;

#define IPTR_MAX INTPTR_MAX
#define IPTR_MIN INTPTR_MIN
#define UPTR_MAX UINTPTR_MAX
#define UPTR_MIN UINTPTR_MIN

//
// Utilities
//

#if !defined(ALIGNOF)
    #define ALIGNOF(Type) offsetof(struct { char c; Type member; }, member)
#endif

#if defined(COMPILER_MSVC)
    #define ATTR_ALIGN(x) __declspec(align(x))
#elif defined(COMPILER_GCC)
    #define ATTR_ALIGN(x) __attribute__ ((aligned(x)))
#endif

#define UNUSED(x) (void)x
#define COUNTOF(a) (sizeof(a) / sizeof((a)[0]))
// NOTE: Signed variant of sizeof!
#define SIZEOF(a) ((iptr)sizeof(a))

#define KILOBYTES(n) (1024*(n))
#define MEGABYTES(n) (1024*KILOBYTES(n))
#define GIGABYTES(n) (1024*MEGABYTES(n))

#define MAXIMUM(a, b) ((a) > (b) ? (a) : (b))
#define MINIMUM(a, b) ((a) < (b) ? (a) : (b))

#define CLAMP(a, min, max) MINIMUM(max, MAXIMUM(min, a))

#if !defined(SWAP)
    #define SWAP(Type, a, b) { Type tmp = (a); (a) = (b); (b) = tmp; }
#endif

#define CAST(Type, Value) (*((Type*)&(Value)))

// Should return:
// < 0 -- Element 'a' is lower than 'b' ('a' goes before 'b')
// = 0 -- Element 'a' is equivalent to 'b'
// > 0 -- Element 'a' is greater than 'b' ('a' goes after 'b')
#define COMPARE_F(Name, Type) int Name(const Type* a, const Type* b)
typedef int (*CompareF)(const void* a, const void* b);

//
// VA
//

#if defined(COMPILER_MSVC)
    #define VA_COUNTOF(...)  INTERNAL_EXPAND_ARGS_PRIVATE(INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
    #define INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
    #define INTERNAL_EXPAND(x) x
    #define INTERNAL_EXPAND_ARGS_PRIVATE(...) INTERNAL_EXPAND(INTERNAL_VA_COUNTOF_PRIVATE(__VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
    #define INTERNAL_VA_COUNTOF_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
#else
    #define VA_COUNTOF(...) INTERNAL_VA_COUNTOF_PRIVATE(0, ## __VA_ARGS__, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
    #define INTERNAL_VA_COUNTOF_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, count, ...) count
#endif

//
//
//

void log_assert_(const char *message, const char *expression, const char *function, const char *file, int line, ...);

// TODO: Change to panic in BUILD_RELEASE
#if defined(SYSTEM_WINDOWS)
    // TODO: Use DebugBreak()
    #define ABORT() __debugbreak()
// #elif defined(SYSTEM_EMSC)
//     #define ABORT() asm("debugger")
// #else // Unix
//     #define ABORT() __asm__ __volatile__ ( "int $3\n\t" )
#else
    #define ABORT() abort()
#endif

#if !defined(BUILD_RELEASE)
    #define ASSERT_MESSAGE(expression, message, ...) \
        ( \
            (!!(expression)) || ( \
                log_assert_(      \
                    message,      \
                    #expression,  \
                    __FUNCTION__, \
                    __FILE__,     \
                    __LINE__,     \
                    ##__VA_ARGS__ \
                ),                \
                ABORT(),          \
                0 \
            ) \
        )
    #define ASSERT(expression) ASSERT_MESSAGE(expression, #expression)
    #define CHECK ASSERT
    #define FAIL(message, ...) (lprintf(message, ##__VA_ARGS__), ABORT())
#else
    #define ASSERT_MESSAGE(expression, ...) ((!!(expression)) || (ABORT(), 0))
    #define ASSERT(expression) ASSERT_MESSAGE(expression, "")
    #define CHECK
    #define FAIL(message, ...) ABORT()
#endif

// #if !defined(BUILD_RELEASE)
#if 1
    static const char* LOG_LEVELS[] = {
        "[error]",
        "[ info]",
        "[debug]"
    };

    #define LOG(Message, ...)       lprintf(Message "\n", ## __VA_ARGS__)
    #define LOG_ERROR(Message, ...) lprintf("%s %24s:%-4d " Message "\n", LOG_LEVELS[0], __FUNCTION__, __LINE__, ## __VA_ARGS__)
    #define LOG_INFO(Message, ...)  lprintf("%s %24s:%-4d " Message "\n", LOG_LEVELS[1], __FUNCTION__, __LINE__, ## __VA_ARGS__)
    #define LOG_DEBUG(Message, ...) lprintf("%s %24s:%-4d " Message "\n", LOG_LEVELS[2], __FUNCTION__, __LINE__, ## __VA_ARGS__)

#else
    #define LOG(message, ...)
    #define LOG_ERROR(...)
    #define LOG_INFO(...)
    #define LOG_DEBUG(...)
#endif


//
// Printing
//

// NOTE: Keep in sync with stb_printf.h
#define CBPRINTF(name) char* name(char* buf, void* user, int length)
typedef CBPRINTF(CBPrintF);

#define cbprintfv(F, User, Buf, Format, Args) stbsp_vsprintfcb(F, User, Buf, Format, Args)

void cbprintf(CBPrintF* f, void* user, const char* format, ...);
void lvprintf(const char* format, va_list args);
void lprintf(const char* format, ...);

//
// Helpers
//

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

inline uint32_t
u32_next_power_of_two(uint32_t m)
{
    uint32_t n = m - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

inline uint64_t
u64_next_power_of_two(uint64_t m)
{
    uint64_t n = m - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}

inline int
iptr_is_power_of_two(iptr x)
{
    if (x <= 0) {
        return 0;
    }
    return !(x & (x-1));
}

inline iptr
iptr_signof(iptr x) {
    return (x > 0) - (x < 0);
}

inline iptr
iptr_align_forward(iptr ptr, iptr alignment)
{
    CHECK(iptr_is_power_of_two(alignment));
    iptr p = (iptr)ptr;
    iptr modulo = p & (alignment-1);
    if (modulo) {
        p += (alignment - modulo);
    }
    return p;
}

//
// Random
//

#define RAND32_MAX 0xffffffff

inline uint32_t
rand_uint32(uint32_t *x)
{
    if (*x == 0) *x = 314159265;
    *x ^= *x << 13;
    *x ^= *x >> 17;
    *x ^= *x << 5;
    return *x;
}

inline float
rand_float(uint32_t *x)
{
    return (float)(rand_uint32(x) & ((1 << 30) - 1)) / (float)((1 << 30));
}

inline float
rand_float_r(uint32_t *x, float min, float max)
{
    return min + (rand_float(x) * (max-min));
}

inline int32_t
rand_int32_r(uint32_t *x, int32_t min, int32_t max)
{
    int32_t f = abs((int32_t)rand_uint32(x));
    return min + (f % ((max-min) + 1));
}

//
// Void math
//

#define void_minus_t(ItA, ItB, Type) \
    (((Type*)ItA) - ((Type*)ItB))

#define void_plus_t(It, Type, Amount) \
    (void*)(((Type*)It) + (Amount))

#define void_minus(ItA, ItB) \
    void_minus_t(ItA, ItB, uint8_t)

#define void_plus(It, Amount) \
    void_plus_t(It, uint8_t, Amount)

inline void*
void_align_forward(void* ptr, iptr alignment)
{
    CHECK(iptr_is_power_of_two(alignment));
    uptr p = (uptr)ptr;
    iptr modulo = p & (alignment-1);
    if (modulo) {
        p += (alignment - modulo);
    }
    return (void*)p;
}

//
// Performance
//

double perf_get();

//
// Memory
//

#if 1
void* heap_alloc(iptr size);
void* heap_alloc_zeroed(iptr size);
void* heap_realloc(const void* ptr, iptr size);
void heap_free(const void* ptr);
#else
#define heap_alloc malloc
#define heap_realloc realloc
#define heap_free free
#endif

void* virtual_reserve(void* ptr, iptr size);
void* virtual_commit(void* ptr, iptr size);
void  virtual_decommit(void* ptr, iptr size);
void  virtual_free(void* ptr, iptr size);
void* virtual_alloc(void* ptr, iptr size);

//
// Threads
//

void thread_sleep(int ms);
int thread_is_main();

//
// Hashing
//

uint64_t hash_blob(const void *ptr, iptr len);
uint64_t hash_blob_begin();
uint64_t hash_blob_add(uint64_t hash, const void* ptr, iptr length);
uint64_t hash_uint64(uint64_t x);
uint64_t hash_ptr(const void *ptr);
uint64_t hash_mix(uint64_t x, uint64_t y);

//
//
//

typedef struct Range
{
    int64_t start;
    int64_t end;
}
Range;

//
//
//

#define Slice(Type)  \
    struct {         \
        Type* items; \
        iptr count;  \
    }

#define slice_is_empty(Slice) \
    (((Slice)->items == 0) || ((Slice)->count == 0))

#define slice_get_end(Slice) \
    ((Slice)->items + (Slice)->count)

#define slice_at(Slice, ...) \
    ((Slice)->items[(CHECK((__VA_ARGS__) >= 0), CHECK((__VA_ARGS__) < (Slice)->count), (__VA_ARGS__))])

#define slice_last(Slice) \
    ((Slice)->items[(CHECK((Slice)->count > 0), (Slice)->count - 1)])

// TODO: Use collection_* API
#define slice_bsearch(Slice, k, f) \
    bsearch(k, (Slice)->items, (Slice)->count, sizeof(*(Slice)->items), f)

#define slice_find(Slice, Needle, It)                                 \
    do                                                                \
    {                                                                 \
        for (                                                         \
            (It) = 0;                                                 \
            (It) < (Slice)->count && slice_at(Slice, (It)) != Needle; \
            (It) = (It) + 1)                                          \
            ;                                                         \
        if ((It) == (Slice)->count)                                   \
        {                                                             \
            (It) = -1;                                                \
        }                                                             \
    } while (0)

#define slice_eachri(Slice, It, Value)                     \
    for (iptr It = (Slice)->count - 1;                     \
         It >= 0 && ((*(Value) = slice_at(Slice, It)), 1); \
         It--)

#define slice_eachi(Slice, It, Value)                                  \
    for (iptr It = 0;                                                  \
         It < (Slice)->count && ((*(Value) = slice_at(Slice, It)), 1); \
         It++)

#define slice_each(Slice, Value) \
    slice_eachi(Slice, i__, Value)

//
//
//

#define ArrayWithSlice(SliceT) \
    struct                     \
    {                          \
        union {                \
            SliceT;            \
            SliceT slice;      \
        };                     \
        iptr capacity;         \
    }

#define Array(T) ArrayWithSlice(Slice(T))

typedef Slice(iptr) IPtrSlice;
typedef ArrayWithSlice(IPtrSlice) IPtrArray;
typedef Slice(int) IntSlice;
typedef ArrayWithSlice(IntSlice) IntArray;
typedef Slice(int) UInt16Slice;
typedef ArrayWithSlice(UInt16Slice) UInt16Array;
typedef Slice(uint8_t) ByteSlice;
typedef ArrayWithSlice(ByteSlice) ByteArray;
typedef Slice(float) FloatSlice;
typedef ArrayWithSlice(FloatSlice) FloatArray;

#define array_init(v) \
    memset((v), 0, sizeof(*(v)))

#define array_purge(v) \
    heap_free((v)->items)

inline void
array_grow_(void **items, iptr* capacity, iptr item_size, iptr new_count)
{
    if (new_count > *capacity) {
        *capacity = (iptr)u64_next_power_of_two(MAXIMUM(new_count, 16));
        *items = heap_realloc(*items, *capacity * item_size);
    }
}

// array_reserve(&array, 10)
// Ensures the array has capacity for at least 10 items.
//
#define array_reserve(v, ...) \
    array_grow_((void**)(&((v)->items)), &(v)->capacity, sizeof(*(v)->items), __VA_ARGS__)

// array_at(&array, 5)
// Returns reference to fifth item of the array.
//

#define array_at slice_at

// array_last(&array)
// Returns last item of the array.
//
#define array_last slice_last

// array_clear(&array)
// Removes all items from the array.
//
#define array_clear(v) \
    ((v)->count = 0)

// array_push(&array, 123)
// Add value '123' at the end of the array and returns reference to the item.
//
#define array_push(v, ...) \
    (array_reserve(v, (v)->count + 1), (v)->items[(v)->count++] = (__VA_ARGS__))

// array_push_n(&array, 10)
// Adds 10 items at the end of the array and returns pointer to the first added item.
//
#define array_push_n(v, ...)                                \
    (                                                       \
        array_reserve(v, (v)->count + (__VA_ARGS__)),       \
        (v)->count += (__VA_ARGS__),                        \
        &((v)->items[(v)->count - (__VA_ARGS__)])           \
    )                                                       \

#define array_pop(v) \
    ((v)->items[(CHECK((v)->count > 0), --(v)->count)])

// array_remove(&array, 5, 10)
// Removes 10 items from offset 5 and returns pointer to 5th item.
#define array_remove(v, i, ...)                                         \
    (                                                                   \
        CHECK((i) >= 0),                                                \
        CHECK((__VA_ARGS__) >= 0),                                      \
        CHECK((i) <= (v)->count),                                       \
        CHECK(((i) + (__VA_ARGS__)) <= (v)->count),                     \
                                                                        \
        memmove(                                                        \
            (v)->items + (i), (v)->items + (i) + (__VA_ARGS__),         \
            ((v)->count - (i) - (__VA_ARGS__)) * sizeof(*(v)->items)    \
        ),                                                              \
                                                                        \
        (v)->count -= (__VA_ARGS__),                                    \
        (v)->items + (i)                                                \
    )

// array_insert(&array, 5, 10)
// Inserts 10 items at offset 5 and returns pointer to first item.
//
#define array_insert_n(v, i, ...)                                       \
    (                                                                   \
        CHECK((i) >= 0),                                                \
        CHECK((i) <= (v)->count),                                       \
        CHECK((__VA_ARGS__) >= 0),                                      \
                                                                        \
        array_reserve(v, (v)->count + (__VA_ARGS__)),                   \
        memmove(                                                        \
            (v)->items + (i) + (__VA_ARGS__),                           \
            (v)->items + (i),                                           \
            ((v)->count - (i)) * (sizeof(*(v)->items) * (__VA_ARGS__))    \
        ),                                                              \
                                                                        \
        (v)->count += (__VA_ARGS__),                                    \
        (v)->items + (i)                                                \
    )

// array_insert(&array, 5, 123)
// Inserts value '123' at the offset 5, returns reference to the value.
//
#define array_insert(v, i, ...) \
    (*array_insert_n(v, i, 1) = (__VA_ARGS__))


    // (                                                       \
    //     array_reserve(v, (v)->count + 1),                   \
    //     memmove(                                            \
    //         (v)->items + (i) + 1,                           \
    //         (v)->items + (i),                               \
    //         (v)->count - (i)) * sizeof(*(v)->items),        \
    //     ++(v)->count,                                       \
    //     (v)->items[(i)] = (__VA_ARGS__)                     \
    // )


// TODO: __VA_ARGS__
#define array_resize(v, n) \
    (array_reserve(v, n), (v)->count = (n))

#define array_sort(v, f) \
    co_qsort((v)->items, (v)->count, sizeof(*(v)->items), (CompareF)(f))

#define array_bsearch slice_bsearch
#define array_find slice_find
#define array_eachri slice_eachri
#define array_eachi slice_eachi
#define array_each slice_each

// Makes sure the array ends with 0, but the count is not updated.
#define array_zero_term(Array) \
    (array_push(Array, 0), (Array)->count--)

//
// Chars
//

inline int char_isspace(char c) {
    return c > 0 && c <= 32;
}
inline int char_isnumber(char c) {
    return c >= '0' && c <= '9';
}

//
// String
//

inline size_t strlen16(register const char16_t* string) {
    if (!string) {
        return 0;
    }
    register size_t length = 0;
    while (string[length++]);
    return length;
}

typedef Slice(char) String;
typedef Slice(char16_t) String16;
typedef Slice(char32_t) String32;

typedef ArrayWithSlice(String) CharArray;
typedef ArrayWithSlice(String16) Char16Array;
typedef ArrayWithSlice(String32) Char32Array;

typedef Slice(String) StringSlice;
typedef ArrayWithSlice(StringSlice) StringArray;

extern String STRING_EMPTY;

inline String
string_make_(const char* string, iptr string_length)
{
    ASSERT(string_length >= 0);
    return ((String){
        .items = (char*)string,
        .count = string_length
    });
}

#define S_helper(MakeF, LenF, String, ...)  \
    (VA_COUNTOF(__VA_ARGS__) == 0           \
        ? MakeF(String, (iptr)LenF(String)) \
        : MakeF(String, (__VA_ARGS__ + 0)))

#define S(String, ...) S_helper(string_make_, strlen, String, __VA_ARGS__)

#define S_STATIC(S) { .items = S, .count = sizeof(S) - 1 }

inline String16
string16_make_(const char16_t* string, iptr string_length)
{
    CHECK(string_length >= 0);
    return (String16){
        .items = (char16_t*)string,
        .count = string_length
    };
}

#define Su(String, ...) S_helper(string16_make_, strlen16, String, __VA_ARGS__)

#define string_is_empty(String) slice_is_empty(String)
#define string16_is_empty(String) slice_is_empty(String)

// TODO: Implement as generic 'slice_is_equal' or `slice_compare` function.
inline int
string_is_equal(const String a, const String b) {
    if (a.items == b.items && a.count == b.count) {
        return 1;
    }

    if (a.count != b.count) {
        return 0;
    }

    return memcmp(a.items, b.items, a.count) == 0;
}

inline String
string_slice(String a, iptr start, iptr end) {
    CHECK(start == -1 || start >= 0);
    CHECK(end == -1 || end >= start);
    CHECK(end <= a.count);
    start = start == -1 ? a.count : start;
    end = end == -1 ? a.count : end;
    return S(a.items + start, end - start);
}

inline String16
string16_slice(String16 a, iptr start, iptr end) {
    CHECK(start >= 0);
    CHECK(end == -1 || end >= start);
    CHECK(end <= a.count);
    return Su(a.items + start, (end == -1 ? a.count : end) - start);
}


// TODO: Implement as generic 'slice_pop_front' function.
inline String
string_pop_front(String* a, iptr amount) {
    CHECK(amount <= a->count);
    String slice = {
        .items = a->items,
        .count = amount
    };
    a->items += amount;
    a->count -= amount;
    return slice;
}

// TODO: Implement as generic 'slice_pop_back' function.
inline String
string_pop_back(String* a, iptr amount) {
    CHECK(amount <= a->count);
    String slice = {
        .items = a->items + a->count - amount,
        .count = amount
    };
    a->count -= amount;
    return slice;
}

// Returns line without line endings via 'o_line'.
// Moves 'state' to start of next line.
// Currently supports only LF or CRLF line endings.
// TODO: Add support for CR line endings.
int string_read_line(String* state, String* o_line);

// TODO Implement this as generic `array_push_slice`.
String chararray_push(CharArray* array, String string);
String chararray_push_f(CharArray* array, const char* format, ...);
String chararray_push_fv(CharArray* array, const char* format, va_list args);

String16 char16array_push(Char16Array* array, String16 string);

//
// String fuzzy.
//

#define StringFuzzyCompare_NoMatch IPTR_MIN
#define StringFuzzyCompare_FullMatch IPTR_MAX

iptr string_fuzzy_compare(String needle, String haystack);
iptr string_fuzzy_compare_r(String needle, String haystack);

//
// Error
//

typedef struct Error {
    int code;
    CharArray message;
} Error;

void error_set(Error* error, int code, const char* format, ...);
void error_clear(Error* error);

#define ERROR_ID(ID) Error_ ## ID

//
// ValueBox
//

enum ValueBoxKind {
    ValueBox_Int64,
    ValueBox_UInt64,
    ValueBox_Int32,
    ValueBox_UInt32,
    ValueBox_Int16,
    ValueBox_UInt16,
    ValueBox_Int8,
    ValueBox_UInt8,
    ValueBox_Float64,
    ValueBox_Float32,
    ValueBox_String,
    ValueBox_String16,
    ValueBox_Ptr,
    ValueBox_Blob
} ValueBoxKind;

#define VALUE_SCALAR_BOX_FIELDS \
    int64_t i64; \
    uint64_t u64; \
    int32_t i32; \
    uint32_t u32; \
    int16_t i16; \
    uint16_t u16; \
    int8_t i8; \
    uint8_t u8; \
    float f32; \
    double f64; \
    void* ptr

#define VALUE_SLICE_BOX_FIELDS \
    String string; \
    String16 string16; \
    struct { \
        void* ptr; \
        iptr size; \
    } blob

typedef union ValueScalar {
    VALUE_SCALAR_BOX_FIELDS;
} ValueScalar;

typedef union Value {
    VALUE_SCALAR_BOX_FIELDS;
    VALUE_SLICE_BOX_FIELDS;
    ValueScalar S;
} Value;

typedef struct ValueBox
{
    enum ValueBoxKind kind;
    union {
        VALUE_SCALAR_BOX_FIELDS;
        VALUE_SLICE_BOX_FIELDS;
        Value V;
        ValueScalar S;
    };
} ValueBox;

//
// Collections
//

// returns < 0 -- collection[index] < rhs
// returns   0 -- collection[index] == rhs
// returns > 0 -- collection[index] > rhs
#define COLLECTION_COMPARE_F(Name) int Name(void* collection, iptr index, ValueBox rhs)
typedef COLLECTION_COMPARE_F(CollectionCompareF);

// Example for Array(int32_t).
inline COLLECTION_COMPARE_F(rawarray_int32_compare) {
    return ((int32_t*)collection)[index] - rhs.i32;
}

// Returns index of 'needle' or -1 in sorted collection.
iptr collection_index_of(void* collection, ValueBox needle, iptr search_start, iptr search_end, CollectionCompareF* comparef);
// Returns index past last occurence of 'needle' in sorted collection.
// If needle is not matched, then returns index of nearest greater value or search_end + 1.
// Useful for cases when you store offsets of line endings. 'upper_bound' returns index of line to which the offset 'needle' belongs.
iptr collection_upper_bound(void* collection, ValueBox needle, iptr search_start, iptr search_end, CollectionCompareF* comparef);
// Returns index of first occurence of 'needle' in sorted collection.
// If needle is not matched, then returns index of nearest lower value or search_start.
iptr collection_lower_bound(void* collection, ValueBox needle, iptr search_start, iptr search_end, CollectionCompareF* comparef);

//
// UTF
//

#define RUNE_REPLACEMENT 0xFFFD

// Returns true if Byte is lower than 128 (plain ASCII codepoint)
#define UTF8_BYTE_IS_H0(Byte)      (((Byte) & 0x80) == 0)
// Returns true if Byte matches 110* **** (2 byte header).
// 1 more byte that matches UTF8_BYTE_IS_C should follow.
#define UTF8_BYTE_IS_H1(Byte)      (((Byte) & 0xE0) == 0xC0)
// Returns true if Byte matches 1110 **** (3 byte header)
// 2 more bytes that matches UTF8_BYTE_IS_C should follow.
#define UTF8_BYTE_IS_H2(Byte)      (((Byte) & 0xF0) == 0xE0)
// Returns true if Byte matches 1111 0*** (4 byte header)
// 3 more bytes that matches UTF8_BYTE_IS_C should follow.
#define UTF8_BYTE_IS_H3(Byte)      (((Byte) & 0xF8) == 0xF0)
// Returns true if Byte matches 10** ****
#define UTF8_BYTE_IS_C(Byte)       (((Byte) & 0xC0) == 0x80)
// Opposite of UTF8_BYTE_IS_H0 -- Returns true if byte is either a header o continuation byte.
#define UTF8_BYTE_IS_H_OR_C(Byte)  (((Byte) & 0x80) == 0x80)

#define UTF_IS_SURROGATE(c) \
    ((c) >= 0xD800U && (c) <= 0xDFFFU)
#define UTF_SURROGATE_H(rune) \
    (0xD7C0 + ((rune) >> 10))
#define UTF_SURROGATE_L(rune) \
    (0xDC00 + ((rune) & 0x3FF))

enum {
    UTF_OutputStarving = -1,
    UTF_Error = 0,
    UTF_Success = 1,
};

#define UTF_CALLBACK_F(Name) char32_t* Name(char32_t* runes, iptr count, void* arg)
typedef UTF_CALLBACK_F(UTFCallbackF);

//
// From UTF8 conversions.
//

// WARNING: Reads 4 bytes from utf8. First byte is used to detect the length,
// and all bytes past the length-count of bytes are ignored, so they can remain
// uninitialized.
// If any error occurs, rune will be changed, but 'error' parameter will be non-zero.
// Returns incremented pointer to utf8 data.
void* utf8_decode_wellons(void *utf8, uint32_t *rune, int *error);

// Common function that converts utf8 to another encoding.
// Function first converts utf8 to 'cache', and flushes the output to the callback passed.
// If 'cache' is NULL, then we use internal 8192 array of uint32_t's allocated on stack.
int utf8_convert(String* utf8, String32* cache, UTFCallbackF* callback, void* array);
// See notes for utf8_to_utf32_ for handling invalid utf8 inputs.
int utf8_to_utf16_(String* utf8, String32* cache, Char16Array* utf16);
// Stops before consuming errorneous UTF-8 sequence and returns UTF_Error.
// Error recovery is responsibility of the caller. In general case you want to
// output RUNE_REPLACMENT to utf32, and skip the utf8 sequence.
// TODO: Implement a function to skip invalid utf8 sequences.
// - Skip first byte. The byte is guaranteed to match UTF8_BYTE_IS_H_OR_C.
// - Skip bytes while UTF8_BYTE_IS_C returns true.
int utf8_to_utf32_(String* utf8, String32* cache, Char32Array* utf32);

// Convenience API.
// Converts continuous utf8 to utf16, replaces invalid utf8 sequences by inserting
// RUNE_REPLACEMENT to the output. If you want to deal with errors yourself,
// use 'utf8_to_utf16_'.
// TODO: Implement default error recovery.
inline int utf8_to_utf16(String utf8, String32* cache, Char16Array* utf16) {
    String state = utf8;
    ASSERT(utf8_to_utf16_(&state, cache, utf16) == UTF_Success);
    return 1;
}

// Assumes there actually is an invalid sequence. It always skips at least one byte.
// Returns number of bytes skipped.
iptr utf8_skip_invalid_sequence(String* state);
int utf8_get_next_rune(String* state, uint32_t* o_rune);

//
// From UTF16 conversions.
//

int utf16_convert(String16* utf16, String32* cache, UTFCallbackF* callback, void* array);
int utf16_to_utf8_(String16* utf16, String32* cache, CharArray* utf8);

// TODO: Implement error recovery.
inline int utf16_to_utf8(String16 utf16, String32* cache, CharArray* utf8) {
    String16 state = utf16;
    ASSERT(utf16_to_utf8_(&state, cache, utf8) == UTF_Success);
    return 1;
}

int utf16_to_utf32(String16* utf16, String32* cache, Char32Array* utf32);

//
// Map
//

// WARNING: 'key' cannot be 0 as the hash would also be 0, and 0 is used to indicate empty slot in the map.
// WARNING: 'value' cannot be 0, as 0 is used as indication that there's no such key.

// Bitwise Extra, Day 11: End-to-End Workflow & Clean-Up
// https://youtu.be/Fj4g7HrjZBU?t=489

// TODO:
// http://nothings.org/computer/judy/hash.c
// http://nothings.org/computer/judy/

typedef struct Map {
    ValueScalar *keys;
    ValueScalar *values;
    uint64_t count;
    uint64_t capacity;
} Map;

// Optional, you can initialize the map with 0.
void map_init(Map* map, iptr count);
void map_purge(Map* map);
void map_clear(Map* map);
ValueScalar map_get(Map *map, const ValueScalar key);
iptr map_find(Map *map, const ValueScalar key);
int map_put(Map *map, const ValueScalar key, const ValueScalar val);

//
// Arena
//

typedef struct Arena {
    // Size of a single block in bytes.
    iptr block_size;
    // Pointers to blocks.
    Array(uptr) blocks;
    // Block pool.
    Array(uptr) blocks_pool;
    // Start of current block.
    void* begin;
    // End of current block.
    void* end;
    // Writing pointer to current block.
    void* it;
    // Current writing index.
    iptr head;
} Arena;

// Optional, to get default just initialize with 0.
void arena_init(Arena* arena, iptr block_size);
void arena_purge(Arena* arena);
void arena_clear(Arena* arena);
void* arena_push(Arena* arena, iptr size, iptr align);

#define arena_push_t(Arena, Type) \
    (Type*)arena_push(Arena, sizeof(Type), ALIGNOF(Type))

#define arena_push_nt(Arena, Type, Count) \
    (Type*)arena_push(Arena, sizeof(Type) * (Count), ALIGNOF(Type))

iptr arena_pour(Arena* arena, const void* data, iptr data_size, iptr align);
iptr arena_copy(Arena* arena, void* buffer, iptr start, iptr count);
// Moves writing head and frees all unused blocks.
void arena_truncate(Arena* arena, iptr end);
// Moves writing head forward.
void arena_rewind(Arena* arena, iptr head);

inline void* arena_at(Arena* arena, iptr index) {
    iptr block = index / arena->block_size;
    iptr block_index = index % arena->block_size;
    uptr addr = arena->blocks.items[block] + block_index;
    return CAST(void*, addr);
}

inline void* arena_blob_at(Arena* arena, iptr index, iptr size) {
    iptr block_index = index / arena->block_size;
    iptr block_offset = index % arena->block_size;
    if (size > (arena->block_size - block_offset)) {
        return CAST(void*, array_at(&arena->blocks, block_index + 1));
    } else {
        uptr addr = array_at(&arena->blocks, block_index) + block_offset;
        return CAST(void*, addr);
    }
}

inline ByteSlice arena_get_slice(Arena* arena, iptr index) {
    CHECK(index <= arena->head);
    iptr block = index / arena->block_size;
    iptr block_index = index % arena->block_size;
    iptr block_end = MINIMUM(arena->head, (block + 1) * arena->block_size);
    ByteSlice slice = {
        .items = (uint8_t*)arena->blocks.items[block] + block_index,
        .count = block_end - index
    };
    return slice;
}

typedef struct ArenaIterator {
    union {
        Slice(uint8_t) slice;
        Slice(uint8_t);
    };
    Range range;
    Range block;
    Arena* arena;
} ArenaIterator;

// Reads 'start[count]' range from arena.
int arenaiterator_read(ArenaIterator* it, Arena* arena, iptr start, iptr count);
// TODO: Implement arenaiterator_write that'll first push enought empty space and then allows to iterate and fill them in.
int arenaiterator_next(ArenaIterator* it);
// Interface compliance.
inline void arenaiterator_purge(ArenaIterator* it) {}

//
// Intern
//

typedef struct Intern {
    Arena arena;
    Map map;
} Intern;

void intern_init(Intern* intern);
void intern_purge(Intern* intern);
const char* intern_put_r(Intern* I, const char* start, const char* end);
const char* intern_put_c(Intern* I, const char* s, iptr s_length);

int id_has(String slice);
String id_make(String slice);

//
// Pool
//

typedef struct Pool {
    Arena* arena;
    Array(uptr) pool;
    iptr size;
    iptr align;
} Pool;

void pool_init(Pool* pool, Arena* arena, iptr size, iptr align);
void pool_purge(Pool* pool);
void* pool_alloc(Pool* pool);
void pool_dealloc(Pool* pool, void* ptr);

#define pool_init_t(Pool, Arena, Type) \
    pool_init(Pool, Arena, sizeof(Type), ALIGNOF(Type))

//
//
//

enum Alignment
{
    Align_Left    = 1 << 0,
    Align_Right   = 1 << 1,
    Align_HCenter = 1 << 2,
    Align_Top     = 1 << 3,
    Align_Bottom  = 1 << 4,
    Align_VCenter = 1 << 5,

    Align_Center  = Align_HCenter | Align_VCenter,

    Align_HMask = Align_Left | Align_Right | Align_HCenter,
    Align_VMask = Align_Top | Align_Bottom | Align_VCenter,
};

//
// 2D Math
//

typedef union Rect {
    struct {
        MathV2 min, max;
    };
    float E[4];
}
Rect;

typedef union Margin {
    struct {
        float left;
        float top;
        float right;
        float bottom;
    };
    float E[4];
} Margin;

//
//
//

inline MathV2
rect_get_size(const Rect rect) {
    return (MathV2){ rect.max.x - rect.min.x, rect.max.y - rect.min.y };
}

inline Rect
rect_make(float min_x, float min_y, float max_x, float max_y) {
    return (Rect){ min_x, min_y, max_x, max_y };
}

inline Rect
rect_make_size(float x, float y, float w, float h) {
    return rect_make(x, y, x + w, y + h);
}

inline int
rect_equal(const Rect* a, const Rect* b) {
    return (
        a->min.x == b->min.x &&
        a->min.y == b->min.y &&
        a->max.x == b->max.x &&
        a->max.y == b->max.y
    );
}

inline float
rect_get_width(const Rect rect) {
    return rect.max.x - rect.min.x;
}

inline float
rect_get_height(const Rect rect) {
    return rect.max.y - rect.min.y;
}

inline Rect
rect_expand_1(const Rect rect, float e) {
    return rect_make(
        rect.min.x - e,
        rect.min.y - e,
        rect.max.x + e,
        rect.max.y + e
    );
}

inline Rect
rect_expand_margin(const Rect rect, const Margin margin) {
    return rect_make(
        rect.min.x - margin.left,
        rect.min.y - margin.top,
        rect.max.x + margin.right,
        rect.max.y + margin.bottom
    );
}

inline Rect
rect_contract_margin(const Rect rect, const Margin margin) {
    return (Rect){
        rect.min.x + margin.left,
        rect.min.y + margin.top,
        rect.max.x - margin.right,
        rect.max.y - margin.bottom
    };
}

inline Rect
rect_contract_2(const Rect rect, float h, float v) {
    return (Rect){
        rect.min.x + h,
        rect.min.y + v,
        rect.max.x - h,
        rect.max.y - v
    };
}

inline Rect
rect_zero(const Rect rect) {
    Rect r = rect_make_size(
        0, 0, rect.max.x - rect.min.x, rect.max.y - rect.min.y);
    return r;
}

inline Rect
rect_minus_2(const Rect rect, float x, float y) {
    return (Rect){
        rect.min.x - x,
        rect.min.y - y,
        rect.max.x - x,
        rect.max.y - y,
    };
}

inline Rect
rect_minus_v2(const Rect rect, MathV2 rhs) {
    return rect_minus_2(rect, rhs.x, rhs.y);
}

inline Rect
rect_plus_2(const Rect rect, float x, float y) {
    return (Rect){
        rect.min.x + x,
        rect.min.y + y,
        rect.max.x + x,
        rect.max.y + y,
    };
}

inline Rect
rect_plus_v2(const Rect rect, MathV2 rhs) {
    return rect_plus_2(rect, rhs.x, rhs.y);
}

inline Rect
rect_mul_1(const Rect rect, float s) {
    return (Rect){
        rect.min.x * s,
        rect.min.y * s,
        rect.max.x * s,
        rect.max.y * s
    };
}

inline Rect
rect_mul_2(const Rect rect, float x, float y) {
    return (Rect){
        rect.min.x * x,
        rect.min.y * y,
        rect.max.x * x,
        rect.max.y * y
    };
}

inline Rect
rect_div_1(const Rect rect, float s) {
    return (Rect){
        rect.min.x / s,
        rect.min.y / s,
        rect.max.x / s,
        rect.max.y / s
    };
}


inline Rect
rect_cut_top(Rect* rect, float amount) {
    float min_y = rect->min.y;
    rect->min.y += amount;
    return rect_make(rect->min.x, min_y, rect->max.x, rect->min.y);
}

inline Rect
rect_cut_bottom(Rect* rect, float amount) {
    float max_y = rect->max.y;
    rect->max.y -= amount;
    return rect_make(rect->min.x, rect->max.y, rect->max.x, max_y);
}

inline Rect
rect_cut_left(Rect* rect, float amount) {
    float min_x = rect->min.x;
    rect->min.x += amount;
    return rect_make(min_x, rect->min.y, rect->min.x, rect->max.y);
}

inline Rect
rect_cut_right(Rect* rect, float amount) {
    float max_x = rect->max.x;
    rect->max.x -= amount;
    return rect_make(rect->max.x, rect->min.y, max_x, rect->max.y);
}

inline Rect
rect_place_inside(Rect bounds, const MathV2 size, uint32_t align)
{
    MathV2 pos;

    switch (align & Align_HMask)
    {
        default:
        case Align_Left:
            pos.x = bounds.min.x;
            break;
        case Align_Right:
            pos.x = bounds.max.x - size.x;
            break;
        case Align_HCenter:
            pos.x = bounds.min.x + ((rect_get_width(bounds) - size.x) * 0.5f);
            break;
    }

    switch (align & Align_VMask)
    {
        default:
        case Align_Top:
            pos.y = bounds.min.y;
            break;
        case Align_Bottom:
            pos.y = bounds.max.y - size.y;
            break;
        case Align_VCenter:
            pos.y = bounds.min.y + ((rect_get_height(bounds) - size.y) * 0.5f);
            break;
    }

    return rect_make_size(pos.x, pos.y, size.x, size.y);
}

inline Rect
rect_place_at(const MathV2 point, const MathV2 size, uint32_t align)
{
    return rect_place_inside(
        (Rect){ point.x, point.y, point.x, point.y },
        size,
        align
    );
}

inline MathV2
rect_pos(const Rect rect, float rx, float ry)
{
    return (MathV2) {
        .x = rect.min.x + rect_get_width(rect) * rx,
        .y = rect.min.y + rect_get_height(rect) * ry,
    };
}

inline int
rect_contains_xy(const Rect rect, float x, float y) {
    return (x >= rect.min.x && x < rect.max.x && y >= rect.min.y && y < rect.max.y);
}

inline int
rect_contains_v2(const Rect rect, MathV2 v2)
{
    return (
        v2.x >= rect.min.x &&
        v2.x < rect.max.x &&
        v2.y >= rect.min.y &&
        v2.y < rect.max.y
    );
}

inline int
rect_intersects_rect(const Rect a, const Rect b)
{
    return !(
        a.max.x < b.min.x ||
        a.min.x > b.max.x ||
        a.max.y < b.min.y ||
        a.min.y > b.max.y
    );
}

inline Rect
rect_transform(const Rect rect, const MathT2* transform)
{
    Rect r;
    r.min = mathv2_transform(rect.min, transform);
    r.max = mathv2_transform(rect.max, transform);
    return r;
}

inline void
rect_to_center_size(const Rect rect, MathV2* center, MathV2* size)
{
    CHECK(center);
    CHECK(size);
    *center = (MathV2){
        (rect.max.x + rect.min.x) * 0.5f,
        (rect.max.y + rect.min.y) * 0.5f
    };
    *size = (MathV2){
        rect.max.x - rect.min.x,
        rect.max.y - rect.min.y
    };
}

inline Rect
rect_normalize(const Rect rect) {
    return (Rect){
        MINIMUM(rect.min.x, rect.max.x),
        MINIMUM(rect.min.y, rect.max.y),
        MAXIMUM(rect.min.x, rect.max.x),
        MAXIMUM(rect.min.y, rect.max.y),
    };
}

inline int
rect_is_empty(const Rect rect, float epsilon) {
    return (
        fabsf(rect.max.x - rect.min.x) < epsilon &&
        fabsf(rect.max.y - rect.min.y) < epsilon
    );
}

inline Rect
rect_union(const Rect a, const Rect b) {
    return (Rect){
        MINIMUM(a.min.x, b.min.x),
        MINIMUM(a.min.y, b.min.y),
        MAXIMUM(a.max.x, b.max.x),
        MAXIMUM(a.max.y, b.max.y),
    };
}

//
//
//

inline Margin
margin_make(float left, float top, float right, float bottom) {
    Margin margin;
    margin.left = left;
    margin.top = top;
    margin.right = right;
    margin.bottom = bottom;
    return margin;
}

inline Margin
margin_add(const Margin a, const Margin b) {
    return (Margin){
        a.left + b.left,
        a.top + b.top,
        a.right + b.right,
        a.bottom + b.bottom
    };
}

inline float
margin_sum_horizontal(const Margin* margin) {
    return margin->left + margin->right;
}

inline float
margin_sum_vertical(const Margin* margin) {
    return margin->top + margin->bottom;
}

inline int
margin_is_zero(const Margin m) {
    return mathf_is_zero(m.left) && mathf_is_zero(m.top) && mathf_is_zero(m.right) && mathf_is_zero(m.bottom);
}

inline Margin
margin_mul_1(const Margin m, float x) {
    return (Margin){
        .left = m.left * x,
        .top = m.top * x,
        .right = m.right * x,
        .bottom = m.bottom * x,
    };
}

inline int
margin_is_equal(const Margin a, const Margin b) {
    return (
        (a.left == b.left) &&
        (a.top == b.top) &&
        (a.right == b.right) &&
        (a.bottom == b.bottom)
    );
}

//
// Colors
//

typedef union RGBA8 {
    struct {
        uint8_t r, g, b, a;
    };
    uint8_t E[4];
    uint32_t V;
}
RGBA8;

typedef union RGBAF {
    struct { float r, g, b, a; };
    float E[4];
} RGBAF;

typedef Slice(RGBA8) RGBA8Slice;


#define RGBA8_FROM_HEX(Hex)        \
    {                              \
        (((0x##Hex) >> 24) & 0xFF), \
        (((0x##Hex) >> 16) & 0xFF), \
        (((0x##Hex) >> 8) & 0xFF),  \
        (((0x##Hex) >> 0) & 0xFF),  \
    }

static const RGBA8 RGBA8_TRANSPARENT = { 0x00, 0x00, 0x00, 0x00 };
static const RGBA8 RGBA8_WHITE       = { 0xFF, 0xFF, 0xFF, 0xFF };
static const RGBA8 RGBA8_BLACK       = { 0x00, 0x00, 0x00, 0xFF };
static const RGBA8 RGBA8_RED         = { 0xFF, 0x00, 0x00, 0xFF };
static const RGBA8 RGBA8_GREEN       = { 0x00, 0xFF, 0x00, 0xFF };
static const RGBA8 RGBA8_BLUE        = { 0x00, 0x00, 0xFF, 0xFF };
static const RGBA8 RGBA8_YELLOW      = { 0xFF, 0xFF, 0x00, 0xFF };
static const RGBA8 RGBA8_CYAN        = { 0x00, 0xFF, 0xFF, 0xFF };
static const RGBA8 RGBA8_GRAY        = { 0x60, 0x60, 0x60, 0xFF };
static const RGBA8 RGBA8_PURPLE      = { 0xFF, 0x00, 0xFF, 0xFF };
static const RGBA8 RGBA8_GRAY_LIGHT  = { 0xc0, 0xc0, 0xc0, 0xFF };

inline RGBA8
rgba8_make(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    RGBA8 rgba8 = { r, g, b, a };
    return rgba8;
}

inline RGBA8
rgba8_with_alpha_f(RGBA8 rgba8, float a) {
    rgba8.a = (uint8_t)(255 * a);
    return rgba8;
}

//
// Files
//

// TODO: Doesn't return correct error message when the file is not found.
// TODO: Make it use 'String'.
bool file_read(ByteArray* buffer, const char *path, Error* error);
bool file_write(const char *path, const void *blob, iptr size, Error* error);

//
// Resources
//

void* resource_get(void* module, const char* name, size_t* size);

//
// TTY Formatting
//

#define TTY_RESET "\x1b[m"
#define TTY_INK(Ink) "\x1b[" #Ink "m"

//
// LZW
//

// Map we use currently is approx. 10-30% faster than binary tree for encoding.

// #define LZ_DEBUG

#define LZFLUSH_F(Name) int Name(uint8_t* data, iptr size, void* user)
typedef LZFLUSH_F(LZFlushF);

typedef Map LZMap;
typedef Array(uint64_t) LZSequences;
typedef Array(iptr) LZLengths;

typedef struct LZWriter
{
    // Map uses 0 value as 'empty' flag, hence we must
    // go around that by storing values other than 0:
    //
    // Map#1: WHATEVER VALUE WE'RE PUTTING TO THE MAP
    //    WE MUST INCREASE IT BY 1
    // Map#2: WHATEVER VALUE WE'RE GETTING FROM THE MAP
    //    WE MUST DECREASE IT BY 1

    LZMap seq_map;
#ifdef LZ_DEBUG
    LZSequences seq_array;
#endif

    // Output bits.
    uint32_t bits;
    uint32_t bits_count;
    uint32_t bits_size;
    uint32_t bits_min;
    uint32_t bits_max;

    // Output buffer.
    uint8_t *buffer;
    iptr buffer_capacity;
    iptr buffer_count;
    iptr buffer_offset;

    // Output callback.
    LZFlushF* flushf;
    void* flushf_user;
}
LZWriter;

typedef struct LZReader
{
    LZSequences seq_array;
    LZLengths seq_lengths;

    // Input buffer.
    uint8_t* data;
    uint8_t* data_end;

    // Output bits.
    uint32_t bits;
    uint32_t bits_count;
    uint32_t bits_size;
    uint32_t bits_min;
    uint32_t bits_max;
}
LZReader;

#endif // CO_LIB_H_INCLUDED