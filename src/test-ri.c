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

void
testri_file_(const char* name)
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

    if (node != NULL) {
        node = ri_resolve(&ri, node);
        if (!ri_typecheck(&ri, node)) {
            node = NULL;
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
    // testri_file_("test1");

    testri_file_("parse-real");

    // testri_file_("decl");
    // testri_file_("if");
    // testri_file_("if-condition-error-bool");
    // testri_file_("for");
    // testri_file_("for-condition-error-is-st");

    // testri_file_("assignment-infer-error");
    // testri_file_("assignment");

    // testri_file_("type-spec");
    // testri_file_("type-inference-const");
    // testri_file_("type-inference-const-binary-left-error");
    // testri_file_("type-inference-const-binary-right-error");

    // testri_file_("op-arithmetic");
    // testri_file_("op-arithmetic-type-mismatch-error");
    // testri_file_("op-bitwise");
    // testri_file_("op-boolean");
    // testri_file_("op-comparison");

    // testri_file_("cast-bool");
    // testri_file_("cast-int-to-bool-error");
    // testri_file_("cast-float-to-bool-error");
    // testri_file_("cast-arguments-count-error");
}

void
testri_main() {
    // testri_lex_print();
    // testri_lex();
    // testri_parse();
    testri_resolve();
}