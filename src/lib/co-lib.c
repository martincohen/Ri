#include "co-lib.h"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include <stb_printf.h>

#if defined(SYSTEM_WINDOWS)
    // COMMENTED: WIN32_LEAN_AND_MEAN would make dsound.h not compile because of missing WAVEFORMATEX.
    // #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #define SYSTEM_PAGE_SIZE 4096
    int _fltused;
#else
    #include <sys/time.h>
    #include <sys/mman.h>
#endif

//
//
//

// TODO: For security reasons, a UTF-8 decoder must not accept UTF-8
// sequences that are longer than necessary to encode a character.
// https://www.cl.cam.ac.uk/~mgk25/unicode.html

// TODO: See utf8 test data in section 'Where do I find nice UTF-8
// example files?' of: https://www.cl.cam.ac.uk/~mgk25/unicode.html

// TODO: Use UTF8 only on input or output. Internally always use
// UTF-16 (or UTF-32).

//
// UTF32
//

UTF_CALLBACK_F(utf32_to_utf8_callback_)
{
    CharArray* utf8 = arg;

    char32_t rune;
    for (iptr i = 0; i < count; ++i) {
        rune = runes[i];
        if (rune >= (1L << 16)) {
            array_push(utf8, 0xf0 |  (rune >> 18));
            array_push(utf8, 0x80 | ((rune >> 12) & 0x3f));
            array_push(utf8, 0x80 | ((rune >>  6) & 0x3f));
            array_push(utf8, 0x80 | ((rune >>  0) & 0x3f));
        } else if (rune >= (1L << 11)) {
            array_push(utf8, 0xe0 |  (rune >> 12));
            array_push(utf8, 0x80 | ((rune >>  6) & 0x3f));
            array_push(utf8, 0x80 | ((rune >>  0) & 0x3f));
        } else if (rune >= (1L << 7)) {
            array_push(utf8, 0xc0 |  (rune >>  6));
            array_push(utf8, 0x80 | ((rune >>  0) & 0x3f));
        } else {
            array_push(utf8, rune);
        }
    }

    return runes;
}


UTF_CALLBACK_F(utf32_to_utf16_callback_)
{
    Char16Array* utf16 = arg;
    char32_t rune;
    for (iptr i = 0; i < count; ++i) {
        rune = runes[i];
        if (rune & ~0xFFFF) {
            array_push(utf16, UTF_SURROGATE_H(rune));
            array_push(utf16, UTF_SURROGATE_L(rune));
        } else {
            array_push(utf16, rune);
        }
    }
    return runes;
}

UTF_CALLBACK_F(utf32_to_utf32_callback_)
{
    Char32Array* utf32 = arg;
    memcpy(
        array_push_n(utf32, count),
        runes,
        count * sizeof(char32_t)
    );
    return runes;
}

//
// UTF-8
//

//
// utf8_decode_wellons
// From https://github.com/skeeto/branchless-utf8/blob/master/utf8.h
//
// TODO: Reimplement in ASM:
// https://github.com/bdonlan/branchless-utf8/blob/master/test/decode.s
//

// bits | b1        | b2        | b3       | b4        | b5*        | b6*       | b7**      | b8*
// 7    | 0xxx xxxx
// 11   | 110x xxxx | 10xx xxxx
// 16   | 1110 xxxx | 10xx xxxx | 10xx xxxx
// 21   | 1111 0xxx | 10xx xxxx | 10xx xxxx | 10xx xxxx
// ---
// *Anything beyond this is not valid utf-8.
// --
// 26   | 1111 10xx | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx
// 31   | 1111 110x | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx
// --
// **Anything beyond this cannot be encoded as 32-bit number.
// ---
// 36   | 1111 1110 | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx
// 42   | 1111 1111 | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx | 10xx xxxx

// WARNING: Reads 4 bytes from utf8.
void*
utf8_decode_wellons(void *utf8, uint32_t *rune, int *error)
{
    static const char lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };
    static const int masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
    static const uint32_t mins[] = {4194304, 0, 128, 2048, 65536};
    static const int shiftc[] = {0, 18, 12, 6, 0};
    static const int shifte[] = {0, 6, 4, 2, 0};

    unsigned char *s = utf8;
    int len = lengths[s[0] >> 3];

    /* Compute the pointer to the next character early so that the next
     * iteration can start working on the next character. Neither Clang
     * nor GCC figure out this reordering on their own.
     */
    unsigned char *next = s + len + !len;

    /* Assume a four-byte character and load four bytes. Unused bits are
     * shifted out.
     */
    *rune  = (uint32_t)(s[0] & masks[len]) << 18;
    *rune |= (uint32_t)(s[1] & 0x3f) << 12;
    *rune |= (uint32_t)(s[2] & 0x3f) <<  6;
    *rune |= (uint32_t)(s[3] & 0x3f) <<  0;
    *rune >>= shiftc[len];

    /* Accumulate the various error conditions. */
    *error  = (*rune < mins[len]) << 6; // non-canonical encoding
    *error |= ((*rune >> 11) == 0x1b) << 7;  // surrogate half?
    *error |= (*rune > 0x10FFFF) << 8;  // out of range?
    *error |= (s[1] & 0xc0) >> 2;
    *error |= (s[2] & 0xc0) >> 4;
    *error |= (s[3]       ) >> 6;
    *error ^= 0x2a; // top two bits of each tail byte correct?
    *error >>= shifte[len];

    return next;
}

//      |....
//  a___|_---
//  aa__|__--
//  aaa_|___
//  aaaa|____

int
utf8_to_utf32_chunked(String* utf8, char32_t* utf32, iptr utf32_capacity, UTFCallbackF* callback, void* callback_arg)
{
    iptr utf32_i = 0;

    char* utf8_it = utf8->items;
    char* utf8_next = utf8->items;
    char* utf8_end = utf8_it + utf8->count - 4;

    int e = 0;

    int ret = UTF_Success;
    while (utf8_it < utf8_end)
    {
        utf8_next = (char*)utf8_decode_wellons(utf8_it, &utf32[utf32_i], &e);
        if (e) {
            ret = UTF_Error;
            break;
        }
        utf8_it = utf8_next;
        ++utf32_i;
        if (utf32_i == utf32_capacity) {
            utf32 = callback(utf32, utf32_i, callback_arg);
            utf32_i = 0;
            if (utf32 == 0) {
                return UTF_OutputStarving;
            }
        }
    }
    if (utf32_i > 0) {
        callback(utf32, utf32_i, callback_arg);
    }
    string_pop_front(utf8, utf8_it - utf8->items);
    return ret;
}

