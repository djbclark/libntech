#include <test.h>

#include <mustache.h>
#include <buffer.h>
#include <json.h>
#include <alloc.h>
#include <logging.h>
#include <logging_priv.h>
#include <string_lib.h>

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

/* The last message MustacheRender() logged at LOG_LEVEL_ERR, captured by
 * the logging hook below. */
static char *last_error = NULL;

static char *CaptureErrorHook(ARG_UNUSED LoggingPrivContext *context,
                              LogLevel level, const char *message)
{
    if (level == LOG_LEVEL_ERR)
    {
        free(last_error);
        last_error = xstrdup(message);
    }
    return xstrdup(message);
}

/**
 * Asserts that rendering #templ (which must begin with the broken tag) fails,
 * and that the error logged for it quotes exactly the first 20 characters of
 * the template, as the trailing "..." in the message promises.
 */
static void assert_broken_tag_log_is_truncated(const char *templ)
{
    LoggingPrivContext ctx = { .log_hook = CaptureErrorHook };
    LoggingPrivSetContext(&ctx);
    free(last_error);
    last_error = NULL;

    Buffer *out = BufferNew();
    assert_false(MustacheRender(out, templ, NULL));
    BufferDestroy(out);

    LoggingPrivSetContext(NULL);
    assert_true(last_error != NULL);

    char snippet[21];
    strncpy(snippet, templ, 20);
    snippet[20] = '\0';
    char *expected_tail = StringFormat("at '%s'...", snippet);
    if (!StringEndsWith(last_error, expected_tail))
    {
        printf("expected the error to end with: %s\n", expected_tail);
        printf("                     actual was: %s\n", last_error);
        assert_true(false);
    }
    free(expected_tail);
}

static void test_broken_tag_log_is_truncated(void)
{
    /* A long remainder must be cut to 20 characters, not dumped whole. */
    assert_broken_tag_log_is_truncated("{{{name but this quoted tag never closes, and the rest of the template follows it");
    assert_broken_tag_log_is_truncated("{{name but this tag never closes either, and the rest of the template follows it");

    /* A short remainder must be quoted as it is, not padded out to 20. */
    assert_broken_tag_log_is_truncated("{{{v");
    assert_broken_tag_log_is_truncated("{{v");
}

static void test_non_renderable_tag_at_start_of_template(void)
{
    /* A section, inverted section or comment at offset 0 has no character
     * before it for the standalone-line scan to look at. */
    assert_render("{{#b}}yes{{/b}}", "{\"b\":true}", "yes");
    assert_render("{{^b}}no{{/b}}", "{\"b\":false}", "no");
    assert_render("{{! comment }}x", "{}", "x");
    assert_render("{{#b}}\nyes\n{{/b}}\n", "{\"b\":true}", "yes\n");
}

int main(void)
{
    PRINT_TEST_BANNER();
    const UnitTest tests[] =
    {
        unit_test(test_broken_tag_log_is_truncated),
        unit_test(test_non_renderable_tag_at_start_of_template),
    };

    int ret = run_tests(tests);
    free(last_error);
    return ret;
}
