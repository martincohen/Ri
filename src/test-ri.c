#include <co.h>

#if 0
static inline CoString
testri_token_to_string(RiToken* token) {
    return S(token->start, token->end - token->start);
}

static inline bool
testri_token_equals(RiToken* actual, CoString expected) {
    return string_is_equal(testri_token_to_string(actual), expected);
}

void
testri_lex_print() {
    Ri ri;
    ri_init(&ri, 0);
    ri_stream_set_(&ri, S("func main(a int32)"));

    while (ri.token.kind != RiToken_End) {
        COLOG("- '%S'", testri_token_to_string(&ri.token));
        ASSERT(ri_lex_next_(&ri));
    }
    // ASSERT(ri_lex(&ri)); testri_token_equals(&ri.token, S("func")); ASSERT(ri.token.kind == RiToken_Keyword_Function);
    // ASSERT(ri_lex(&ri)); testri_token_equals(&ri.token, S("main")); ASSERT(ri.token.kind == RiToken_Identifier);
    // ASSERT(ri_lex(&ri)); testri_token_equals(&ri.token, S("func")); ASSERT(ri.token.kind == RiToken_Keyword_Function);

    ri_purge(&ri);
}

static inline void
testri_next_token_equals(Ri* ri, RiTokenKind expected_kind, CoString expected) {
    ASSERT(ri->token.kind == expected_kind);
    ASSERT(testri_token_equals(&ri->token, expected));
    ASSERT(ri_lex_next_(ri));
}

void
testri_lex() {
    Ri ri;
    ri_init(&ri, 0);
    ri_stream_set_(&ri, S("func main(a int32)"));

    testri_next_token_equals(&ri, RiToken_Keyword_Func, S("func"));
    testri_next_token_equals(&ri, RiToken_Identifier, S("main"));
    testri_next_token_equals(&ri, RiToken_LP, S("("));
    testri_next_token_equals(&ri, RiToken_Identifier, S("a"));
    testri_next_token_equals(&ri, RiToken_Identifier, S("int32"));
    testri_next_token_equals(&ri, RiToken_RP, S(")"));
    testri_next_token_equals(&ri, RiToken_End, S(""));

    ri_purge(&ri);
}

void
testri_parse() {
    Ri ri;
    ri_init(&ri, 0);

    RiError error = {0};
    RiNode* node = ri_parse(S(
        "func main(var a int32);"
        "main;"
    ), &ri.arena_, &ri.intern, &ri.global, error);
    ri_log(&ri, node);

    ri_purge(&ri);
}
#endif

#if 0
typedef enum TestRiMode {
    TestRi_Parse,
    TestRi_Resolve
} TestRiMode;

void
testri_file_(const char* name, TestRiMode mode)
{
    COLOG("'%s': testing", name);

    Ri ri;
    ri_init(&ri, 0);
    coarray_push(&ri.paths, CoString("./src/test"));

    RiNode* node = NULL;
    {
        CoCharArray path_source = {0};
        cochararray_push_f(&path_source, "./src/test/%s.ri", name);
        coarray_zero_term(&path_source);
            CoByteArray source = {0};
            ASSERT(cofile_read_c(&source, path_source.items, 0));
            node = ri_parse(&ri, S((char*)source.items, source.count), path_source.slice);
            coarray_purge(&source);
        coarray_purge(&path_source);
    }

    CoByteArray expected = {0};
    {
        CoCharArray path_expected = {0};
        cochararray_push_f(&path_expected, "./src/test/%s.expected.lisp", name);
        coarray_zero_term(&path_expected);
        if (!cofile_read_c(&expected, path_expected.items, 0)) {
            COLOG("'%s': expected file not found", name);
        }
        coarray_purge(&path_expected);
    }

    if (mode == TestRi_Resolve && node != NULL) {
        node = ri_resolve(&ri, node);
        if (node) {
            if (!ri_type_patch(&ri, node)) {
                node = NULL;
            }
        }
    }

    {
        CoCharArray actual = {0};

        if (node != NULL) {
            ri_dump(&ri, node, &actual);
        } else {
            cochararray_push_f(&actual,
                "(error\n"
                "  \"%s.ri\" %d %d\n"
                "  \"%S\"\n"
                ")\n",
                name,
                ri.error.pos.row + 1,
                ri.error.pos.col + 1,
                ri.error.message.slice
            );
        }

        if (expected.items != NULL) {
            if (!string_is_equal(S(expected.items, expected.count), actual.slice)) {
                COLOG("'%s': expected does not match actual", name);
                COLOG("expected:\n%S", expected.slice);
                COLOG("actual:\n%S", actual.slice);
                COLOG("---");
            }
        }

        CoCharArray path_recent = {0};
        cochararray_push_f(&path_recent, "./src/test/%s.recent.lisp", name);
        coarray_zero_term(&path_recent);
        ASSERT(file_write(path_recent.items, actual.items, actual.count, 0));
        coarray_purge(&path_recent);

        coarray_purge(&actual);
    }

    coarray_purge(&expected);
    ri_purge(&ri);
}
#endif