int
utf8_convert(String* utf8, String32* cache, UTFCallbackF* callback, void* array)
{
    if (utf8->count == 0) {
        return 1;
    }
    char32_t cache_local_[8192];
    String32 cache_local;
    if (cache == 0) {
        cache_local.items = cache_local_;
        cache_local.count = COUNTOF(cache_local_);
        cache = &cache_local;
    }
    int r = utf8_to_utf32_chunked(utf8, cache->items, cache->count, callback, array);
    if (r != UTF_Error)
    {
        CHECK(r == UTF_Success);
        if (utf8->count)
        {
            iptr remainder = utf8->count;
            CHECK(remainder <= 4); // TODO: Should this be 3?
            char aligned[8] = {0}; // TODO: Should this be 4?
            switch (remainder)
            {
                case 4: aligned[3] = utf8->items[3];
                case 3: aligned[2] = utf8->items[2];
                case 2: aligned[1] = utf8->items[1];
                case 1: aligned[0] = utf8->items[0];
            }
            String state;
            state.count = remainder + 4; // TODO: Should this be 3?
            state.items = aligned;
            // TODO: Perhaps use utf8_to_utf32_wellons directly here?
            r = utf8_to_utf32_chunked(&state, cache->items, cache->count, callback, array);
            CHECK(r == UTF_Success || r == UTF_Error);
            remainder = state.items - aligned;
            utf8->count -= remainder;
            utf8->items += remainder;
        }
    }
    return r;
}

int
utf8_to_utf16_(String* utf8, String32* cache, Char16Array* utf16) {
    return utf8_convert(utf8, cache, utf32_to_utf16_callback_, utf16);
}

int
utf8_to_utf32_(String* utf8, String32* cache, Char32Array* utf32) {
    return utf8_convert(utf8, cache, utf32_to_utf32_callback_, utf32);
}

iptr
utf8_skip_invalid_sequence(String* state) {
    if (state->count == 0) {
        return 0;
    }

    // Skip first byte.
    iptr i = 1;
    // Skip continuation bytes.
    for (; i < state->count && UTF8_BYTE_IS_C(state->items[i]); ++i) {};
    string_pop_front(state, i);

    return i;
}

int
utf8_get_next_rune(String* state, uint32_t* o_rune)
{
    int e;
    char* next;

    char cache[4];
    char* it = state->items;

    switch (state->count)
    {
        default:
            next = utf8_decode_wellons(it, o_rune, &e);
            break;

        case 3: cache[2] = it[2];
        case 2: cache[1] = it[1];
        case 1: cache[0] = it[0];
            it = cache;
            next = utf8_decode_wellons(it, o_rune, &e);
            e = e || (next - it) > state->count;
            break;

        case 0:
            *o_rune = 0;
            return 0;
    }


    if (e) {
        string_pop_front(state, 1);
        *o_rune = RUNE_REPLACEMENT;
    } else {
        string_pop_front(state, next - it);
    }

    return 1;
}

#if 0
int
utf8_peek_next_rune(String* state, uint32_t* o_rune)
{
    if (state->count == 0) {
        return 0;
    }

    int e;
    char* next;
    char* it = state->items;
    switch (state->count)
    {
        default:
            next = utf8_decode_wellons(it, o_rune, &e);
            break;
        case 3:

    }

    if (e) {
        return 0;
    }

    string_pop_front(state, next - it);
    return 1;
}
#endif

//
// UTF-16
//

int
utf16_convert(String16* utf16, String32* cache, UTFCallbackF* callback, void* array)
{
    String32 cache_local;
    if (cache == 0) {
        cache_local.items = _alloca(8192 * sizeof(char32_t));
        cache_local.count = 8192;
        cache = &cache_local;
    }

    iptr utf32_i = 0;
    char32_t* utf32_items = cache->items;
    iptr utf32_capacity = cache->count;

    iptr utf16_i = 0;
    iptr utf16_count = utf16->count - 1;
    char16_t *utf16_items = utf16->items;
    char16_t u0, u1;
    int ret = UTF_Success;
    for (; utf16_i < utf16_count; ++utf16_i)
    {
        u0 = utf16_items[utf16_i];
        if (u0 >= 0xD800U && u0 <= 0xDFFFU) {
            u1 = utf16_items[utf16_i + 1];
            if (u0 > 0xDFFFU || u1 < 0xDC00U || u1 > 0xDFFFU) {
                ret = UTF_Error;
                goto end;
            }
            utf32_items[utf32_i] = (
                ((u0 - 0xD800U) << 10) +
                ((u1 - 0xDC00U) + 0x10000U)
            );
            ++utf32_i;
            ++utf16_i;
        } else {
            utf32_items[utf32_i] = u0;
            ++utf32_i;
        }

        if (utf32_i == utf32_capacity) {
            utf32_items = callback(utf32_items, utf32_i, array);
            utf32_i = 0;
            if (utf32_items == 0) {
                return UTF_OutputStarving;
            }
        }
    }

    if (ret == UTF_Success)
    {
        if (utf16->count != utf16_i)
        {
            ASSERT((utf16->count - utf16_i) == 1);
            u0 = utf16_items[utf16_i];
            if (u0 >= 0xD800U && u0 <= 0xDFFFU) {
                ret = UTF_Error;
                goto end;
            } else {
                utf32_items[utf32_i] = u0;
                ++utf32_i;
                ++utf16_i;
            }
        }
        if (utf32_i) {
            callback(utf32_items, utf32_i, array);
        }
    }

end:;
    utf16->items += utf16_i;
    utf16->count -= utf16_i;

    return ret;
}

int
utf16_to_utf8_(String16* utf16, String32* cache, CharArray* utf8)
{
    return utf16_convert(utf16, cache, utf32_to_utf8_callback_, utf8);
}

int
utf16_to_utf32(String16* utf16, String32* cache, Char32Array* utf32)
{
    return utf16_convert(utf16, cache, utf32_to_utf32_callback_, utf32);
}

//
//
//

//
//
//

// TODO: Remove 'ascii_to_utf16_naive'
void
ascii_to_utf16_naive(const char* s8, iptr s8_length, Char16Array* s16)
{
    if (s8_length == -1)
    {
        const char* s8_it = s8;
        for (; *s8_it; ++s8_it) {
            array_push(s16, *s8_it);
        }
    }
    else
    {
        CHECK(s8_length >= 0);
        const char* s8_it = s8;
        for (; s8_length; --s8_length, ++s8_it) {
            array_push(s16, *s8_it);
        }
    }

    // array_push(s16, 0);
}

// TODO: Remove 'utf16_to_utf8_naive'
void
utf16_to_utf8_naive(const char16_t* s16, iptr s16_length, CharArray* s8)
{
    if (s16_length == -1)
    {
        const char16_t* s16_it = s16;
        for (; *s16_it; ++s16_it) {
            array_push(s8, (char)*s16_it);
        }
    }
    else
    {
        CHECK(s16_length >= 0);
        const char16_t* s16_it = s16;
        for (; s16_length; --s16_length, ++s16_it) {
            array_push(s8, (char)*s16_it);
        }
    }

    // array_push(s8, 0);
}

