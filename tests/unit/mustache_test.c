#include <test.h>

#include <mustache.h>
#include <buffer.h>
#include <json.h>

/**
 * Renders #templ with #json_hash (a JSON document, or NULL for no hash) and
 * asserts that rendering succeeded and produced #expected.
 */
static void assert_render(const char *templ, const char *json_hash, const char *expected)
{
    JsonElement *hash = NULL;
    if (json_hash != NULL)
    {
        const char *data = json_hash;
        assert_int_equal(JsonParse(&data, &hash), JSON_PARSE_OK);
    }

    Buffer *out = BufferNew();
    assert_true(MustacheRender(out, templ, hash));
    assert_string_equal(BufferData(out), expected);

    BufferDestroy(out);
    JsonDestroy(hash);
}

/**
 * Asserts that rendering #templ with #json_hash fails.
 */
static void assert_render_fails(const char *templ, const char *json_hash)
{
    JsonElement *hash = NULL;
    if (json_hash != NULL)
    {
        const char *data = json_hash;
        assert_int_equal(JsonParse(&data, &hash), JSON_PARSE_OK);
    }

    Buffer *out = BufferNew();
    assert_false(MustacheRender(out, templ, hash));

    BufferDestroy(out);
    JsonDestroy(hash);
}

static void test_variables(void)
{
    assert_render("Hello {{name}}!", "{\"name\":\"world\"}", "Hello world!");
    assert_render("{{ name }}", "{\"name\":\"world\"}", "world");
    assert_render("{{a}} {{b}}", "{\"a\":\"x\",\"b\":\"y\"}", "x y");
    assert_render("{{n}}", "{\"n\":42}", "42");
    assert_render("{{n}}", "{\"n\":-7}", "-7");
    assert_render("{{t}}/{{f}}", "{\"t\":true,\"f\":false}", "true/false");
}

static void test_missing_variables(void)
{
    assert_render("[{{nope}}]", "{\"a\":1}", "[]");
    assert_render("[{{a}}]", "{\"a\":null}", "[]");
    assert_render("[{{a}}]", "[1,2,3]", "[]");
    assert_render("[{{a}}]", NULL, "[]");
}

static void test_html_escaping(void)
{
    assert_render("{{a}}", "{\"a\":\"<b>&\\\"\"}", "&lt;b&gt;&amp;&quot;");

    /* Single quotes are deliberately not escaped. */
    assert_render("{{a}}", "{\"a\":\"it's\"}", "it's");

    /* Both unescaped forms leave the value alone. */
    assert_render("{{{a}}}", "{\"a\":\"<b>&\\\"\"}", "<b>&\"");
    assert_render("{{&a}}", "{\"a\":\"<b>&\\\"\"}", "<b>&\"");
}

static void test_dotted_names(void)
{
    assert_render("{{a.b.c}}", "{\"a\":{\"b\":{\"c\":\"deep\"}}}", "deep");
    assert_render("[{{a.b.z}}]", "{\"a\":{\"b\":{\"c\":\"deep\"}}}", "[]");

    /* Descending into a primitive finds nothing. */
    assert_render("[{{a.b}}]", "{\"a\":\"str\"}", "[]");
}

static void test_comments(void)
{
    assert_render("a{{! comment }}b", "{}", "ab");
    assert_render("a{{! line one\nline two }}b", "{}", "ab");

    /* A comment alone on a line takes the whole line with it. */
    assert_render("a\n{{! comment }}\nb\n", "{}", "a\nb\n");
    assert_render("a\r\n{{! comment }}\r\nb", "{}", "a\r\nb");
}

static void test_sections_boolean(void)
{
    assert_render("{{#b}}yes{{/b}}", "{\"b\":true}", "yes");
    assert_render("[{{#b}}yes{{/b}}]", "{\"b\":false}", "[]");
    assert_render("[{{#b}}yes{{/b}}]", "{}", "[]");
    assert_render("{{#a}}A{{/a}}{{#b}}B{{/b}}", "{\"a\":true,\"b\":true}", "AB");

    /* A section over a non-boolean primitive is an error. */
    assert_render_fails("{{#s}}x{{/s}}", "{\"s\":\"str\"}");
}

static void test_sections_array(void)
{
    assert_render("{{#l}}({{.}}){{/l}}", "{\"l\":[\"a\",\"b\",\"c\"]}", "(a)(b)(c)");
    assert_render("{{#l}}{{n}}={{v}};{{/l}}",
                  "{\"l\":[{\"n\":\"a\",\"v\":1},{\"n\":\"b\",\"v\":2}]}",
                  "a=1;b=2;");
    assert_render("[{{#l}}x{{/l}}]", "{\"l\":[]}", "[]");

    /* Names not found in the element fall back to the enclosing hash. */
    assert_render("{{#l}}{{v}}{{/l}}", "{\"v\":\"outer\",\"l\":[{\"v\":\"inner\"},{}]}",
                  "innerouter");

    assert_render("{{#l}}{{#i}}{{.}},{{/i}}{{/l}}",
                  "{\"l\":[{\"i\":[1,2]},{\"i\":[3]}]}", "1,2,3,");
}

static void test_sections_object(void)
{
    /* Without {{@}} an object section is entered once, not iterated. */
    assert_render("{{#o}}{{x}}{{/o}}", "{\"o\":{\"x\":\"X\"}}", "X");
    assert_render("{{#o}}{{p.q}}{{/o}}", "{\"o\":{\"p\":{\"q\":\"PQ\"}}}", "PQ");
    assert_render("{{#a}}{{#b}}{{v}}{{/b}}{{/a}}", "{\"a\":{\"b\":{\"v\":\"n\"}}}", "n");
}

