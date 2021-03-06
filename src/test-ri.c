static inline String
testri_token_to_string(RiToken* token) {
    return S(token->start, token->end - token->start);
}

static inline bool
testri_token_equals(RiToken* actual, String expected) {
    return string_is_equal(testri_token_to_string(actual), expected);
}

void
testri_lex_print() {
    Ri ri;
    ri_init(&ri);
    ri_stream_set_(&ri, S("func main(a int32)"));

    while (ri.token.kind != RiToken_End) {
        LOG("- '%S'", testri_token_to_string(&ri.token));
        ASSERT(ri_lex_next_(&ri));
    }
    // ASSERT(ri_lex(&ri)); testri_token_equals(&ri.token, S("func")); ASSERT(ri.token.kind == RiToken_Keyword_Function);
    // ASSERT(ri_lex(&ri)); testri_token_equals(&ri.token, S("main")); ASSERT(ri.token.kind == RiToken_Identifier);
    // ASSERT(ri_lex(&ri)); testri_token_equals(&ri.token, S("func")); ASSERT(ri.token.kind == RiToken_Keyword_Function);

    ri_purge(&ri);
}

static inline void
testri_next_token_equals(Ri* ri, RiTokenKind expected_kind, String expected) {
    ASSERT(ri->token.kind == expected_kind);
    ASSERT(testri_token_equals(&ri->token, expected));
    ASSERT(ri_lex_next_(ri));
}

void
testri_lex() {
    Ri ri;
    ri_init(&ri);
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
    ri_init(&ri);

    RiNode* node = ri_parse(&ri, S(
        "func main(var a int32);"
        "main;"
    ), S("testri_parse.ri"));
    ri_log(&ri, node);

    ri_purge(&ri);
}

typedef enum TestRiMode {
    TestRi_Parse,
    TestRi_Resolve
} TestRiMode;

void
testri_file_(const char* name, TestRiMode mode)
{
    LOG("'%s': testing", name);

    Ri ri;
    ri_init(&ri);

    RiNode* node = NULL;
    {
        CharArray path_source = {0};
        chararray_push_f(&path_source, "./src/test/ast/%s.ri", name);
        array_zero_term(&path_source);
            ByteArray source = {0};
            ASSERT(file_read(&source, path_source.items, 0));
            node = ri_parse(&ri, S((char*)source.items, source.count), path_source.slice);
            array_purge(&source);
        array_purge(&path_source);
    }

    ByteArray expected = {0};
    {
        CharArray path_expected = {0};
        chararray_push_f(&path_expected, "./src/test/ast/%s.expected.lisp", name);
        array_zero_term(&path_expected);
        if (!file_read(&expected, path_expected.items, 0)) {
            LOG("'%s': expected file not found", name);
        }
        array_purge(&path_expected);
    }

    if (mode == TestRi_Resolve && node != NULL) {
        node = ri_resolve(&ri, node);
        if (node) {
            if (!ri_typecheck(&ri, node)) {
                node = NULL;
            }
        }
    }

    {
        CharArray actual = {0};

        if (node != NULL) {
            ri_dump(&ri, node, &actual);
        } else {
            chararray_push_f(&actual,
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
                LOG("'%s': expected does not match actual", name);
                LOG("expected:\n%S", expected.slice);
                LOG("actual:\n%S", actual.slice);
                LOG("---");
            }
        }

        CharArray path_recent = {0};
        chararray_push_f(&path_recent, "./src/test/ast/%s.recent.lisp", name);
        array_zero_term(&path_recent);
        ASSERT(file_write(path_recent.items, actual.items, actual.count, 0));
        array_purge(&path_recent);

        array_purge(&actual);
    }

    array_purge(&expected);
    ri_purge(&ri);
}

void
testri_resolve() {
    testri_file_("parse/func-no-input-arguments", TestRi_Parse);

#if 0
    testri_file_("parse/switch", TestRi_Parse);
    testri_file_("parse/const-real", TestRi_Parse);

    // TODO: testri_file_("test1");

    testri_file_("resolve/decl", TestRi_Resolve);
    testri_file_("resolve/if", TestRi_Resolve);
    testri_file_("resolve/if-condition-error-bool", TestRi_Resolve);
    testri_file_("resolve/for", TestRi_Resolve);
    testri_file_("resolve/for-condition-error-is-st", TestRi_Resolve);

    testri_file_("resolve/assignment-infer-error", TestRi_Resolve);
    testri_file_("resolve/assignment", TestRi_Resolve);

    testri_file_("resolve/type-spec", TestRi_Resolve);
    testri_file_("resolve/type-inference-const", TestRi_Resolve);
    testri_file_("resolve/type-inference-const-binary-left-error", TestRi_Resolve);
    testri_file_("resolve/type-inference-const-binary-right-error", TestRi_Resolve);

    testri_file_("resolve/op-arithmetic", TestRi_Resolve);
    testri_file_("resolve/op-arithmetic-type-mismatch-error", TestRi_Resolve);
    testri_file_("resolve/op-bitwise", TestRi_Resolve);
    testri_file_("resolve/op-boolean", TestRi_Resolve);
    testri_file_("resolve/op-comparison", TestRi_Resolve);

    testri_file_("resolve/cast-bool", TestRi_Resolve);
    testri_file_("resolve/cast-int-to-bool-error", TestRi_Resolve);
    testri_file_("resolve/cast-float-to-bool-error", TestRi_Resolve);
    testri_file_("resolve/cast-arguments-count-error", TestRi_Resolve);
#endif
}

void
testri_main() {
    // testri_lex_print();
    // testri_lex();
    // testri_parse();
    testri_resolve();
}