//
// String
//

String STRING_EMPTY = {0};

//
// String
//

const char*
string_find_char_first_r(const char *it, const char *end, char c)
{
    CHECK(it);
    CHECK(end >= it);
    for (; it < end; ++it) {
        if (*it == c) {
            return it;
        }
    }
    return 0;
}

const char*
string_find_char_last_r(const char *it, const char *begin, char c)
{
    CHECK(it);
    CHECK(begin);

    for (; it >= begin; --it) {
        if (*it == c) {
            return it;
        }
    }
    return 0;
}

int
string_read_line(String* state, String* o_line)
{
    if (state->count == 0) {
        return 0;
    }
    iptr i = 0;
    // Handle LF.
    while (i < state->count && state->items[i] != '\n') ++i;
    o_line->items = state->items;
    o_line->count = i;
    // Handle CR LF.
    if (o_line->count > 0 && o_line->items[o_line->count - 1] == '\r') {
        --o_line->count;
    }
    if (i < state->count) ++i;
    state->items += i;
    state->count -= i;
    return 1;
}

//
//
//

String
chararray_push(CharArray* array, String string)
{
    if (string_is_empty(&string)) {
        return S(array->items + array->count, 0);
    }

    String slice = S(
        array_push_n(array, string.count),
        string.count
    );
    memcpy(slice.items, string.items, string.count);

    return slice;
}

String16
char16array_push(Char16Array* array, String16 string)
{
    if (string16_is_empty(&string)) {
        return Su(array->items + array->count, 0);
    }

    String16 slice = Su(
        array_push_n(array, string.count),
        string.count
    );
    memcpy(slice.items, string.items, string.count * sizeof(char16_t));

    return slice;
}

CBPRINTF(chararray_push_f_cb_)
{
    chararray_push(user, S(buf, length));
    return buf;
}

String
chararray_push_f(CharArray* array, const char* format, ...)
{
    iptr start = array->count;
    char buf[STB_SPRINTF_MIN + 1];
    va_list args;
    va_start(args, format);
    cbprintfv(&chararray_push_f_cb_, array, buf, format, args);
    va_end(args);

    return S(array->items + start, array->count - start);
}

String
chararray_push_fv(CharArray* array, const char* format, va_list args)
{
    iptr start = array->count;
    char buf[STB_SPRINTF_MIN + 1];
    cbprintfv(&chararray_push_f_cb_, array, buf, format, args);
    return S(array->items + start, array->count - start);
}

//
// String fuzzy
//


iptr
string_fuzzy_compare(String needle, String haystack)
{
    if (string_is_empty(&needle) || string_is_empty(&haystack)) {
        return StringFuzzyCompare_NoMatch;
    }

    iptr ni = 0;
    iptr hi = 0;

    // COMMENTED: Ignores offset.
    // for (; hi < haystack.count && ni < needle.count && needle.items[ni] != haystack.items[hi]; ++hi) {
    //     // score += 1 - m;
    // }

    int m, score = 0;
    for (; hi < haystack.count && ni < needle.count; ++hi) {
        m = needle.items[ni] == haystack.items[hi];
        ni += m;
        score -= 1 - m;
    }

    if (ni != needle.count) {
        return StringFuzzyCompare_NoMatch;
    }

    if (needle.count == haystack.count && score == 0) {
        return StringFuzzyCompare_FullMatch;
    }

    return score;
}

iptr
string_fuzzy_compare_r(String needle, String haystack)
{
    if (string_is_empty(&needle) || string_is_empty(&haystack)) {
        return StringFuzzyCompare_NoMatch;
    }

    iptr ni = needle.count - 1;
    iptr hi = haystack.count - 1;

    // COMMENTED: Ignores offset.
    // for (; hi >= 0 && ni >= 0 && needle.items[ni] != haystack.items[hi]; --hi) {
    //     // score += 1 - m;
    // }

    int m, score = 0;
    for (; hi >= 0 && ni >= 0; --hi) {
        m = needle.items[ni] == haystack.items[hi];
        ni -= m;
        score -= 1 - m;
    }

    if (ni != -1) {
        return StringFuzzyCompare_NoMatch;
    }

    if (needle.count == haystack.count && score == 0) {
        return StringFuzzyCompare_FullMatch;
    }

    return score;
}

//
// Error
//

void
error_clear(Error* error)
{
    error->code = 0;
    array_clear(&error->message);
}

void
error_set(Error* error, int code, const char* format, ...)
{
    if (error) {
        array_clear(&error->message);
        if (format) {
            va_list args;
            va_start(args, format);
            chararray_push_fv(&error->message, format, args);
            va_end(args);
        }
        error->code = code;
    }
}

//
//
//

#if defined(SYSTEM_WINDOWS)
void
win32_error(const char* function)
{
    DWORD error = GetLastError();

    const char* message = 0;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&message,
        0, NULL
    );

    LOG_ERROR("%s failed: (%d) %s", function, error, message);

    LocalFree((void*)message);
}
#endif

//
//
//

#if defined(SYSTEM_WINDOWS)
static CBPRINTF(win32_logprintfv_debugger_)
{
    buf[length] = 0;
    OutputDebugStringA(buf);
    DWORD cbWritten;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, length, &cbWritten, 0);
    return buf;
}

static CBPRINTF(win32_logprintfv_stdout_)
{
    DWORD cbWritten;
    WriteFile((HANDLE)user, buf, length, &cbWritten, 0);
    return buf;
}
#else
static CBPRINTF(unix_logprintfv_file_)
{
    fwrite(buf, 1, length, (FILE*)user);
    return buf;
}
#endif

void
cbprintf(CBPrintF* f, void* user, const char* format, ...)
{
    char buf[STB_SPRINTF_MIN + 1];
    va_list args;
    va_start(args, format);
    cbprintfv(f, user, buf, format, args);
    va_end(args);
}

void
lvprintf(const char* format, va_list args)
{
    char buf[STB_SPRINTF_MIN + 1];
#if defined(SYSTEM_WINDOWS)
    if (IsDebuggerPresent()) {
        cbprintfv(win32_logprintfv_debugger_, 0, buf, format, args);
    } else {
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        cbprintfv(win32_logprintfv_stdout_, handle, buf, format, args);
    }
#else
    cbprintfv(unix_logprintfv_file_, stdout, buf, format, args);
#endif
}

void
lprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    lvprintf(format, args);
    va_end(args);
}

//
// Logging
//

