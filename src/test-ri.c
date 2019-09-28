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
    ));
    ri_log(&ri, node);

    ri_purge(&ri);
}

void
testri_file_(const char* name)
{
    Ri ri;
    ri_init(&ri);

    CharArray path = {0};
    chararray_push_f(&path, __FILE__ "/../test/%s.ri", name);
    array_zero_term(&path);

    ByteArray buffer = {0};
    ASSERT(file_read(&buffer, path.items, 0));

    RiNode* node = ri_parse(&ri, S((char*)buffer.items, buffer.count));

    // ri_log(&ri, node);
    node = ri_resolve(&ri, node);
    ri_log(&ri, node);


    array_clear(&buffer);
    array_clear(&path);
    chararray_push_f(&path, __FILE__ "/../test/%s.recent.lisp", name);
    array_zero_term(&path);

    ri_dump(&ri, node, (CharArray*)&buffer);
    ASSERT(file_write(path.items, buffer.items, buffer.count, 0));

    array_purge(&buffer);
    array_purge(&path);

    ri_purge(&ri);
}

void
testri_resolve() {
    testri_file_("test1");
    testri_file_("for1");
}

void
testri_main() {
    // testri_lex_print();
    // testri_lex();
    // testri_parse();
    testri_resolve();
}