static void test_inverted_sections(void)
{
    assert_render("{{^b}}no{{/b}}", "{\"b\":false}", "no");
    assert_render("[{{^b}}no{{/b}}]", "{\"b\":true}", "[]");
    assert_render("{{^b}}no{{/b}}", "{}", "no");
    assert_render("[{{^l}}empty{{/l}}]", "{\"l\":[]}", "[empty]");
    assert_render("[{{^l}}empty{{/l}}]", "{\"l\":[1]}", "[]");
    assert_render("[{{^o}}empty{{/o}}]", "{\"o\":{\"k\":1}}", "[]");
}

/* {{.}} expands the element currently being iterated over. */
static void test_item_extension(void)
{
    assert_render("{{#l}}{{.}}{{/l}}", "{\"l\":[1,2,3]}", "123");
    assert_render("{{#l}}{{.}}{{/l}}", "{\"l\":[\"<a>\"]}", "&lt;a&gt;");
    assert_render("{{#l}}{{&.}}{{/l}}", "{\"l\":[\"<a>\"]}", "<a>");

    /* A container needs an explicit conversion. */
    assert_render_fails("{{#l}}{{.}}{{/l}}", "{\"l\":[[1,2]]}");
}

/* {{@}} expands the key of the current object member, or the array index. */
static void test_key_extension(void)
{
    assert_render("{{#o}}{{@}}={{.}};{{/o}}", "{\"o\":{\"a\":1,\"b\":2}}", "a=1;b=2;");
    assert_render("{{#o}}{{&@}}={{.}};{{/o}}", "{\"o\":{\"a\":1}}", "a=1;");
    assert_render("{{#o}}{{{@}}}={{.}};{{/o}}", "{\"o\":{\"a\":1}}", "a=1;");
    assert_render("{{#o}}{{@}};{{/o}}", "{\"o\":{\"<a>\":1}}", "&lt;a&gt;;");
    assert_render("{{#l}}{{@}}:{{.}} {{/l}}", "{\"l\":[\"a\",\"b\"]}", "0:a 1:b ");

    /* Outside of an iteration there is no key to expand. */
    assert_render_fails("{{@}}", "{\"a\":1}");
}

/* {{%x}} and {{$x}} serialize a container as pretty and compact JSON. */
static void test_serialization_extension(void)
{
    assert_render("{{%o}}", "{\"o\":{\"a\":1}}", "{\n  \"a\": 1\n}");
    assert_render("{{$o}}", "{\"o\":{\"a\":1,\"b\":[1,2]}}", "{\"a\":1,\"b\":[1,2]}");
    assert_render("{{$l}}", "{\"l\":[1,2,3]}", "[1,2,3]");
    assert_render("{{#l}}{{$.}}{{/l}}", "{\"l\":[[1,2],[3]]}", "[1,2][3]");
}

/* {{-top-}} refers to the hash the template was rendered with. */
static void test_top_extension(void)
{
    assert_render("{{#o}}{{-top-.t}}{{/o}}", "{\"t\":\"TOP\",\"o\":{\"t\":\"inner\"}}", "TOP");
    assert_render("{{$-top-}}", "{\"a\":1}", "{\"a\":1}");
}

static void test_set_delimiters(void)
{
    assert_render("{{=<% %>=}}<%v%>", "{\"v\":\"D\"}", "D");
    assert_render("{{=| |=}}|#b|yes|/b|", "{\"b\":true}", "yes");

    /* A delimiter tag alone on a line takes the whole line with it. */
    assert_render("a\n{{=| |=}}\n|v|", "{\"v\":\"D\"}", "a\nD");

    assert_render_fails("{{=<%=}}x", "{\"v\":1}");
}

static void test_standalone_tags(void)
{
    assert_render("a\n{{#b}}\nx\n{{/b}}\nc\n", "{\"b\":true}", "a\nx\nc\n");
    assert_render("a\n{{#b}}\nx\n{{/b}}\nc\n", "{\"b\":false}", "a\nc\n");

    /* A variable is not standalone, so its line is kept. */
    assert_render("a\n {{v}} \nc\n", "{\"v\":\"V\"}", "a\n V \nc\n");
}

static void test_malformed_templates(void)
{
    assert_render_fails("a{{v", "{\"v\":1}");
    assert_render_fails("a{{{v}}", "{\"v\":1}");
    assert_render_fails("a{{/x}}b", "{}");
    assert_render_fails("a{{#v}}b", "{\"v\":true}");
}

static void test_no_expansion(void)
{
    assert_render("", "{}", "");
    assert_render("plain text", "{}", "plain text");

    /* An empty tag is passed through verbatim. */
    assert_render("{{}}", "{\"v\":1}", "{{}}");
}

int main(void)
{
    PRINT_TEST_BANNER();
    const UnitTest tests[] =
    {
        unit_test(test_variables),
        unit_test(test_missing_variables),
        unit_test(test_html_escaping),
        unit_test(test_dotted_names),
        unit_test(test_comments),
        unit_test(test_sections_boolean),
        unit_test(test_sections_array),
        unit_test(test_sections_object),
        unit_test(test_inverted_sections),
        unit_test(test_item_extension),
        unit_test(test_key_extension),
        unit_test(test_serialization_extension),
        unit_test(test_top_extension),
        unit_test(test_set_delimiters),
        unit_test(test_standalone_tags),
        unit_test(test_malformed_templates),
        unit_test(test_no_expansion),
    };

    return run_tests(tests);
}