void
log_assert_(const char *message, const char *expression, const char *function, const char *file, int line, ...)
{
    char e[4096];
    char *e_it = e;
    char *e_end = e + COUNTOF(e) - 2;
    while (e_it < e_end && expression) {
        if (*expression == '%') {
            *e_it++ = '%';
        }
        *e_it++ = *expression++;
    }
    *e_it = 0;

    char m[4096];
    stbsp_snprintf(
        m, COUNTOF(m),
        "ASSERTION FAILED:\n"
        "----------------------------\n"
        "         message: %s\n"
        "      expression: %s\n"
        "        function: %s\n"
        "            file: %s\n"
        "            line: %d\n",
        message,
        e,
        function,
        file,
        line);

    va_list args;
    va_start(args, line);
    lvprintf(m, args);
    va_end(args);
}

//
// Performance
//

double
perf_get()
{
#if defined(SYSTEM_WINDOWS)
    static int64_t frequency_i = 0;
    static double frequency;
    static int64_t first = 0;
    if (frequency_i == 0) {
        QueryPerformanceFrequency((LARGE_INTEGER *)&frequency_i);
        frequency = (double)frequency_i;
        QueryPerformanceCounter((LARGE_INTEGER *)&first);
    }
    int64_t counter;
    QueryPerformanceCounter((LARGE_INTEGER *)&counter);
    return (double)((double)(counter-first) / frequency);
#else
    #define PERF_GET_FREQUENCY 1000000
    static int first = 1;
    static double first_value = 0;
    struct timeval time;
    gettimeofday(&time, NULL);
    double value = ((time.tv_usec + time.tv_sec * PERF_GET_FREQUENCY) / (double)PERF_GET_FREQUENCY) - first_value;
    if (first == 1) {
        first = 0;
        first_value = value;
        value = 0;
    }
    #undef PERF_GET_FREQUENCY
    return value;
#endif
}

//
//
//

void*
heap_alloc(iptr size) {
#if defined(SYSTEM_WINDOWS)
    return HeapAlloc(GetProcessHeap(), 0, (SIZE_T)size);
#else
    return malloc(size);
#endif
}