void
testri_load_(CoString id)
{
    Ri ri;
    ri_init(&ri, 0);
    coarray_push(&ri.paths, CoS("./src/test"));

    CoCharArray path = {0};
    CoByteSlice stream = {0};

    RI_ASSERT(ri_load(&ri, id, &path, &stream));

    RiModule* module = ri_add(&ri, path.slice);

    if (ri_parse(&ri, module, stream)) {
        ri_log(module->node);
        RI_ASSERT(ri_resolve(&ri, module));
        ri_log(module->node);
    }

    ri_error_log(&ri);
    coheap_free(stream.items);
    ri_purge(&ri);
}

void
testri_resolve()
{
    // testri_load_(S("import/main"));
    testri_load_(CoS("struct"));
    // testri_load_(S("const"));

    // testri_file_("string-not-terminated.error", TestRi_Parse);
    // testri_file_("string", TestRi_Parse);
    // testri_file_("import/main", TestRi_Resolve);
    // testri_file_("resolve/type-func", TestRi_Resolve);

#if 0
    testri_file_("ast/parse/switch", TestRi_Parse);
    testri_file_("ast/parse/const-real", TestRi_Parse);
    testri_file_("ast/parse/func-no-input-arguments", TestRi_Parse);

    // TODO: testri_file_("ast/test1");

    testri_file_("ast/resolve/decl", TestRi_Resolve);
    testri_file_("ast/resolve/if", TestRi_Resolve);
    testri_file_("ast/resolve/if-condition-error-bool", TestRi_Resolve);
    testri_file_("ast/resolve/for", TestRi_Resolve);
    testri_file_("ast/resolve/for-condition-error-is-st", TestRi_Resolve);

    testri_file_("ast/resolve/assignment-infer-error", TestRi_Resolve);
    testri_file_("ast/resolve/assignment", TestRi_Resolve);


    testri_file_("ast/resolve/type-spec", TestRi_Resolve);
    testri_file_("ast/resolve/type-inference-const", TestRi_Resolve);
    testri_file_("ast/resolve/type-inference-const-binary-left-error", TestRi_Resolve);
    testri_file_("ast/resolve/type-inference-const-binary-right-error", TestRi_Resolve);

    testri_file_("ast/resolve/op-arithmetic", TestRi_Resolve);
    testri_file_("ast/resolve/op-arithmetic-type-mismatch-error", TestRi_Resolve);
    testri_file_("ast/resolve/op-binary", TestRi_Resolve);
    testri_file_("ast/resolve/op-bitwise", TestRi_Resolve);
    testri_file_("ast/resolve/op-boolean", TestRi_Resolve);
    testri_file_("ast/resolve/op-comparison", TestRi_Resolve);

    testri_file_("ast/resolve/cast-bool", TestRi_Resolve);
    testri_file_("ast/resolve/cast-int-to-bool-error", TestRi_Resolve);
    testri_file_("ast/resolve/cast-float-to-bool-error", TestRi_Resolve);
    testri_file_("ast/resolve/cast-arguments-count-error", TestRi_Resolve);
    // TODO: testri_file_("ast/resolve/cast-pointer-mul-conflict", TestRi_Resolve);

    // testri_file_("ast/resolve/func", TestRi_Resolve);
    // testri_file_("ast/resolve/call", TestRi_Resolve);
#endif
}

void
testri_main() {
    // testri_lex_print();
    // testri_lex();
    // testri_parse();
    testri_resolve();
}