void*
heap_alloc_zeroed(iptr size) {
    void* ptr = heap_alloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void*
heap_realloc(const void* ptr, iptr size) {
#if defined(SYSTEM_WINDOWS)
    if (ptr == 0) {
        ptr = HeapAlloc(GetProcessHeap(), 0, (SIZE_T)size);
    } else {
        ptr = HeapReAlloc(GetProcessHeap(), 0, (LPVOID)ptr, (SIZE_T)size);
    }
#else
    ptr = realloc((void*)ptr, size);
#endif
    ASSERT(ptr);
    CHECK((((uptr)ptr) & 7) == 0);
    return (void*)ptr;
}

void
heap_free(const void* ptr) {
#if defined(SYSTEM_WINDOWS)
    HeapFree(GetProcessHeap(), 0, (void*)ptr);
#else
    free((void*)ptr);
#endif
}

//
// Virtual memory
//

void*
virtual_reserve(void* ptr, iptr size)
{
#if defined(SYSTEM_WINDOWS)
    ptr = VirtualAlloc(ptr, size, MEM_RESERVE, PAGE_NOACCESS);
    ASSERT(ptr);
#else
    ptr = mmap((void*)ptr, size,
               PROT_NONE,
               MAP_PRIVATE | MAP_ANON,
               -1, 0);
    ASSERT(ptr != MAP_FAILED);
    // msync(ptr, size, MS_SYNC|MS_INVALIDATE);
#endif
    return ptr;
}

void*
virtual_commit(void* ptr, iptr size)
{
#if defined(SYSTEM_WINDOWS)
    ptr = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
    ASSERT(ptr);
#else
    // TODO: MAP_PRIVATE or MAP_SHARED?
    ptr = mmap(ptr, size,
               PROT_READ | PROT_WRITE,
               // MAP_ANONYMOUS | MAP_PRIVATE,
               MAP_FIXED | MAP_SHARED | MAP_ANON,
               -1, 0);
    ASSERT(ptr != MAP_FAILED);
    msync(ptr, size, MS_SYNC|MS_INVALIDATE);
#endif
    return ptr;
}

void
virtual_decommit(void* ptr, iptr size)
{
#if defined(SYSTEM_WINDOWS)
    VirtualFree(ptr, size, MEM_DECOMMIT);
#else
    // instead of unmapping the address, we're just gonna trick
    // the TLB to mark this as a new mapped area which, due to
    // demand paging, will not be committed until used.
    mmap(ptr, size,
         PROT_NONE,
         MAP_FIXED | MAP_PRIVATE | MAP_ANON,
         -1, 0);
    msync(ptr, size, MS_SYNC|MS_INVALIDATE);
#endif
}

void
virtual_free(void* ptr, iptr size)
{
#if defined(SYSTEM_WINDOWS)
    UNUSED(size);
    VirtualFree((void*)ptr, 0, MEM_RELEASE);
#else
    msync(ptr, size, MS_SYNC);
    munmap(ptr, size);
#endif
}

void*
virtual_alloc(void* ptr, iptr size)
{
    return virtual_commit(virtual_reserve(ptr, size), size);
}

//
// Collections
//

iptr
collection_index_of(void* collection, ValueBox needle, iptr search_start, iptr search_end, CollectionCompareF* comparef)
{
    iptr m;
    int c;

    while (search_start < search_end)
    {
        m = search_start + ((search_end - search_start) / 2);
        c = comparef(collection, m, needle);
        if (c == 0) {
            return m;
        } else if (c < 0) {
            search_start = m + 1;
        } else {
            search_end = m;
        }
    }

    return -1;
}

iptr
collection_upper_bound(void* collection, ValueBox needle, iptr search_start, iptr search_end, CollectionCompareF* comparef)
{
#if 0
    iptr it, step, count = search_end - search_start;
    while (count > 0)
    {
        it = search_start;
        step = count / 2;
        it += step;
        // if (collection[it] <= needle) ...
        if (comparef(collection, it, needle) <= 0) {
            search_start = ++it;
            count -= step + 1;
        } else {
            count = step;
        }
    }

    return search_start;
#else
    while (search_start < search_end)
    {
        // NOTE: This ensures we won't jump out of 64-bit range. Original 'm = (search_end + search_start) / 2' would.
        iptr m = search_start + ((search_end - search_start) / 2);
        if (comparef(collection, m, needle) <= 0) {
            search_start = m + 1;
        } else {
            search_end = m;
        }
    }

    return search_start;
#endif
}

iptr
collection_lower_bound(void* collection, ValueBox needle, iptr search_start, iptr search_end, CollectionCompareF* comparef)
{
#if 0
    iptr it, step, count = search_end - search_start;
    while (count > 0)
    {
        it = search_start;
        step = count / 2;
        it += step;
        // if (collection[it] < needle) ...
        if (comparef(collection, it, needle) < 0) {
            search_start = ++it;
            count -= step + 1;
        } else {
            count = step;
        }
    }

    return search_start;
#else
    while (search_start < search_end)
    {
        // NOTE: This ensures we won't jump out of 64-bit range. Original 'm = (search_end + search_start) / 2' would.
        iptr m = search_start + ((search_end - search_start) / 2);
        if (comparef(collection, m, needle) < 0) {
            search_start = m + 1;
        } else {
            search_end = m;
        }
    }

    return search_start;
#endif
}

//
// Hashing
//

uint64_t
hash_blob_begin()
{
    return 0xcbf29ce484222325;
}

uint64_t
hash_blob_add(uint64_t hash, const void* ptr, iptr length)
{
    const uint8_t* buf = (const uint8_t*)ptr;
    for (iptr i = 0; i < length; i++) {
        hash ^= buf[i];
        hash *= 0x100000001b3;
        hash ^= hash >> 32;
    }
    return hash;
}

uint64_t
hash_blob(const void* ptr, iptr len)
{
    uint64_t x = 0xcbf29ce484222325;
    const uint8_t* buf = (const uint8_t*)ptr;
    for (iptr i = 0; i < len; i++) {
        x ^= buf[i];
        x *= 0x100000001b3;
        x ^= x >> 32;
    }
    return x;
}

uint64_t
hash_uint64(uint64_t x)
{
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

uint64_t
hash_ptr(const void *ptr)
{
    return hash_uint64((uintptr_t)ptr);
}

uint64_t
hash_mix(uint64_t x, uint64_t y)
{
    x ^= y;
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

//
// Map
//

void
map_init(Map* map, iptr count)
{
    iptr capacity = 2 * count;
    CHECK(iptr_is_power_of_two(capacity));

    memset(map, 0, sizeof(Map));
    map->keys = heap_alloc(capacity * sizeof(ValueScalar)),
    map->values = heap_alloc(capacity * sizeof(ValueScalar)),
    memset(map->keys, 0, capacity * sizeof(ValueScalar));
    map->capacity = capacity;
}

void
map_purge(Map* map)
{
    heap_free(map->keys);
    heap_free(map->values);
}

void
map_clear(Map* map)
{
    map->count = 0;
    memset(map->keys, 0, map->capacity * sizeof(ValueScalar));
}

static void
map_grow__(Map *map, size_t new_cap)
{
    new_cap = MAXIMUM(new_cap, 16);
    // TODO: Single allocation.
    Map new_map = {
        .keys = heap_alloc(new_cap * sizeof(ValueScalar)),
        .values = heap_alloc(new_cap * sizeof(ValueScalar)),
        .capacity = new_cap,
    };
    memset(new_map.keys, 0, new_cap * sizeof(ValueScalar));
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->keys[i].u64 != 0) {
            map_put(&new_map, map->keys[i], map->values[i]);
        }
    }
    heap_free((void *)map->keys);
    heap_free(map->values);
    *map = new_map;
}

//
//
//

iptr
map_find(Map* map, const ValueScalar key)
{
    if (key.u64 == 0 || map->count == 0) {
        return -1;
    }
    CHECK(iptr_is_power_of_two(map->capacity));
    uint64_t i = (uint64_t)hash_uint64(key.u64);
    CHECK(map->count < map->capacity);
    for (;;) {
        i &= map->capacity - 1;
        if (map->keys[i].u64 == key.u64) {
            CHECK(i < INT64_MAX);
            return (iptr)i;
        } else if (map->keys[i].u64 == 0) {
            return -1;
        }
        i++;
    }
    return -1;
}

ValueScalar
map_get(Map *map, const ValueScalar key)
{
    uint64_t i = map_find(map, key);
    if (i == -1) {
        return (ValueScalar){0};
    }
    return map->values[i];
}

int
map_put(Map *map, const ValueScalar key, const ValueScalar val)
{
    ASSERT(key.u64);

    if (map->capacity == 0 || (2 * map->count) > map->capacity) {
        map_grow__(map, 2 * map->capacity);
    }
    CHECK((2 * map->count) <= map->capacity);
    CHECK(iptr_is_power_of_two(map->capacity));
    uint64_t i = (uint64_t)hash_uint64(key.u64);
    for (;;) {
        i &= map->capacity - 1;
        if (map->keys[i].u64 == 0) {
            map->count++;
            map->keys[i] = key;
            map->values[i] = val;
            return 1;
        } else if (map->keys[i].u64 == key.u64) {
            map->values[i] = val;
            return 2;
        }
        i++;
    }

    // NOT REACHED
    return 0;
}

//
// Intern
//

typedef struct InternItem {
    size_t length;
    struct InternItem *next;
    char s[];
} InternItem;

void
intern_init(Intern* intern) {
    arena_init(&intern->arena, KILOBYTES(64));
}

void
intern_purge(Intern* intern) {
    arena_purge(&intern->arena);
    map_purge(&intern->map);
}

const char*
intern_put_r(Intern* I, const char* start, const char* end)
{
    size_t length = end - start;
    uint64_t hash = hash_blob(start, length);
    ValueScalar key = { .u64 = hash ? hash : 1 };
    InternItem *intern = map_get(&I->map, key).ptr;
    for (InternItem *it = intern; it; it = it->next) {
        if (it->length == length && (it->s == start || (strncmp(it->s, start, length) == 0))) {
            return it->s;
        }
    }
    InternItem *new_intern = arena_push(
        &I->arena,
        offsetof(InternItem, s) + length + 1,
        ALIGNOF(InternItem)
    );
    new_intern->length = length;
    new_intern->next = intern;
    memcpy(new_intern->s, start, length);
    new_intern->s[length] = 0;
    map_put(&I->map, key, (ValueScalar){ .ptr = new_intern });
    return new_intern->s;
}

const char*
intern_put_c(Intern* I, const char* s, iptr s_length) {
    if (s_length == -1) {
        s_length = strlen(s);
    }
    return intern_put_r(I, s, s + s_length);
}

int
intern_has(Intern* I, String string) {
    uint64_t hash = hash_blob(string.items, string.count);
    ValueScalar key = { .u64 = hash ? hash : 1 };
    InternItem *intern = map_get(&I->map, key).ptr;
    for (InternItem *it = intern; it; it = it->next) {
        if (it->length == string.count && (it->s == string.items || (strncmp(it->s, string.items, string.count) == 0))) {
            return 1;
        }
    }
    return 0;
}

static Intern INTERN_DEFAULT = {0};

int id_has(String string) {
    return intern_has(&INTERN_DEFAULT, string);
}

String id_make(String slice) {
    // ASSERT(thread_is_main());
    String id = {
        .items = (char*)intern_put_c(&INTERN_DEFAULT, slice.items, slice.count),
        .count = slice.count,
    };
    return id;
}

//
// Arena
//

static void
arenablock_pool_(Arena* arena, uptr block)
{
    array_push(&arena->blocks_pool, block);
}

static void
arenablock_push_(Arena* arena)
{
    ASSERT(arena->it <= arena->end);

    if (arena->block_size == 0) {
        arena->block_size = 16384;
    }
    if (arena->blocks_pool.count > 0) {
        uptr block = array_pop(&arena->blocks_pool);
        arena->begin = CAST(void*, block);
    } else {
        arena->begin = virtual_alloc(0, arena->block_size);
    }
    arena->head += void_minus(arena->end, arena->it);
    arena->it = arena->begin;
    arena->end = void_plus(arena->begin, arena->block_size);

    array_push(&arena->blocks, (uptr)arena->begin);
}

// TODO: Add support for global block pool.

void
arena_init(Arena* arena, iptr block_size)
{
    memset(arena, 0, sizeof(Arena));
    arena->block_size = block_size;
    arenablock_push_(arena);
}

void
arena_clear(Arena* arena)
{
    // TODO: Keep the blocks alive.
    void** ptr;
    ptr = (void**)arena->blocks.items;
    for (iptr i = 0; i < arena->blocks.count; ++i) {
        virtual_free(ptr[i], arena->block_size);
    }
    array_clear(&arena->blocks);
    memset(arena, 0, sizeof(Arena));
}

void
arena_purge(Arena* arena)
{
    void** ptr;
    ptr = (void**)arena->blocks.items;
    for (iptr i = 0; i != arena->blocks.count; ++i) {
        virtual_free(ptr[i], arena->block_size);
    }
    array_purge(&arena->blocks);
    memset(arena, 0, sizeof(Arena));
}

void*
arena_push(Arena* arena, iptr size, iptr align)
{
    CHECK(arena);
    CHECK(arena->block_size);
    CHECK(size >= 0);
    CHECK(align > 0);
    CHECK(align <= 16);
    CHECK(iptr_is_power_of_two(align));

    ASSERT(arena->begin == 0 || size <= arena->block_size);

    void* it = void_align_forward(arena->it, align);
    if (arena->begin == 0 || (void*)void_plus(it, size) > arena->end) {
        arenablock_push_(arena);
        it = arena->it;
    } else {
        size += void_minus(it, arena->it);
    }

    arena->it = void_plus(arena->it, size);
    arena->head += size;
    return it;
}

iptr
arena_copy(Arena* arena, void* buffer, iptr start, iptr count)
{
    CHECK(arena);
    CHECK(arena->begin);

    iptr end = start;
    if (count == 0) {
        return start;
    } else if (count < 0) {
        start += count;
    } else {
        end += count;
    }

    CHECK(start >= 0);
    CHECK(start < end);
    CHECK(end <= arena->head);

    iptr block_first = start / arena->block_size;
    iptr block_last = end / arena->block_size;

    iptr buffer_size = end - start;
    iptr offset = start - block_first * arena->block_size;
    iptr size;
    for (iptr i = block_first; i <= block_last; ++i) {
        size = MINIMUM(buffer_size, arena->block_size - offset);
        memcpy(
            buffer,
            (void*)(array_at(&arena->blocks, i) + offset),
            size
        );
        buffer = void_plus(buffer, size);
        buffer_size -= size;
        offset = 0;
    }

    ASSERT(buffer_size == 0);

    if (count < 0) {
        return start;
    } else {
        return end;
    }
}

iptr
arena_pour(Arena* arena, const void* data, iptr data_size, iptr data_align)
{
    // NOTE: arena_push() does all the checks.
    arena_push(arena, 0, data_align);

    iptr head = arena->head;

    iptr size;
    while (data_size > 0) {
        if (arena->it == arena->end) {
            arenablock_push_(arena);
        }
        size = MINIMUM(data_size, void_minus(arena->end, arena->it));
        ASSERT(size > 0);
        memcpy(arena_push(arena, size, 1), data, size);
        data = void_plus(data, size);
        data_size -= size;
    }

    return head;
}

void
arena_rewind(Arena* arena, iptr head)
{
    CHECK(head <= arena->head);
    iptr block_index  = head / arena->block_size;
    uptr begin = array_at(&arena->blocks, block_index);
    iptr block_offset = head % arena->block_size;
    CHECK(block_offset <= arena->block_size);

    arena->begin = CAST(void*, begin);
    arena->end = void_plus(arena->begin, arena->block_size);
    arena->it = void_plus(arena->begin, block_offset);

    arena->head = head;
}

void
arena_truncate(Arena* arena, iptr end)
{
    iptr block = (end / arena->block_size) + 1;
    for (iptr i = block + 1; i < arena->blocks.count; ++i) {
        arenablock_pool_(arena, arena->blocks.items[i]);
    }
    array_resize(&arena->blocks, block);
    arena->begin = CAST(void*, arena->blocks.items[block - 1]);
    arena->it = void_plus(arena->begin, (end % arena->block_size));
    arena->end = void_plus(arena->begin, arena->block_size);
    arena->head = end;
}

// Arena iterator.

int
arenaiterator_read(ArenaIterator* it, Arena* arena, iptr start, iptr count)
{
    CHECK(it);
    CHECK(arena);
    CHECK(arena->begin);

    iptr end = start;
    if (count == 0) {
        return 0;
    } else if (count < 0) {
        start += count;
    } else {
        end += count;
    }

    CHECK(start >= 0);
    CHECK(start < end);
    CHECK(end <= arena->head);

    it->arena = arena;
    it->range.start = start;
    it->range.end = end;

    it->block.start = start / arena->block_size;
    it->block.end = end / arena->block_size;

    iptr block_offset = start - (it->block.start * arena->block_size);
    it->items = (uint8_t*)(arena->blocks.items[it->block.start] + block_offset);
    if (it->block.start == it->block.end) {
        it->count = count;
    } else {
        it->count = arena->block_size - block_offset;
    }

    return 1;
}

int
arenaiterator_next(ArenaIterator* it)
{
    if (it->block.start == it->block.end) {
        return 0;
    }
    CHECK(it->block.start < it->block.end);
    ++it->block.start;

    Arena* arena = it->arena;
    it->items = (uint8_t*)arena->blocks.items[it->block.start];
    if (it->block.start == it->block.end) {
        it->count = it->range.end - (it->block.start * arena->block_size);
    } else {
        it->count = arena->block_size;
    }

    return 1;
}

//
// Pool
//

void
pool_init(Pool* pool, Arena* arena, iptr size, iptr align)
{
    CHECK(pool);
    CHECK(arena);
    CHECK(size > 0);
    CHECK(align > 0);

    memset(pool, 0, sizeof(Pool));
    pool->arena = arena;
    pool->size = size;
    pool->align = align;
}

void
pool_purge(Pool* pool)
{
    array_purge(&pool->pool);
    if (pool->arena) {
        arena_purge(pool->arena);
    }
}

void*
pool_alloc(Pool* pool)
{
    CHECK(pool);
    CHECK(pool->arena);
    ASSERT(pool->size > 0);
    ASSERT(pool->align > 0);

    if (pool->pool.count != 0) {
        return (void*)array_pop(&pool->pool);
    } else {
        return arena_push(pool->arena, pool->size, pool->align);
    }
}

void
pool_dealloc(Pool* pool, void* ptr)
{
    CHECK(pool);
    array_push(&pool->pool, (uptr)ptr);
}

//
// Files
//

int
file_read(ByteArray* buffer, const char *path, Error* error)
{
#ifdef SYSTEM_WINDOWS
    int ret = 0;
    HANDLE file = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        error_set(error, ERROR_ID(File_Open), 0);
        ret = 0;
    } else {
        iptr file_size;
        GetFileSizeEx(file, (PLARGE_INTEGER)&file_size);
        uint8_t* blob = array_push_n(buffer, file_size + 1);
        array_resize(buffer, file_size);
        if (file_size) {
            DWORD read_size;
            if (ReadFile(file, blob, file_size, &read_size, NULL) == FALSE || read_size == 0) {
                error_set(error, ERROR_ID(File_Read), 0);
                ret = 0;
            } else {
                ret = 1;
            }
        }
        CloseHandle(file);
    }
    return ret;
#else
    FILE *file = fopen(path, "rb");
    if (!file) {
        error_set(error, ERROR_ID(File_Open), 0);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    iptr file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t* blob = array_push_n(buffer, file_size + 1);
    array_resize(buffer, file_size);
    if (file_size && fread(blob, file_size, 1, file) != 1) {
        error_set(error, ERROR_ID(File_Read), 0);
        fclose(file);
        return 0;
    }
    fclose(file);
    ((char*)blob)[buffer->count] = '\0';
    return 1;
#endif
}

int
file_write(const char *path, const void *blob, iptr size, Error* error)
{
    // TODO: Fill error.
#ifdef SYSTEM_WINDOWS
    int ret = 1;
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        error_set(error, ERROR_ID(File_Open), 0);
        ret = 0;
    } else {
        DWORD write_size;
        if (WriteFile(file, blob, size, &write_size, NULL) == 0 || write_size == 0) {
            error_set(error, ERROR_ID(File_Write), 0);
            ret = 0;
        }
        CloseHandle(file);
    }
    return ret;
#else
    FILE *file = fopen(path, "wb");
    if (!file) {
        error_set(error, ERROR_ID(File_Write), 0);
        return 0;
    }
    int ret = 1;
    size_t n = fwrite(blob, size, 1, file);
    if (n != 1) {
        error_set(error, ERROR_ID(File_Write), 0);
        ret = 0;
    }
    fclose(file);
    return ret;
#endif
}

//
// Binary serialization
//

#define DE_SERIALIZE_SCALAR_IMPL(Suffix, Type) \
    inline int serialize_ ## Suffix(FILE* file, Type value) { \
        CHECK(file); \
        return fwrite(&value, sizeof(Type), 1, file) == 1; \
    } \
    inline int deserialize_ ## Suffix(FILE* file, Type* o_value) { \
        CHECK(file); \
        return fread(o_value, sizeof(Type), 1, file) == 1; \
    }

DE_SERIALIZE_SCALAR_IMPL(int32, int32_t)
DE_SERIALIZE_SCALAR_IMPL(uint32, uint32_t)
DE_SERIALIZE_SCALAR_IMPL(int64, int64_t)
DE_SERIALIZE_SCALAR_IMPL(uint64, uint64_t)
DE_SERIALIZE_SCALAR_IMPL(float, float)
DE_SERIALIZE_SCALAR_IMPL(double, double)
DE_SERIALIZE_SCALAR_IMPL(iptr, iptr)

inline int serialize_string(FILE* file, const String* string) {
    CHECK(file);
    if (serialize_iptr(file, string->count)) {
        return fwrite(string->items, string->count, 1, file) == 1;
    }
    return 0;
}

inline size_t deserialize_string(FILE* file, CharArray* buffer) {
    CHECK(file);
    iptr count = 0;
    if (deserialize_iptr(file, &count)) {
        array_reserve(buffer, count + 1);
        buffer->count = count;
        if (fread(buffer->items, count, 1, file) == 1) {
            array_zero_term(buffer);
            return 1;
        }
    }
    return 0;
}

//
// LZW
//

#ifdef LZ_DEBUG
void
lzseq_dump_sequence(LZSequences* seq_array, uint32_t si, CharArray* buffer)
{
    uint32_t i = si;
    while (si) {
        uint64_t k = array_at(seq_array, si);
        array_insert(buffer, 0, k & 0xFF);
        si = k >> 32;
    }
}

void
lzseq_log_sequence(LZSequences* seq_array, uint32_t si)
{
    CharArray buffer = {0};
    lzseq_dump_sequence(seq_array, si, &buffer);
    LOG("%d: '%S' [%d]\n", si, buffer.slice, buffer.count);
    array_purge(&buffer);
}

void
lzseq_log_dictionary(LZSequences* seq_array)
{
    CharArray buffer = {0};
    for (uint32_t i = 256; i < seq_array->count; ++i) {
        array_clear(&buffer);
        lzseq_dump_sequence(seq_array, i, &buffer);
        LOG("%d: '%S' [%d]\n", i, buffer.slice, buffer.count);
    }
    array_purge(&buffer);
}
#endif

#define LZ_CHECK_BITS_RANGE_(Min, Max) \
    CHECK(Max >= Min); \
    CHECK(Max < 31); \
    CHECK(Min > 8);


//
// LZW Writer.
//

void
lzwriter_init(
    LZWriter* W,
    uint32_t bits_min,
    uint32_t bits_max,
    uint8_t* buffer,
    iptr buffer_capacity,
    LZFlushF* flushf,
    void* flushf_user
) {
    LZ_CHECK_BITS_RANGE_(bits_min, bits_max);
    memset(W, 0, sizeof(LZWriter));
    W->buffer = buffer;
    W->buffer_capacity = buffer_capacity;
    W->flushf = flushf;
    W->flushf_user = flushf_user;
    W->bits_min = bits_min;
    W->bits_max = bits_max + 1;
    map_init(&W->seq_map, 1ull << bits_max);
}

void
lzwriter_purge(LZWriter* W) {
    map_purge(&W->seq_map);
#ifdef LZ_DEBUG
    array_purge(&W->seq_array);
#endif
}

static int
lzwriter_write_(LZWriter* W, uint32_t si)
{
    W->bits |= si << W->bits_count;
    W->bits_count += W->bits_size;
    while (W->bits_count >= 8) {
        W->buffer[W->buffer_count++] = W->bits & 0xFF;
        if (W->buffer_count == W->buffer_capacity) {
            if (W->flushf(W->buffer, W->buffer_count, W->flushf_user) == 0) {
                return 0;
            }
            W->buffer_count = 0;
        }
        W->bits >>= 8;
        W->bits_count -= 8;
        W->buffer_offset++;
    }

    return 1;
}

void
lzwriter_map_reset(LZWriter* W)
{
    map_clear(&W->seq_map);
    for (uint32_t i = 0; i < 256; ++i) {
        map_put(&W->seq_map, (ValueScalar){ .u64 = (257ull << 32) | i }, (ValueScalar){ .u32 = i + 1 });
    }
}

int
lzwriter_push(LZWriter* W, void* data_, iptr data_size)
{
    uint8_t* data = data_;

    W->bits = 0;
    W->bits_size = W->bits_min;
    W->bits_count = 0;
    W->buffer_count = 0;
    W->buffer_offset = 0;

#ifdef LZ_DEBUG
    array_clear(&W->seq_array);
    for (iptr i = 0; i < 257; ++i) {
        array_push(&W->seq_array, i, i);
    }
#endif

    lzwriter_map_reset(W);

    // 256 is reserved.
    uint32_t count = 257;

    ValueScalar k;
    uint32_t byte, si1, si0 = 256;
    for (iptr i = 0; i < data_size; ++i)
    {
        byte = data[i];
        // Append 'byte' to the sequence.
        k.u64 = ((uint64_t)(si0 + 1) << 32) | byte;
        // Check if we have the sequence in the dictionary.
        si1 = map_get(&W->seq_map, k).u32;
        if (!si1) {
            // Write parent sequence's id.
            lzwriter_write_(W, si0);
            // Add this new sequence to dictionary.
            si1 = count++;
            // See Map#1 rule.
            map_put(&W->seq_map, k, (ValueScalar){ .u32 = si1 + 1 });
#ifdef LZ_DEBUG
            array_push(&W->seq_array, k);
#endif
            if (count == ((1 << W->bits_size) - 1)) {
                W->bits_size++;
                // LOG_DEBUG("bump at %d", count);
                if (W->bits_size == W->bits_max) {
                    W->bits_size = W->bits_min;
                    lzwriter_map_reset(W);
                    count = 257;
                }
            }
            // Reset the sequence, start with 'byte` as first item.
            si0 = byte;
        } else {
            // Set the found sequence as parent.
            // See Map#2 rule.
            si0 = si1 - 1;
        }
    }

    lzwriter_write_(W, si0);

    if (W->buffer_count) {
        if (W->bits_count) {
            W->buffer[W->buffer_count++] = W->bits & 0xFF;
        }
        if (W->flushf(W->buffer, W->buffer_count, W->flushf_user) == 0) {
            return 0;
        }
    }

    return 1;
}

//
// LZW Reader.
//

void
lzreader_init(LZReader* R, uint32_t bits_min, uint32_t bits_max, void* data, iptr data_size)
{
    LZ_CHECK_BITS_RANGE_(bits_min, bits_max);
    memset(R, 0, sizeof(LZReader));
    R->data = data;
    R->data_end = R->data + data_size;
    R->bits_min = bits_min;
    R->bits_max = bits_max + 1;
    array_reserve(&R->seq_array, 1ull << bits_max);
    array_reserve(&R->seq_lengths, 1ull << bits_max);
    for (iptr i = 0; i < 257; ++i) {
        array_push(&R->seq_array, (256ull << 32) | i);
        array_push(&R->seq_lengths, 1);
    }
}

void
lzreader_purge(LZReader* R) {
    array_purge(&R->seq_array);
    array_purge(&R->seq_lengths);
    memset(R, 0, sizeof(LZReader));
}

__forceinline uint32_t
lzreader_pull_(LZReader* R)
{
    uint8_t* data = R->data;

    while (R->bits_count < R->bits_size) {
        CHECK(data < R->data_end);
        R->bits |= (uint32_t)data[0] << R->bits_count;
        R->bits_count += 8;
        ++data;
    }

    uint32_t si = R->bits & ((1 << R->bits_size) - 1);
    R->bits >>= R->bits_size;
    R->bits_count -= R->bits_size;

    R->data = data;

    return si;
}

#define lzreader_write_byte_(Out, Byte) \
    *(Out).items = Byte; \
    ++(Out).items; \
    --(Out).count;

__forceinline uint8_t
lzreader_write_sequence_(LZReader* R, ByteSlice* out, uint32_t si)
{
    iptr length = array_at(&R->seq_lengths, si);
    CHECK(length >= 1);
    CHECK(out->count >= length);
    out->count -= length;
    out->items += length;
    length = -1;

    uint64_t k;
    do {
        k = array_at(&R->seq_array, si);
        out->items[length--] = k & 0xFF;
        si = k >> 32;
    } while (si != 256);

    return k & 0xFF;
}

int
lzreader_read(LZReader* R, void* o_buffer, iptr o_buffer_count)
{
    ByteSlice out = {
        .items = o_buffer,
        .count = o_buffer_count
    };

    R->bits = 0;
    R->bits_count = 0;
    R->bits_size = R->bits_min;

    LZSequences* seq_array = &R->seq_array;
    LZLengths* seq_lengths = &R->seq_lengths;

    // NOTE: 256 is reserved.
    array_resize(seq_array, 257);
    array_resize(seq_lengths, 257);

    uint8_t byte;
    uint32_t si1, si0 = lzreader_pull_(R);
    lzreader_write_byte_(out, si0);
    while (R->data != R->data_end)
    {
        si1 = lzreader_pull_(R);

        if (si1 < seq_array->count) {
            byte = lzreader_write_sequence_(R, &out, si1);
        } else {
            CHECK(si1 == seq_array->count);
            CHECK(si0 < seq_array->count);
            byte = lzreader_write_sequence_(R, &out, si0);
            lzreader_write_byte_(out, byte);
        }

        array_push(seq_array, (uint64_t)si0 << 32 | byte);
        array_push(seq_lengths, seq_lengths->items[si0] + 1);

        if (seq_array->count == ((iptr)(1 << R->bits_size) - 2)) {
            R->bits_size++;
            if (R->bits_size == R->bits_max) {
                R->bits_size = R->bits_min;
                array_resize(seq_array, 257);
                array_resize(seq_lengths, 257);
                si0 = lzreader_pull_(R);
                lzreader_write_byte_(out, si0);
                continue;
            }
        }

        si0 = si1;
    }

    return 1;
}