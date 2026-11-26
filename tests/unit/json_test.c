#include <test.h>
#include <logging.h>

#include <json.h>
#include <string_lib.h>
#include <file_lib.h>
#include <misc_lib.h> /* xsnprintf */
#include <alloc.h>    // xasprintf()

#include <float.h>


static const char *OBJECT_ARRAY =
    "{\n"
    "  \"first\": [\n"
    "    \"one\",\n"
    "    \"two\"\n"
    "  ]\n"
    "}";

static const char *OBJECT_COMPOUND =
    "{\n"
    "  \"first\": \"one\",\n"
    "  \"fourth\": {\n"
    "    \"fifth\": \"five\"\n"
    "  },\n"
    "  \"second\": {\n"
    "    \"third\": \"three\"\n"
    "  }\n"
    "}";

static const char *OBJECT_SIMPLE =
    "{\n"
    "  \"first\": \"one\",\n"
    "  \"second\": \"two\"\n"
    "}";

static const char *OBJECT_NUMERIC =
    "{\n"
    "  \"int\": -1234567890,\n"
    "  \"real\": 1234.5678\n"
    "}";

static const char *OBJECT_BOOLEAN =
    "{\n"
    "  \"bool_value\": true\n"
    "}";

static const char *OBJECT_ESCAPED =
    "{\n"
    "  \"escaped\": \"quote\\\"stuff \\t \\n\\n\"\n"
    "}";

static const char *ARRAY_SIMPLE =
    "[\n"
    "  \"one\",\n"
    "  \"two\"\n"
    "]";

static const char *ARRAY_NUMERIC =
    "[\n"
    "  123,\n"
    "  123.1234\n"
    "]";

static const char *ARRAY_OBJECT =
    "[\n"
    "  {\n"
    "    \"first\": \"one\"\n"
    "  }\n"
    "]";

static JsonElement *LoadTestFile(const char *filename)
{
    char path[PATH_MAX];
    xsnprintf(path, sizeof(path), "%s/%s", TESTDATADIR, filename);

    Writer *w = FileRead(path, SIZE_MAX, NULL);
    if (!w)
    {
        return NULL;
    }
    JsonElement *json = NULL;
    const char *data = StringWriterData(w);
    if (JsonParse(&data, &json) != JSON_PARSE_OK)
    {
        WriterClose(w);
        return NULL;
    }

    WriterClose(w);
    return json;
}

static void test_new_delete(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonObjectAppendString(json, "first", "one");
    JsonDestroy(json);
}

static void test_object_duplicate_key(void)
{
    JsonElement *a = JsonObjectCreate(1);

    JsonObjectAppendString(a, "a", "a");
    JsonObjectAppendString(a, "a", "a");

    assert_int_equal(1, JsonLength(a));

    JsonDestroy(a);
}

static void test_show_string(void)
{
    JsonElement *str = JsonStringCreate("snookie");

    Writer *writer = StringWriter();

    JsonWrite(writer, str, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal("\"snookie\"", output);

    JsonDestroy(str);
    free(output);
}

static void test_show_object_simple(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonObjectAppendString(json, "first", "one");
    JsonObjectAppendString(json, "second", "two");

    Writer *writer = StringWriter();

    JsonWrite(writer, json, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(OBJECT_SIMPLE, output);

    JsonDestroy(json);
    free(output);
}

static void test_show_object_escaped(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonObjectAppendString(json, "escaped", "quote\"stuff \t \n\n");

    Writer *writer = StringWriter();

    JsonWrite(writer, json, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(OBJECT_ESCAPED, output);

    JsonDestroy(json);
    free(output);
}

static void test_show_object_numeric(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonObjectAppendReal(json, "real", 1234.5678);
    JsonObjectAppendInteger(json, "int", -1234567890);

    Writer *writer = StringWriter();

    JsonWrite(writer, json, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(OBJECT_NUMERIC, output);

    JsonDestroy(json);
    free(output);
}

static void test_show_object_boolean(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonObjectAppendBool(json, "bool_value", true);

    Writer *writer = StringWriter();

    JsonWrite(writer, json, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(OBJECT_BOOLEAN, output);

    JsonDestroy(json);
    free(output);
}

static void test_show_object_compound(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonObjectAppendString(json, "first", "one");
    {
        JsonElement *inner = JsonObjectCreate(10);

        JsonObjectAppendString(inner, "third", "three");

        JsonObjectAppendObject(json, "second", inner);
    }
    {
        JsonElement *inner = JsonObjectCreate(10);

        JsonObjectAppendString(inner, "fifth", "five");

        JsonObjectAppendObject(json, "fourth", inner);
    }

    Writer *writer = StringWriter();

    JsonWrite(writer, json, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(OBJECT_COMPOUND, output);

    JsonDestroy(json);
    free(output);
}

static void test_show_object_compound_compact(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonObjectAppendString(json, "first", "one");
    {
        JsonElement *inner = JsonObjectCreate(10);

        JsonObjectAppendString(inner, "third", "three");

        JsonObjectAppendObject(json, "second", inner);
    }
    {
        JsonElement *inner = JsonObjectCreate(10);

        JsonObjectAppendString(inner, "fifth", "five");

        JsonObjectAppendObject(json, "fourth", inner);
    }

    Writer *writer = StringWriter();

    JsonWriteCompact(writer, json);
    char *output = StringWriterClose(writer);

    assert_string_equal(
        "{\"first\":\"one\",\"fourth\":{\"fifth\":\"five\"},\"second\":{\"third\":\"three\"}}",
        output);

    JsonDestroy(json);
    free(output);
}

static void test_show_object_array(void)
{
    JsonElement *json = JsonObjectCreate(10);

    JsonElement *array = JsonArrayCreate(10);

    JsonArrayAppendString(array, "one");
    JsonArrayAppendString(array, "two");

    JsonObjectAppendArray(json, "first", array);

    Writer *writer = StringWriter();

    JsonWrite(writer, json, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(OBJECT_ARRAY, output);

    JsonDestroy(json);
    free(output);
}

static void test_show_array(void)
{
    JsonElement *array = JsonArrayCreate(10);

    JsonArrayAppendString(array, "one");
    JsonArrayAppendString(array, "two");

    Writer *writer = StringWriter();

    JsonWrite(writer, array, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(ARRAY_SIMPLE, output);

    JsonDestroy(array);
    free(output);
}

static void test_show_array_compact(void)
{
    JsonElement *array = JsonArrayCreate(10);

    JsonArrayAppendString(array, "one");
    JsonArrayAppendString(array, "two");

    Writer *writer = StringWriter();

    JsonWriteCompact(writer, array);
    char *output = StringWriterClose(writer);

    assert_string_equal("[\"one\",\"two\"]", output);

    JsonDestroy(array);
    free(output);
}

static void test_show_array_boolean(void)
{
    JsonElement *array = JsonArrayCreate(10);

    JsonArrayAppendBool(array, true);
    JsonArrayAppendBool(array, false);

    Writer *writer = StringWriter();

    JsonWrite(writer, array, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(
        "[\n"
        "  true,\n"
        "  false\n"
        "]",
        output);

    JsonDestroy(array);
    free(output);
}

static void test_show_array_numeric(void)
{
    JsonElement *array = JsonArrayCreate(10);

    JsonArrayAppendInteger(array, 123);
    JsonArrayAppendReal(array, 123.1234);

    Writer *writer = StringWriter();

    JsonWrite(writer, array, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(ARRAY_NUMERIC, output);

    JsonDestroy(array);
    free(output);
}

static void test_show_array_object(void)
{
    JsonElement *array = JsonArrayCreate(10);
    JsonElement *object = JsonObjectCreate(10);

    JsonObjectAppendString(object, "first", "one");

    JsonArrayAppendObject(array, object);

    Writer *writer = StringWriter();

    JsonWrite(writer, array, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal(ARRAY_OBJECT, output);

    JsonDestroy(array);
    free(output);
}

static void test_show_array_empty(void)
{
    JsonElement *array = JsonArrayCreate(10);

    Writer *writer = StringWriter();

    JsonWrite(writer, array, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal("[]", output);

    JsonDestroy(array);
    free(output);
}

static void test_show_array_nan(void)
{
    JsonElement *array = JsonArrayCreate(10);
    JsonArrayAppendReal(array, sqrt(-1));

    Writer *writer = StringWriter();

    JsonWrite(writer, array, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal("[\n  0.0000\n]", output);

    JsonDestroy(array);
    free(output);
}

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

static void test_show_array_infinity(void)
{
    JsonElement *array = JsonArrayCreate(10);
    JsonArrayAppendReal(array, INFINITY);

    Writer *writer = StringWriter();

    JsonWrite(writer, array, 0);
    char *output = StringWriterClose(writer);

    assert_string_equal("[\n  0.0000\n]", output);

    JsonDestroy(array);
    free(output);
}

static void test_object_get_string(void)
{
    JsonElement *obj = JsonObjectCreate(10);

    JsonObjectAppendString(obj, "first", "one");
    JsonObjectAppendString(obj, "second", "two");

    assert_string_equal(JsonObjectGetAsString(obj, "second"), "two");
    assert_string_equal(JsonObjectGetAsString(obj, "first"), "one");

    JsonDestroy(obj);
}

static void test_object_get_bool(void)
{
    JsonElement *obj = JsonObjectCreate(10);

    JsonObjectAppendBool(obj, "true", true);
    JsonObjectAppendBool(obj, "false", false);

    assert_int_equal(JsonObjectGetAsBool(obj, "true"), true);
    assert_int_equal(JsonObjectGetAsBool(obj, "false"), false);

    JsonDestroy(obj);
}

static void test_object_get_array(void)
{
    JsonElement *arr = JsonArrayCreate(10);

    JsonArrayAppendString(arr, "one");
    JsonArrayAppendString(arr, "two");

    JsonElement *obj = JsonObjectCreate(10);

    JsonObjectAppendArray(obj, "array", arr);

    JsonElement *arr2 = JsonObjectGetAsArray(obj, "array");

    assert_string_equal(JsonArrayGetAsString(arr2, 1), "two");

    JsonDestroy(obj);
}

static void test_object_iterator(void)
{
    JsonElement *obj = JsonObjectCreate(10);

    JsonObjectAppendString(obj, "first", "one");
    JsonObjectAppendString(obj, "second", "two");
    JsonObjectAppendInteger(obj, "third", 3);
    JsonObjectAppendBool(obj, "fourth", true);
    JsonObjectAppendBool(obj, "fifth", false);


    {
        JsonIterator it = JsonIteratorInit(obj);

        assert_true(JsonIteratorHasMore(&it));
        assert_string_equal("first", JsonIteratorNextKey(&it));
        assert_string_equal("second", JsonIteratorNextKey(&it));
        assert_string_equal("third", JsonIteratorNextKey(&it));
        assert_string_equal("fourth", JsonIteratorNextKey(&it));
        assert_true(JsonIteratorHasMore(&it));
        assert_string_equal("fifth", JsonIteratorNextKey(&it));
        assert_false(JsonIteratorHasMore(&it));
        assert_false(JsonIteratorNextKey(&it));
    }

    {
        JsonIterator it = JsonIteratorInit(obj);

        assert_true(JsonIteratorHasMore(&it));
        assert_string_equal(
            "one", JsonPrimitiveGetAsString(JsonIteratorNextValue(&it)));
        assert_string_equal(
            "two", JsonPrimitiveGetAsString(JsonIteratorNextValue(&it)));
        assert_int_equal(
            3, JsonPrimitiveGetAsInteger(JsonIteratorNextValue(&it)));
        assert_true(JsonPrimitiveGetAsBool(JsonIteratorNextValue(&it)));
        assert_true(JsonIteratorHasMore(&it));
        assert_false(JsonPrimitiveGetAsBool(JsonIteratorNextValue(&it)));
        assert_false(JsonIteratorHasMore(&it));
        assert_false(JsonIteratorNextValue(&it));
    }

    JsonDestroy(obj);
}

static void test_array_get_string(void)
{
    JsonElement *arr = JsonArrayCreate(10);

    JsonArrayAppendString(arr, "first");
    JsonArrayAppendString(arr, "second");

    assert_string_equal(JsonArrayGetAsString(arr, 1), "second");
    assert_string_equal(JsonArrayGetAsString(arr, 0), "first");

    JsonDestroy(arr);
}

static void test_array_iterator(void)
{
    JsonElement *arr = JsonArrayCreate(10);

    JsonArrayAppendString(arr, "first");
    JsonArrayAppendString(arr, "second");

    {
        JsonIterator it = JsonIteratorInit(arr);

        assert_true(JsonIteratorHasMore(&it));
        assert_string_equal(
            "first", JsonPrimitiveGetAsString(JsonIteratorNextValue(&it)));
        assert_true(JsonIteratorHasMore(&it));
        assert_string_equal(
            "second", JsonPrimitiveGetAsString(JsonIteratorNextValue(&it)));
        assert_false(JsonIteratorHasMore(&it));
        assert_false(JsonIteratorNextValue(&it));
    }

    JsonDestroy(arr);
}

static void test_copy_compare(void)
{
    JsonElement *bench = LoadTestFile("benchmark.json");
    assert_true(bench != NULL);

    JsonElement *copy = JsonCopy(bench);
    assert_true(copy != NULL);

    assert_int_equal(0, JsonCompare(copy, bench));

    JsonDestroy(bench);
    JsonDestroy(copy);
}

static void test_compare_container_type_mismatch(void)
{
    JsonElement *object_a = JsonObjectCreate(1);
    JsonElement *object_b = JsonObjectCreate(1);

    JsonElement *child = JsonObjectCreate(1);
    JsonObjectAppendObject(object_a, "key", child);

    JsonElement *array = JsonArrayCreate(1);
    JsonArrayAppendString(array, "first");
    JsonArrayAppendString(array, "second");
    JsonObjectAppendArray(object_b, "key", array);

    assert_true(JsonCompare(object_a, object_b) != 0);

    JsonDestroy(object_a);
    JsonDestroy(object_b);
}

static void test_select(void)
{
    const char *data = OBJECT_ARRAY;
    JsonElement *obj = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &obj));

    assert_true(obj == JsonSelect(obj, 0, NULL));

    {
        char *indices[] = {"first"};
        assert_int_equal(
            JSON_CONTAINER_TYPE_ARRAY,
            JsonGetContainerType(JsonSelect(obj, 1, indices)));
    }
    {
        char *indices[] = {"first", "0"};
        assert_string_equal(
            "one", JsonPrimitiveGetAsString(JsonSelect(obj, 2, indices)));
    }
    {
        char *indices[] = {"first", "1"};
        assert_string_equal(
            "two", JsonPrimitiveGetAsString(JsonSelect(obj, 2, indices)));
    }
    {
        char *indices[] = {"first", "2"};
        assert_true(JsonSelect(obj, 2, indices) == NULL);
    }
    {
        char *indices[] = {"first", "x"};
        assert_true(JsonSelect(obj, 2, indices) == NULL);
    }

    {
        char *indices[] = {"first", "0", "x"};
        assert_true(JsonSelect(obj, 3, indices) == NULL);
    }

    {
        char *indices[] = {"second"};
        assert_true(JsonSelect(obj, 1, indices) == NULL);
    }

    JsonDestroy(obj);
}

static void test_merge_array(void)
{
    JsonElement *a = JsonArrayCreate(2);
    JsonArrayAppendString(a, "a");
    JsonArrayAppendString(a, "b");

    JsonElement *b = JsonArrayCreate(2);
    JsonArrayAppendString(b, "c");
    JsonArrayAppendString(b, "d");

    JsonElement *c = JsonMerge(a, b);

    assert_int_equal(2, JsonLength(a));
    assert_int_equal(2, JsonLength(b));
    assert_int_equal(4, JsonLength(c));

    assert_string_equal("a", JsonArrayGetAsString(c, 0));
    assert_string_equal("b", JsonArrayGetAsString(c, 1));
    assert_string_equal("c", JsonArrayGetAsString(c, 2));
    assert_string_equal("d", JsonArrayGetAsString(c, 3));

    JsonDestroy(a);
    JsonDestroy(b);
    JsonDestroy(c);
}

static void test_merge_object(void)
{
    JsonElement *a = JsonObjectCreate(2);
    JsonObjectAppendString(a, "a", "a");
    JsonObjectAppendString(a, "b", "b");

    JsonElement *b = JsonObjectCreate(2);
    JsonObjectAppendString(b, "b", "b");
    JsonObjectAppendString(b, "c", "c");

    JsonElement *c = JsonMerge(a, b);

    assert_int_equal(2, JsonLength(a));
    assert_int_equal(2, JsonLength(b));
    assert_int_equal(3, JsonLength(c));

    assert_string_equal("a", JsonObjectGetAsString(c, "a"));
    assert_string_equal("b", JsonObjectGetAsString(c, "b"));
    assert_string_equal("c", JsonObjectGetAsString(c, "c"));

    JsonDestroy(a);
    JsonDestroy(b);
    JsonDestroy(c);
}

static void test_parse_empty_containers(void)
{
    {
        const char *data = "{}";
        JsonElement *obj = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &obj));
        assert_true(obj != NULL);
        assert_int_equal(JSON_TYPE_OBJECT, JsonGetType(obj));
        assert_int_equal(0, JsonLength(obj));
        JsonDestroy(obj);
    }

    {
        const char *data = "[]";
        JsonElement *arr = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &arr));
        assert_true(arr != NULL);
        assert_int_equal(JSON_ELEMENT_TYPE_CONTAINER, JsonGetElementType(arr));
        assert_int_equal(JSON_CONTAINER_TYPE_ARRAY, JsonGetContainerType(arr));
        assert_int_equal(JSON_TYPE_ARRAY, JsonGetType(arr));
        assert_int_equal(JsonGetContainerType(arr), JsonGetType(arr));
        assert_int_equal(0, JsonLength(arr));
        JsonDestroy(arr);
    }
}

static void test_parse_object_simple(void)
{
    const char *data = OBJECT_SIMPLE;
    JsonElement *obj = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &obj));

    assert_string_equal(JsonObjectGetAsString(obj, "second"), "two");
    assert_string_equal(JsonObjectGetAsString(obj, "first"), "one");
    assert_int_equal(JsonObjectGetAsString(obj, "third"), NULL);

    JsonDestroy(obj);
}

static void test_parse_object_escaped(void)
{
    const char *decoded = "\"/var/cfenigne/bin/cf-know\" ";

    const char *json_string =
        "{\n  \"key\": \"\\\"/var/cfenigne/bin/cf-know\\\" \"\n}";

    JsonElement *obj = NULL;
    const char *data = json_string;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &obj));

    assert_int_not_equal(obj, NULL);
    assert_string_equal(JsonObjectGetAsString(obj, "key"), decoded);

    {
        Writer *w = StringWriter();
        JsonWrite(w, obj, 0);

        assert_string_equal(json_string, StringWriterData(w));

        WriterClose(w);
    }

    JsonDestroy(obj);
}

static void test_parse_tzz_evil_key(void)
{
    const char *data =
        "{ \"third key! can? be$ anything&\": [ \"a\", \"b\", \"c\" ]}";
    JsonElement *obj = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &obj));

    assert_string_equal(
        "b",
        JsonArrayGetAsString(
            JsonObjectGetAsArray(obj, "third key! can? be$ anything&"), 1));

    JsonDestroy(obj);
}

static void test_parse_primitives(void)
{
    JsonElement *pri = NULL;

    const char *data = "\"foo\"";
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &pri));
    assert_string_equal("foo", JsonPrimitiveGetAsString(pri));
    JsonDestroy(pri);

    data = "-123";
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &pri));
    assert_true(-123 == JsonPrimitiveGetAsInteger(pri));
    JsonDestroy(pri);

    data = "1.23";
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &pri));
    assert_double_close(1.23, JsonPrimitiveGetAsReal(pri));
    JsonDestroy(pri);

    data = "true";
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &pri));
    assert_true(JsonPrimitiveGetAsBool(pri));
    JsonDestroy(pri);
}

static void test_parse_array_simple(void)
{
    const char *data = ARRAY_SIMPLE;
    JsonElement *arr = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &arr));

    assert_string_equal(JsonArrayGetAsString(arr, 1), "two");
    assert_string_equal(JsonArrayGetAsString(arr, 0), "one");

    JsonDestroy(arr);
}

static void test_parse_object_compound(void)
{
    const char *data = OBJECT_COMPOUND;
    JsonElement *obj = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &obj));

    assert_string_equal(JsonObjectGetAsString(obj, "first"), "one");

    JsonElement *second = JsonObjectGetAsObject(obj, "second");

    assert_string_equal(JsonObjectGetAsString(second, "third"), "three");

    JsonElement *fourth = JsonObjectGetAsObject(obj, "fourth");

    assert_string_equal(JsonObjectGetAsString(fourth, "fifth"), "five");

    JsonDestroy(obj);
}

static void test_parse_object_diverse(void)
{
    {
        const char *data =
            "{ \"a\": 1, \"b\": \"snookie\", \"c\": 1.0, \"d\": {}, \"e\": [], \"f\": true, \"g\": false, \"h\": null }";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data =
            "{\"a\":1,\"b\":\"snookie\",\"c\":1.0,\"d\":{},\"e\":[],\"f\":true,\"g\":false,\"h\":null}";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }
}

static void test_parse_array_object(void)
{
    const char *data = ARRAY_OBJECT;
    JsonElement *arr = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &arr));

    JsonElement *first = JsonArrayGetAsObject(arr, 0);

    assert_string_equal(JsonObjectGetAsString(first, "first"), "one");

    JsonDestroy(arr);
}

static void test_iterator_current(void)
{
    const char *data = ARRAY_SIMPLE;
    JsonElement *arr = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &arr));

    JsonElement *json = JsonObjectCreate(1);
    JsonObjectAppendArray(json, "array", arr);

    JsonIterator it = JsonIteratorInit(json);
    while (JsonIteratorNextValue(&it) != NULL)
    {
        assert_int_equal(
            (int) JsonIteratorCurrentElementType(&it),
            (int) JSON_ELEMENT_TYPE_CONTAINER);
        assert_int_equal(
            (int) JsonIteratorCurrentContainerType(&it),
            (int) JSON_CONTAINER_TYPE_ARRAY);
        assert_string_equal(JsonIteratorCurrentKey(&it), "array");
    }

    JsonDestroy(json);
}

static bool VisitPrimitive(JsonElement *primitive, void *data)
{
    assert_int_equal(JsonGetElementType(primitive), JSON_ELEMENT_TYPE_PRIMITIVE);

    Writer *writer = data;
    WriterWrite(writer, JsonPrimitiveGetAsString(primitive));
    WriterWriteChar(writer, ',');
    return true;
}

static bool VisitArray(JsonElement *array, void *data)
{
    assert_int_equal(JsonGetElementType(array), JSON_ELEMENT_TYPE_CONTAINER);
    assert_int_equal(JsonGetContainerType(array), JSON_CONTAINER_TYPE_ARRAY);

    Writer *writer = data;
    WriterWriteF(writer, "[%zd]", JsonLength(array));
    return true;
}

static bool VisitObject(JsonElement *object, void *data)
{
    assert_int_equal(JsonGetElementType(object), JSON_ELEMENT_TYPE_CONTAINER);
    assert_int_equal(JsonGetContainerType(object), JSON_CONTAINER_TYPE_OBJECT);

    Writer *writer = data;
    WriterWriteChar(writer, '{');
    JsonIterator iter = JsonIteratorInit(object);
    while (JsonIteratorHasMore(&iter))
    {
        WriterWrite(writer, JsonIteratorNextKey(&iter));
        WriterWriteChar(writer, ',');
    }
    WriterWriteChar(writer, '}');
    return true;
}

static bool VisitArrayAbortOnEmpty(JsonElement *array, void *data)
{
    assert_int_equal(JsonGetElementType(array), JSON_ELEMENT_TYPE_CONTAINER);
    assert_int_equal(JsonGetContainerType(array), JSON_CONTAINER_TYPE_ARRAY);

    Writer *writer = data;
    WriterWriteF(writer, "[%zd]", JsonLength(array));
    return (JsonLength(array) != 0);
}

static void test_json_walk(void)
{

    JsonElement *data = LoadTestFile("sample.json");
    assert_true(data != NULL);

    Writer *trace_writer = StringWriter();
    bool ret = JsonWalk(data, VisitObject, VisitArray, VisitPrimitive, (void *) trace_writer);
    assert_true(ret);

    const char *expected_trace = "{primitive1,array0,array1,obj0,obj1,}value1,[0][2]value2,value3,"
                                 "{}{primitive2,array2,obj2,}value4,[1]value5,{array3,primitive3,}[0]value6,";
    assert_string_equal(StringWriterData(trace_writer), expected_trace);
    WriterClose(trace_writer);

    trace_writer = StringWriter();
    ret = JsonWalk(data, VisitObject, VisitArrayAbortOnEmpty, VisitPrimitive, (void *) trace_writer);
    assert_false(ret);
    expected_trace = "{primitive1,array0,array1,obj0,obj1,}value1,[0]";
    assert_string_equal(StringWriterData(trace_writer), expected_trace);
    WriterClose(trace_writer);

    JsonDestroy(data);
}

static void test_parse_empty_string(void)
{
    const char *data = "";
    JsonElement *json = NULL;
    assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));

    assert_false(json);

    data = "\"\"";
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));

    assert_string_equal("", JsonPrimitiveGetAsString(json));

    JsonDestroy(json);
}

static char *JsonToString(const JsonElement *json)
{
    Writer *w = StringWriter();
    JsonWriteCompact(w, json);
    return StringWriterClose(w);
}

static void test_parse_escaped_string(void)
{
    {
        const char *data = "\"\\\\\"";
        const char *original = data;
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));

        assert_string_equal("\\", JsonPrimitiveGetAsString(json));

        char *out = JsonToString(json);
        assert_string_equal(original, out);
        free(out);

        JsonDestroy(json);
    }

    {
        // included by paranoia from Redmine #5773
        const char *data = "\"/\\\\//\\\\/\\\\\"";
        const char *original = data;
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));

        assert_string_equal("/\\//\\/\\", JsonPrimitiveGetAsString(json));

        char *out = JsonToString(json);
        assert_string_equal(original, out);
        free(out);

        JsonDestroy(json);
    }

    {
        const char *data = "\"x\\tx\"";
        const char *original = data;
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));

        assert_string_equal("x\tx", JsonPrimitiveGetAsString(json));

        char *out = JsonToString(json);
        assert_string_equal(original, out);
        free(out);

        JsonDestroy(json);
    }
}

static void test_parse_big_numbers(void)
{
#define JSON_TEST_BIG_NUMBER "9999999999"
#define JSON_TEST_BIG_NUMBER_INT64 9999999999LL
    // JsonPrimitiveGetAsInt64():
    {
        const char *data = "[" JSON_TEST_BIG_NUMBER "]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json != NULL);

        const JsonElement *const primitive = JsonArrayGet(json, 0);
        int64_t number;
        const int error_code = JsonPrimitiveGetAsInt64(primitive, &number);
        assert_int_equal(error_code, 0);
        char *result;
        xasprintf(&result, "%jd", (intmax_t) number);
        assert_string_equal(result, JSON_TEST_BIG_NUMBER);
        free(result);
        JsonDestroy(json);
    }
    {
        const char *data = "[-" JSON_TEST_BIG_NUMBER "]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json != NULL);

        const JsonElement *const primitive = JsonArrayGet(json, 0);
        int64_t number;
        const int error_code = JsonPrimitiveGetAsInt64(primitive, &number);
        assert_int_equal(error_code, 0);
        char *result;
        xasprintf(&result, "%jd", (intmax_t) number);
        assert_string_equal(result, "-" JSON_TEST_BIG_NUMBER);
        free(result);
        JsonDestroy(json);
    }
    // JsonPrimitiveGetAsInt64DefaultOnError():
    {
        const char *data = "[" JSON_TEST_BIG_NUMBER "]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json != NULL);

        const JsonElement *const primitive = JsonArrayGet(json, 0);
        const int64_t number = JsonPrimitiveGetAsInt64DefaultOnError(primitive, -1);
        char *result;
        xasprintf(&result, "%jd", (intmax_t) number);
        assert_string_equal(result, JSON_TEST_BIG_NUMBER);
        free(result);
        JsonDestroy(json);
    }
    {
        const char *data = "[-" JSON_TEST_BIG_NUMBER "]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json != NULL);

        const JsonElement *const primitive = JsonArrayGet(json, 0);
        const int64_t number = JsonPrimitiveGetAsInt64DefaultOnError(primitive, -1);
        char *result;
        xasprintf(&result, "%jd", (intmax_t) number);
        assert_string_equal(result, "-" JSON_TEST_BIG_NUMBER);
        free(result);
        JsonDestroy(json);
    }
    // JsonPrimitiveGetAsInt64ExitOnError():
    {
        const char *data = "[" JSON_TEST_BIG_NUMBER "]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json != NULL);

        const JsonElement *const primitive = JsonArrayGet(json, 0);
        const int64_t number = JsonPrimitiveGetAsInt64ExitOnError(primitive);
        char *result;
        xasprintf(&result, "%jd", (intmax_t) number);
        assert_string_equal(result, JSON_TEST_BIG_NUMBER);
        free(result);
        JsonDestroy(json);
    }
    {
        const char *data = "[-" JSON_TEST_BIG_NUMBER "]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json != NULL);

        const JsonElement *const primitive = JsonArrayGet(json, 0);
        const int64_t number = JsonPrimitiveGetAsInt64ExitOnError(primitive);
        char *result;
        xasprintf(&result, "%jd", (intmax_t) number);
        assert_string_equal(result, "-" JSON_TEST_BIG_NUMBER);
        free(result);
        JsonDestroy(json);
    }
    // JsonIntegerCreate64
    {
        JsonElement *object = JsonObjectCreate(1);
        JsonElement *size = JsonIntegerCreate64(JSON_TEST_BIG_NUMBER_INT64);
        JsonObjectAppendElement(object, "64bitvalue", size);
        assert_int_equal(1, JsonLength(object));
        JsonElement *const primitive = JsonObjectGet(object, "64bitvalue");
        assert_string_equal(
            JSON_TEST_BIG_NUMBER, JsonPrimitiveGetAsString(primitive));
        JsonDestroy(object);
    }
    // JsonObjectAppendInteger64
    {
        JsonElement *object = JsonObjectCreate(1);
        JsonObjectAppendInteger64(object, "64bitvalue", JSON_TEST_BIG_NUMBER_INT64);
        assert_int_equal(1, JsonLength(object));
        const JsonElement *const primitive =
            JsonObjectGet(object, "64bitvalue");
        int64_t number;
        const int error_code = JsonPrimitiveGetAsInt64(primitive, &number);
        assert_int_equal(error_code, 0);
        char *result;
        xasprintf(&result, "%" PRIi64, number);
        assert_string_equal(result, JSON_TEST_BIG_NUMBER);
        free(result);
        JsonDestroy(object);
    }
#undef JSON_TEST_BIG_NUMBER
#undef JSON_TEST_BIG_NUMBER_INT64
}

static void test_parse_good_numbers(void)
{
    {
        const char *data = "[0.1]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[0.1234567890123456789]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[0.1234e10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[0.1234e+10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[0.1234e-10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[1203e10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[1203e+10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[123e-10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[0e-10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[0.0e-10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[-0.0e-10]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }
}

static void test_parse_bad_numbers(void)
{
    {
        const char *data = "[01]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }

    {
        const char *data = "[01.1]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }

    {
        const char *data = "[1.]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }

    {
        const char *data = "[e10]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }

    {
        const char *data = "[-e10]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }

    {
        const char *data = "[+2]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }

    {
        const char *data = "[1e]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }

    {
        const char *data = "[e10]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    }
}

static void test_parse_trim(void)
{
    const char *data = "           []    ";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));

    assert_true(json);

    JsonDestroy(json);
}

static void test_parse_array_extra_closing(void)
{
    const char *data = "  []]";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));

    assert_true(json);

    JsonDestroy(json);
}

static void test_parse_all(void)
{
    const char *data = "\"\"a";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "\"\""; // Good
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseAll(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    data = "{}b";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "{}"; // Good
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseAll(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    data = "[]c";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "{}}";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "{\"d\": \"e\"}}";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "[]]";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "[[]]]";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "[[[]]]]";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "  []]";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "\"some\": [ \"json\" ] }";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "{ \"some\": [ \"json\" ] } [";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "[\"some\", \"json\"]!";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "    [\"some\", \"json\"]a";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);

    data = "[\"some\", \"json\"] {\"foo\": \"var\"}   ";
    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);
}

static void test_parse_array_diverse(void)
{
    {
        const char *data = "[1, \"snookie\", 1.0, {}, [], true, false, null ]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }

    {
        const char *data = "[1,\"snookie\",1.0,{},[],true,false,null]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        JsonDestroy(json);
    }
}

static void test_parse_bad_apple2(void)
{
    const char *data = "][";
    JsonElement *json = NULL;
    assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));

    assert_false(json);
}

static void test_parse_object_garbage(void)
{
    {
        const char *data = "{ \"first\": 1, garbage \"second\": 2 }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{ \"first\": 1 garbage \"second\": 2 }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{ \"first\": garbage, \"second\": 2 }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{ \"first\": garbage \"second\": 2 }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }
}

static void test_parse_object_nested_garbage(void)
{
    {
        const char *data = "{ \"first\": { garbage } }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{ \"first\": [ garbage ] }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }
}

static void test_parse_array_garbage(void)
{
    {
        const char *data = "[1, garbage]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[1 garbage]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[garbage]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[garbage, 1]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }
}

static void test_parse_array_nested_garbage(void)
{
    {
        const char *data = "[1, [garbage]]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[1, { garbage }]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }
}

static void test_array_extend(void)
{
    {
        JsonElement *a = JsonArrayCreate(6);
        JsonArrayAppendString(a, "one");
        JsonArrayAppendString(a, "two");
        JsonArrayAppendString(a, "three");

        JsonElement *b = JsonArrayCreate(3);
        JsonArrayAppendString(b, "four");
        JsonArrayAppendString(b, "five");
        JsonArrayAppendString(b, "six");

        JsonArrayExtend(a, b);

        assert_int_equal(JsonLength(a), 6);
        assert_string_equal(JsonArrayGetAsString(a, 0), "one");
        assert_string_equal(JsonArrayGetAsString(a, 1), "two");
        assert_string_equal(JsonArrayGetAsString(a, 2), "three");
        assert_string_equal(JsonArrayGetAsString(a, 3), "four");
        assert_string_equal(JsonArrayGetAsString(a, 4), "five");
        assert_string_equal(JsonArrayGetAsString(a, 5), "six");

        JsonDestroy(a);
    }
    {
        JsonElement *a = JsonArrayCreate(3);
        JsonArrayAppendString(a, "one");
        JsonArrayAppendString(a, "two");
        JsonArrayAppendString(a, "three");

        JsonElement *b = JsonArrayCreate(0);

        JsonArrayExtend(a, b);

        assert_int_equal(JsonLength(a), 3);
        assert_string_equal(JsonArrayGetAsString(a, 0), "one");
        assert_string_equal(JsonArrayGetAsString(a, 1), "two");
        assert_string_equal(JsonArrayGetAsString(a, 2), "three");

        JsonDestroy(a);
    }
    {
        JsonElement *a = JsonArrayCreate(3);

        JsonElement *b = JsonArrayCreate(3);
        JsonArrayAppendString(b, "four");
        JsonArrayAppendString(b, "five");
        JsonArrayAppendString(b, "six");

        JsonArrayExtend(a, b);

        assert_int_equal(JsonLength(a), 3);
        assert_string_equal(JsonArrayGetAsString(a, 0), "four");
        assert_string_equal(JsonArrayGetAsString(a, 1), "five");
        assert_string_equal(JsonArrayGetAsString(a, 2), "six");

        JsonDestroy(a);
    }
}

static void test_array_remove_range(void)
{
    {
        // remove whole
        JsonElement *arr = JsonArrayCreate(5);

        JsonArrayAppendString(arr, "one");
        JsonArrayAppendString(arr, "two");
        JsonArrayAppendString(arr, "three");
        JsonArrayRemoveRange(arr, 0, 2);

        assert_int_equal(JsonLength(arr), 0);

        JsonDestroy(arr);
    }

    {
        // remove middle
        JsonElement *arr = JsonArrayCreate(5);

        JsonArrayAppendString(arr, "one");
        JsonArrayAppendString(arr, "two");
        JsonArrayAppendString(arr, "three");
        JsonArrayRemoveRange(arr, 1, 1);

        assert_int_equal(JsonLength(arr), 2);
        assert_string_equal(JsonArrayGetAsString(arr, 0), "one");
        assert_string_equal(JsonArrayGetAsString(arr, 1), "three");

        JsonDestroy(arr);
    }

    {
        // remove rest
        JsonElement *arr = JsonArrayCreate(5);

        JsonArrayAppendString(arr, "one");
        JsonArrayAppendString(arr, "two");
        JsonArrayAppendString(arr, "three");
        JsonArrayRemoveRange(arr, 1, 2);

        assert_int_equal(JsonLength(arr), 1);
        assert_string_equal(JsonArrayGetAsString(arr, 0), "one");

        JsonDestroy(arr);
    }

    {
        // remove but last
        JsonElement *arr = JsonArrayCreate(5);

        JsonArrayAppendString(arr, "one");
        JsonArrayAppendString(arr, "two");
        JsonArrayAppendString(arr, "three");
        JsonArrayRemoveRange(arr, 0, 1);

        assert_int_equal(JsonLength(arr), 1);
        assert_string_equal(JsonArrayGetAsString(arr, 0), "three");

        JsonDestroy(arr);
    }
}

static void test_remove_key_from_object(void)
{
    JsonElement *object = JsonObjectCreate(3);

    JsonObjectAppendInteger(object, "one", 1);
    JsonObjectAppendInteger(object, "two", 2);
    JsonObjectAppendInteger(object, "three", 3);

    JsonObjectRemoveKey(object, "two");

    assert_int_equal(2, JsonLength(object));

    JsonDestroy(object);
}

/* Strict parsing: JsonParseStrict() and JsonParseFileStrict() refuse what the
 * lenient functions repair, and the lenient functions stay as they were. */

static void assert_strict_refused(
    const char *const text, const JsonParseError expected)
{
    const char *data = text;
    JsonElement *json = NULL;
    assert_int_equal(expected, JsonParseStrict(&data, &json));
    assert_true(json == NULL);
}

static void test_parse_strict_valid(void)
{
    const char *data =
        "{\"vars\": {\"a\": \"1\", \"b\": null},"
        " \"list\": [{\"k\": 1}, {\"k\": 2}, [{\"k\": 3}]],"
        " \"other\": {\"a\": \"2\"}, \"plain\": true}";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    assert_string_equal("1", JsonObjectGetAsString(
                                 JsonObjectGetAsObject(json, "vars"), "a"));
    assert_string_equal("2", JsonObjectGetAsString(
                                 JsonObjectGetAsObject(json, "other"), "a"));
    assert_int_equal(3, JsonLength(JsonObjectGet(json, "list")));
    JsonDestroy(json);

    /* Whitespace is the only thing allowed after the document. */
    data = "{}  \n\t\r ";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    data = "[]\n";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    JsonDestroy(json);

    data = " 12 ";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    JsonDestroy(json);

    data = "\"s\"";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    JsonDestroy(json);
}

static void test_parse_strict_duplicate_member(void)
{
    /* Top level, nested, and in an object inside an array. */
    assert_strict_refused(
        "{\"a\": \"1\", \"a\": \"2\"}",
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);
    assert_strict_refused(
        "{\"vars\": {\"x\": \"1\", \"y\": \"2\", \"x\": \"3\"}}",
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);
    assert_strict_refused(
        "{\"l\": [{\"k\": 1}, {\"k\": 1, \"k\": 2}]}",
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);
    assert_strict_refused(
        "[[{\"a\": {\"b\": [], \"b\": []}}]]",
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);

    /* Whatever the value types, however far apart the two are. */
    assert_strict_refused(
        "{\"a\": {}, \"b\": 1, \"a\": []}",
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);
    assert_strict_refused(
        "{\"a\": true, \"a\": true}", JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);
}

static void test_parse_strict_non_json_forms(void)
{
    /* Unquoted object keys are refused in strict mode. */
    assert_strict_refused("{a: 1}", JSON_PARSE_ERROR_OBJECT_BAD_SYMBOL);
    assert_strict_refused("{plain: true}", JSON_PARSE_ERROR_OBJECT_BAD_SYMBOL);
    assert_strict_refused(
        "{\"ok\": 1, bad: 2}", JSON_PARSE_ERROR_OBJECT_BAD_SYMBOL);

    /* Missing colons between object key and value are refused. */
    assert_strict_refused("{\"a\" \"b\"}", JSON_PARSE_ERROR_OBJECT_COLON);
    assert_strict_refused("{\"a\" 1}", JSON_PARSE_ERROR_OBJECT_COLON);
    assert_strict_refused("{\"a\" []}", JSON_PARSE_ERROR_OBJECT_COLON);
    assert_strict_refused("{\"a\" {}}", JSON_PARSE_ERROR_OBJECT_COLON);
    assert_strict_refused("{\"a\" true}", JSON_PARSE_ERROR_OBJECT_COLON);
    assert_strict_refused("{\"a\" null}", JSON_PARSE_ERROR_OBJECT_COLON);

    /* \u0000 escapes are refused in strict mode (keys, values, arrays, top). */
    assert_strict_refused(
        "\"a\\u0000b\"", JSON_PARSE_ERROR_STRING_NULL_ESCAPE);
    assert_strict_refused(
        "{\"a\\u0000b\": 1}", JSON_PARSE_ERROR_STRING_NULL_ESCAPE);
    assert_strict_refused(
        "{\"a\": \"x\\u0000y\"}", JSON_PARSE_ERROR_STRING_NULL_ESCAPE);
    assert_strict_refused(
        "[\"\\u0000\"]", JSON_PARSE_ERROR_STRING_NULL_ESCAPE);
    assert_strict_refused(
        "\"\\\\\\u0000\"", JSON_PARSE_ERROR_STRING_NULL_ESCAPE);

    /* Escaped backslash followed by literal u0000 is not a null escape. */
    const char *data = "{\"a\": \"\\\\u0000\"}";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    /* Truncated \u escapes are rejected as invalid escapes without OOB read. */
    assert_strict_refused("\"\\u", JSON_PARSE_ERROR_STRING_INVALID_ESCAPE);
    assert_strict_refused("\"\\u00", JSON_PARSE_ERROR_STRING_INVALID_ESCAPE);

    /* Lenient JsonParse() still accepts unquoted keys. */
    data = "{plain: true}";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);
}

static void test_parse_strict_commas_and_array_separators(void)
{
    /* Trailing and leading commas in objects and arrays are refused. */
    assert_strict_refused("{\"a\": 1,}", JSON_PARSE_ERROR_OBJECT_COMMA);
    assert_strict_refused("{, \"a\": 1}", JSON_PARSE_ERROR_OBJECT_COMMA);
    assert_strict_refused("{,}", JSON_PARSE_ERROR_OBJECT_COMMA);
    assert_strict_refused("[1,]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("[, 1]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("[,]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("{\"a\": [1,]}", JSON_PARSE_ERROR_ARRAY_COMMA);

    /* Missing commas between array elements are refused. */
    assert_strict_refused("[\"a\" \"b\"]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("[1 2]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("[true null \"x\"]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("[[] \"a\"]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("[{} 1]", JSON_PARSE_ERROR_ARRAY_COMMA);
    assert_strict_refused("[1 []]", JSON_PARSE_ERROR_ARRAY_START);
    assert_strict_refused("[\"a\" {}]", JSON_PARSE_ERROR_ARRAY_START);
    assert_strict_refused("[1\"a\"]", JSON_PARSE_ERROR_NUMBER_BAD_SYMBOL);
    assert_strict_refused("[\"a\"1]", JSON_PARSE_ERROR_ARRAY_COMMA);

    /* Valid arrays and objects still parse cleanly. */
    const char *data = "[1, \"a\", true, false, null, [], {\"k\": \"v\"}]";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    assert_int_equal(7, JsonLength(json));
    JsonDestroy(json);

    /* Lenient JsonParse() keeps accepting trailing commas. */
    data = "{\"a\": 1,}";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    data = "[1,]";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);
}

static void test_parse_strict_control_chars_and_escapes(void)
{
    /* Raw unescaped control characters 0x01..0x1f are refused in strings
     * (values and keys). */
    for (unsigned char c = 1; c < 0x20; c++)
    {
        char val_doc[] = "{\"k\": \"aXb\"}";
        val_doc[8] = (char) c;
        assert_strict_refused(val_doc, JSON_PARSE_ERROR_STRING_CONTROL_CHAR);

        char key_doc[] = "{\"aXb\": 1}";
        key_doc[3] = (char) c;
        assert_strict_refused(key_doc, JSON_PARSE_ERROR_STRING_CONTROL_CHAR);
    }

    /* Escaped control characters and valid escapes still parse. */
    const char *data = "\"\\b\\f\\n\\r\\t\\/\\\\\\\"\\u001f\\u0041\"";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    /* Valid UTF-16 surrogate pair parses cleanly. */
    data = "\"\\uD83D\\uDE00\"";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    /* Unrecognized escapes and malformed \u / unpaired surrogates are refused. */
    const char *const bad_escapes[] = {
        "\"\\a\"",
        "\"\\U0000\"",
        "\"\\'\"",
        "\"\\x41\"",
        "\"\\1\"",
        "\"\\u\"",
        "\"\\u1\"",
        "\"\\u123\"",
        "\"\\u123g\"",
        "\"\\uD800\"",
        "\"\\uD83Dx\"",
        "\"\\uD83D\\u0041\"",
        "\"\\uD800\\uD800\\uDC00\"",
        "\"\\uD83D\\uDE00\\uDC00\"",
        "\"\\uD83D\\\\uDE00\"",
        "\"\\uDC00\"",
        "\"\\uDFFF\"",
        "{\"\\a\": 1}",
        "[\"\\uD83D\"]",
        NULL
    };
    for (size_t i = 0; bad_escapes[i] != NULL; i++)
    {
        assert_strict_refused(
            bad_escapes[i], JSON_PARSE_ERROR_STRING_INVALID_ESCAPE);
    }

    /* Exact-sized heap buffers for truncated surrogate pairs (ASan OOB proof). */
    const char *const trunc_cases[] = {
        "\"\\u",
        "\"\\u00",
        "\"\\uD83D",
        "\"\\uD83D\\",
        "\"\\uD83D\\u",
        "\"\\uD83D\\uDE",
        NULL
    };
    for (size_t i = 0; trunc_cases[i] != NULL; i++)
    {
        const size_t len = strlen(trunc_cases[i]);
        char *buf = xmalloc(len + 1);
        memcpy(buf, trunc_cases[i], len + 1);
        const char *p = buf;
        JsonElement *out = NULL;
        assert_int_equal(
            JSON_PARSE_ERROR_STRING_INVALID_ESCAPE, JsonParseStrict(&p, &out));
        assert_true(out == NULL);
        free(buf);
    }

    /* Lenient JsonParse() still keeps unrecognized escapes and raw tab. */
    data = "\"\\a\t\"";
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);
}

static void test_parse_strict_negative_leading_zero(void)
{
    assert_strict_refused("-00", JSON_PARSE_ERROR_NUMBER_DUPLICATE_ZERO);
    assert_strict_refused(
        "-01", JSON_PARSE_ERROR_NUMBER_EXPONENT_FOLLOW_LEADING_ZERO);
    assert_strict_refused("-00.5", JSON_PARSE_ERROR_NUMBER_DUPLICATE_ZERO);
    assert_strict_refused(
        "-01e2", JSON_PARSE_ERROR_NUMBER_EXPONENT_FOLLOW_LEADING_ZERO);
    assert_strict_refused(
        "[-01]", JSON_PARSE_ERROR_NUMBER_EXPONENT_FOLLOW_LEADING_ZERO);
    assert_strict_refused(
        "{\"n\": -00}", JSON_PARSE_ERROR_NUMBER_DUPLICATE_ZERO);
    assert_strict_refused("1e2.3", JSON_PARSE_ERROR_NUMBER_BAD_SYMBOL);
    assert_strict_refused("1E-2.5", JSON_PARSE_ERROR_NUMBER_BAD_SYMBOL);
    assert_strict_refused("0e0.0", JSON_PARSE_ERROR_NUMBER_BAD_SYMBOL);
    assert_strict_refused("[1e2.3]", JSON_PARSE_ERROR_NUMBER_BAD_SYMBOL);
    assert_strict_refused(
        "{\"n\": 1E-2.5}", JSON_PARSE_ERROR_NUMBER_BAD_SYMBOL);

    /* Valid zero and negative numbers still parse cleanly. */
    const char *const valid_nums[] = {
        "0", "-0", "-0.5", "-0e1", "-0E-1", "1e-05", "0e-05", "-10",
        "[-0, -0.5, -0e1, 1e-05]",
        "{\"a\": -0, \"b\": -0.5, \"c\": -0e1, \"d\": 1e-05}",
        NULL
    };
    for (size_t i = 0; valid_nums[i] != NULL; i++)
    {
        const char *data = valid_nums[i];
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
        assert_true(json != NULL);
        JsonDestroy(json);
    }

    /* Lenient JsonParse() keeps accepting -01, but still rejects decimal point
     * after exponent (which would poison REAL -> StringToDouble assert). */
    const char *data = "-01";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    const char *const bad_lenient_exp_dot[] = {
        "1e2.3", "1E-2.5", "0e0.0", "[1e2.3]", NULL
    };
    for (size_t i = 0; bad_lenient_exp_dot[i] != NULL; i++)
    {
        data = bad_lenient_exp_dot[i];
        json = NULL;
        assert_int_equal(
            JSON_PARSE_ERROR_NUMBER_BAD_SYMBOL, JsonParse(&data, &json));
        assert_true(json == NULL);
    }
}

static void test_parse_strict_utf8(void)
{
    /* Valid 2-, 3-, and 4-byte RFC 3629 UTF-8 sequences in values and keys. */
    const char *const valid_utf8[] = {
        "\"\xc2\xa2\"",                   /* U+00A2 */
        "\"\xc3\xa9\"",                   /* U+00E9 */
        "\"\xdf\xbf\"",                   /* U+07FF */
        "\"\xe0\xa0\x80\"",               /* U+0800 */
        "\"\xe2\x82\xac\"",               /* U+20AC */
        "\"\xed\x9f\xbf\"",               /* U+D7FF */
        "\"\xee\x80\x80\"",               /* U+E000 */
        "\"\xef\xbf\xbf\"",               /* U+FFFF */
        "\"\xf0\x90\x80\x80\"",           /* U+10000 */
        "\"\xf0\x9f\x98\x80\"",           /* U+1F600 */
        "\"\xf4\x8f\xbf\xbf\"",           /* U+10FFFF */
        "{\"caf\xc3\xa9\": \"\xf0\x9f\x98\x80\"}",
        NULL
    };
    for (size_t i = 0; valid_utf8[i] != NULL; i++)
    {
        const char *data = valid_utf8[i];
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
        assert_true(json != NULL);
        JsonDestroy(json);
    }

    /* Verify decoded multi-byte UTF-8 bytes are preserved intact. */
    {
        const char *data = "{\"k\": \"caf\xc3\xa9 \xf0\x9f\x98\x80\"}";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
        assert_string_equal(
            "caf\xc3\xa9 \xf0\x9f\x98\x80", JsonObjectGetAsString(json, "k"));
        JsonDestroy(json);
    }

    /* Invalid UTF-8 sequences are refused in strict mode. */
    const char *const invalid_utf8[] = {
        "\"\x80\"",                       /* lone continuation */
        "\"\xbf\"",                       /* lone continuation */
        "\"\xc0\xaf\"",                   /* overlong '/' */
        "\"\xc1\xbf\"",                   /* overlong 2-byte */
        "\"\xe0\x9f\xbf\"",               /* overlong 3-byte */
        "\"\xed\xa0\x80\"",               /* UTF-8 high surrogate U+D800 */
        "\"\xed\xbf\xbf\"",               /* UTF-8 low surrogate U+DFFF */
        "\"\xf0\x8f\xbf\xbf\"",           /* overlong 4-byte */
        "\"\xf4\x90\x80\x80\"",           /* > U+10FFFF */
        "\"\xf5\x80\x80\x80\"",           /* invalid lead byte */
        "\"\xfe\"",                       /* invalid byte */
        "\"\xff\"",                       /* invalid byte */
        "\"\xc2\"",                       /* truncated 2-byte before '"' */
        "\"\xe2\x82\"",                   /* truncated 3-byte before '"' */
        "\"\xf0\x9f\x98\"",               /* truncated 4-byte before '"' */
        "{\"\x80\": 1}",                  /* invalid UTF-8 in object key */
        "[\"\xed\xa0\x80\"]",             /* invalid UTF-8 in array */
        NULL
    };
    for (size_t i = 0; invalid_utf8[i] != NULL; i++)
    {
        assert_strict_refused(
            invalid_utf8[i], JSON_PARSE_ERROR_STRING_INVALID_UTF8);
    }

    /* Exact-sized heap buffers truncated right at NUL (ASan OOB proof). */
    const char *const trunc_utf8[] = {
        "\"\xc2",
        "\"\xe2",
        "\"\xe2\x82",
        "\"\xf0",
        "\"\xf0\x9f",
        "\"\xf0\x9f\x98",
        NULL
    };
    for (size_t i = 0; trunc_utf8[i] != NULL; i++)
    {
        const size_t len = strlen(trunc_utf8[i]);
        char *buf = xmalloc(len + 1);
        memcpy(buf, trunc_utf8[i], len + 1);
        const char *p = buf;
        JsonElement *out = NULL;
        assert_int_equal(
            JSON_PARSE_ERROR_STRING_INVALID_UTF8, JsonParseStrict(&p, &out));
        assert_true(out == NULL);
        free(buf);
    }

    /* Lenient JsonParse() keeps accepting raw non-UTF-8 bytes. */
    const char *data = "\"\x80\xff\"";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);
}

static void test_parse_strict_max_members(void)
{
    /* 65536 members in one object succeed; 65537 exceed the cap. */
    const size_t max_members = 65536;
    Writer *w = StringWriter();
    WriterWriteChar(w, '{');
    for (size_t i = 0; i < max_members; i++)
    {
        if (i > 0)
        {
            WriterWriteChar(w, ',');
        }
        WriterWriteF(w, "\"m%zu\":0", i);
    }
    WriterWriteChar(w, '}');
    char *ok_doc = StringWriterClose(w);

    const char *data = ok_doc;
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    assert_int_equal(max_members, JsonLength(json));
    JsonDestroy(json);
    free(ok_doc);

    w = StringWriter();
    WriterWriteChar(w, '{');
    for (size_t i = 0; i <= max_members; i++)
    {
        if (i > 0)
        {
            WriterWriteChar(w, ',');
        }
        WriterWriteF(w, "\"m%zu\":0", i);
    }
    WriterWriteChar(w, '}');
    char *too_many = StringWriterClose(w);
    assert_strict_refused(too_many, JSON_PARSE_ERROR_MAX_MEMBERS);
    free(too_many);

    /* Large array of null/boolean literals parses in O(n) without strlen O(n^2). */
    const size_t lit_count = 30000;
    w = StringWriter();
    WriterWriteChar(w, '[');
    for (size_t i = 0; i < lit_count; i++)
    {
        if (i > 0)
        {
            WriterWriteChar(w, ',');
        }
        const char *lit = (i % 3 == 0) ? "null" : ((i % 3 == 1) ? "true" : "false");
        WriterWrite(w, lit);
    }
    WriterWriteChar(w, ']');
    char *lit_doc = StringWriterClose(w);
    data = lit_doc;
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    assert_int_equal(lit_count, JsonLength(json));
    JsonDestroy(json);
    free(lit_doc);
}

static void test_parse_strict_large_object_linear(void)
{
    /* 10,000 unique keys: exercises O(n) hash-set duplicate tracking and
     * direct append without O(n^2) rescans. */
    const size_t n = 10000;
    Writer *w = StringWriter();
    WriterWriteChar(w, '{');
    for (size_t i = 0; i < n; i++)
    {
        if (i > 0)
        {
            WriterWriteChar(w, ',');
        }
        WriterWriteF(w, "\"k%zu\":%zu", i, i);
    }
    WriterWriteChar(w, '}');
    char *doc = StringWriterClose(w);

    const char *data = doc;
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    assert_int_equal(n, JsonLength(json));
    JsonDestroy(json);
    free(doc);

    /* Duplicate at the tail of a large object is still refused. */
    w = StringWriter();
    WriterWriteChar(w, '{');
    for (size_t i = 0; i < n; i++)
    {
        WriterWriteF(w, "\"k%zu\":%zu,", i, i);
    }
    WriterWrite(w, "\"k0\":0}");
    doc = StringWriterClose(w);

    data = doc;
    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY, JsonParseStrict(&data, &json));
    assert_true(json == NULL);
    free(doc);
}

static void test_parse_strict_max_depth(void)
{
    /* 128 nested arrays succeed; 129 exceed the cap. */
    const size_t max_depth = 128;
    Writer *w = StringWriter();
    for (size_t i = 0; i < max_depth; i++)
    {
        WriterWriteChar(w, '[');
    }
    for (size_t i = 0; i < max_depth; i++)
    {
        WriterWriteChar(w, ']');
    }
    char *ok_arr = StringWriterClose(w);

    const char *data = ok_arr;
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);
    free(ok_arr);

    w = StringWriter();
    for (size_t i = 0; i < max_depth + 1; i++)
    {
        WriterWriteChar(w, '[');
    }
    for (size_t i = 0; i < max_depth + 1; i++)
    {
        WriterWriteChar(w, ']');
    }
    char *too_deep_arr = StringWriterClose(w);
    assert_strict_refused(too_deep_arr, JSON_PARSE_ERROR_MAX_DEPTH);
    free(too_deep_arr);

    /* Same boundary for nested objects. */
    w = StringWriter();
    for (size_t i = 0; i < max_depth; i++)
    {
        WriterWrite(w, i + 1 < max_depth ? "{\"a\":" : "{}");
    }
    for (size_t i = 1; i < max_depth; i++)
    {
        WriterWriteChar(w, '}');
    }
    char *ok_obj = StringWriterClose(w);

    data = ok_obj;
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseStrict(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);
    free(ok_obj);

    w = StringWriter();
    for (size_t i = 0; i < max_depth + 1; i++)
    {
        WriterWrite(w, i < max_depth ? "{\"a\":" : "{}");
    }
    for (size_t i = 0; i < max_depth; i++)
    {
        WriterWriteChar(w, '}');
    }
    char *too_deep_obj = StringWriterClose(w);
    assert_strict_refused(too_deep_obj, JSON_PARSE_ERROR_MAX_DEPTH);
    free(too_deep_obj);
}

static void test_parse_strict_invalid_earlier_duplicate(void)
{
    /* The lenient parser drops the null before anything can look at it. */
    const char *const text = "{\"vars\": {\"good\": null, \"good\": \"ok\"}}";

    assert_strict_refused(text, JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);

    const char *data = text;
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    JsonElement *vars = JsonObjectGetAsObject(json, "vars");
    assert_int_equal(1, JsonLength(vars));
    assert_string_equal("ok", JsonObjectGetAsString(vars, "good"));
    JsonDestroy(json);
}

static void test_parse_strict_repeated_section(void)
{
    const char *const text =
        "{\"vars\": {\"a\": \"1\"}, \"classes\": {},"
        " \"vars\": {\"b\": \"2\"}}";

    assert_strict_refused(text, JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);

    /* The lenient parser keeps only the last section. */
    const char *data = text;
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    JsonElement *vars = JsonObjectGetAsObject(json, "vars");
    assert_true(JsonObjectGet(vars, "a") == NULL);
    assert_string_equal("2", JsonObjectGetAsString(vars, "b"));
    JsonDestroy(json);
}

static void test_parse_strict_trailing_characters(void)
{
    assert_strict_refused(
        "{\"vars\": {}} not-json", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("{} {}", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("{}}", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("{}\n\n x", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("[] ,", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("12 x", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("true false", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("null x", JSON_PARSE_ERROR_INVALID_END);
    assert_strict_refused("\"s\" \"t\"", JSON_PARSE_ERROR_INVALID_END);

    /* An ordinary syntax error is still its own error. */
    assert_strict_refused("{\"a\": ", JSON_PARSE_ERROR_OBJECT_END);
    assert_strict_refused("", JSON_PARSE_ERROR_NO_DATA);
    assert_strict_refused("  \n", JSON_PARSE_ERROR_NO_DATA);

    /* The lenient functions accept the valid prefix, as before. */
    const char *data = "{\"vars\": {}} not-json";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_true(json != NULL);
    JsonDestroy(json);

    data = "{\"vars\": {}} not-json";
    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_INVALID_END, JsonParseAll(&data, &json));
    assert_true(json == NULL);
}

static void test_parse_lenient_duplicate_last_wins(void)
{
    const char *const text =
        "{\"a\": \"1\", \"b\": {\"x\": 1, \"x\": 2}, \"a\": \"3\","
        " c: 1, c: 4}";

    const char *data = text;
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
    assert_int_equal(3, JsonLength(json));
    assert_string_equal("3", JsonObjectGetAsString(json, "a"));
    assert_int_equal(1, JsonLength(JsonObjectGetAsObject(json, "b")));
    JsonDestroy(json);

    data = text;
    json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseAll(&data, &json));
    assert_string_equal("3", JsonObjectGetAsString(json, "a"));
    JsonDestroy(json);

    data = text;
    json = NULL;
    assert_int_equal(
        JSON_PARSE_OK, JsonParseWithLookup(NULL, NULL, &data, &json));
    assert_string_equal("3", JsonObjectGetAsString(json, "a"));
    JsonDestroy(json);
}

static void test_parse_strict_error_string(void)
{
    const char *const message =
        JsonParseErrorToString(JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY);
    assert_true(message != NULL);
    assert_true(strstr(message, "duplicate") != NULL);

    const char *const nul_msg =
        JsonParseErrorToString(JSON_PARSE_ERROR_STRING_NULL_ESCAPE);
    assert_true(nul_msg != NULL);
    assert_true(strstr(nul_msg, "null") != NULL);

    const char *const depth_msg =
        JsonParseErrorToString(JSON_PARSE_ERROR_MAX_DEPTH);
    assert_true(depth_msg != NULL);
    assert_true(strstr(depth_msg, "depth") != NULL);

    const char *const ctrl_msg =
        JsonParseErrorToString(JSON_PARSE_ERROR_STRING_CONTROL_CHAR);
    assert_true(ctrl_msg != NULL);
    assert_true(strstr(ctrl_msg, "control") != NULL);

    const char *const esc_msg =
        JsonParseErrorToString(JSON_PARSE_ERROR_STRING_INVALID_ESCAPE);
    assert_true(esc_msg != NULL);
    assert_true(strstr(esc_msg, "escape") != NULL);

    const char *const mem_msg =
        JsonParseErrorToString(JSON_PARSE_ERROR_MAX_MEMBERS);
    assert_true(mem_msg != NULL);
    assert_true(strstr(mem_msg, "member") != NULL);

    const char *const utf8_msg =
        JsonParseErrorToString(JSON_PARSE_ERROR_STRING_INVALID_UTF8);
    assert_true(utf8_msg != NULL);
    assert_true(strstr(utf8_msg, "UTF-8") != NULL);
}

static void write_strict_test_file(
    char *const filename, const char *const content, const size_t length)
{
    const int fd = mkstemp(filename);
    assert_true(fd >= 0);
    assert_int_equal(length, FullWrite(fd, content, length));
    close(fd);
}

static JsonParseError strict_file(
    const char *const content,
    const size_t length,
    const size_t size_max,
    JsonElement **const json_out)
{
    char filename[] = "/tmp/json_test_strict_XXXXXX";
    write_strict_test_file(filename, content, length);
    const JsonParseError error =
        JsonParseFileStrict(filename, size_max, json_out);
    unlink(filename);
    return error;
}

#define STRICT_FILE(text, size_max, json_out) \
    strict_file(text, sizeof(text) - 1, size_max, json_out)

static void test_parse_file_strict(void)
{
    JsonElement *json = NULL;
    assert_int_equal(
        JSON_PARSE_OK,
        STRICT_FILE("{\"vars\": {\"a\": \"1\"}}\n", 1024, &json));
    assert_string_equal(
        "1", JsonObjectGetAsString(JsonObjectGetAsObject(json, "vars"), "a"));
    JsonDestroy(json);

    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY,
        STRICT_FILE("{\"vars\": {\"a\": null, \"a\": \"1\"}}", 1024, &json));
    assert_true(json == NULL);

    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_OBJECT_DUPLICATE_KEY,
        STRICT_FILE("{\"vars\": {}, \"vars\": {}}", 1024, &json));
    assert_true(json == NULL);

    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_INVALID_END,
        STRICT_FILE("{\"vars\": {}} not-json\n", 1024, &json));
    assert_true(json == NULL);

    /* A null byte must not hide what follows it from the parser. */
    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_INVALID_END,
        STRICT_FILE("{\"vars\": {}}\0not-json", 1024, &json));
    assert_true(json == NULL);

    /* The size cap is kept: a larger file is refused, not cut short. */
    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_TRUNCATED,
        STRICT_FILE("{\"vars\": {\"a\": \"1\"}}", 8, &json));
    assert_true(json == NULL);

    json = NULL;
    assert_int_equal(
        JSON_PARSE_ERROR_NO_SUCH_FILE,
        JsonParseFileStrict("/nonexistent/json_test_strict", 1024, &json));
    assert_true(json == NULL);

    json = NULL;
    assert_int_equal(JSON_PARSE_ERROR_NO_DATA, STRICT_FILE("", 1024, &json));
    assert_true(json == NULL);
}

static void test_parse_file_lenient_unchanged(void)
{
    /* JsonParseFile() still repairs both, as before. */
    char filename[] = "/tmp/json_test_lenient_XXXXXX";
    const char content[] = "{\"a\": \"1\", \"a\": \"2\"} not-json\n";
    write_strict_test_file(filename, content, sizeof(content) - 1);

    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParseFile(filename, 1024, &json));
    assert_string_equal("2", JsonObjectGetAsString(json, "a"));
    JsonDestroy(json);

    json = NULL;
    assert_int_equal(
        JSON_PARSE_OK, JsonParseAnyFile(filename, 1024, &json, false));
    assert_string_equal("2", JsonObjectGetAsString(json, "a"));
    JsonDestroy(json);

    unlink(filename);
}

static void test_detach_key_from_object(void)
{
    JsonElement *object = JsonObjectCreate(3);

    JsonObjectAppendInteger(object, "one", 1);
    JsonObjectAppendInteger(object, "two", 2);
    JsonObjectAppendInteger(object, "three", 3);

    JsonElement *detached = JsonObjectDetachKey(object, "two");

    assert_int_equal(2, JsonLength(object));
    JsonDestroy(object);

    assert_int_equal(1, JsonLength(detached));
    JsonDestroy(detached);
}

static void test_parse_array_double_and_trailing_commas(void)
{
    {
        const char *data = "[ \"foo\",, \"bar\" ]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[\"foo\", , \"bar\" ]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[ \"foo\", \"bar\",, ]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[ \"foo\", \"bar\", , ]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        // We accept one, and only one, trailing comma.
        const char *data = "[ \"foo\", \"bar\", ]";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        JsonDestroy(json);
    }
}

static void test_parse_array_comma_after_brace(void)
{
    {
        const char *data = "[ , \"foo\", \"bar\" ]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[,\"foo\",\"bar\"]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }
}

static void test_parse_array_bad_nested_elems(void)
{
    {
        const char *data = "[ \"foo\" [\"baz\"], \"bar\" ]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[\"foo\"[\"baz\"],\"bar\"]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[ \"foo\" {\"boing\": \"baz\"}, \"bar\" ]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "[\"foo\"{\"boing\":\"baz\"},\"bar\"]";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }
}

static void test_parse_object_double_colon(void)
{
    {
        const char *data = "{ \"foo\":: \"bar\" }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{\"foo\"::\"bar\"}";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }
}

static void test_parse_object_double_and_trailing_comma(void)
{
    {
        const char *data = "{ ,, }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{,,}";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{\"foo\":\"bar\",,}";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data = "{\"foo\": \"bar\", , }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        // We accept one, and only one, trailing comma.
        const char *data = "{\"foo\": \"bar\", }";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        JsonDestroy(json);
    }
}

#define assert_json_strings_eq(unescaped, escaped)     \
    {                                                  \
        char *const esc = JsonEncodeString(unescaped); \
        char *const unesc = JsonDecodeString(escaped); \
        assert_string_equal(esc, escaped);             \
        assert_string_equal(unesc, unescaped);         \
        free(esc);                                     \
        free(unesc);                                   \
    }

static void test_string_escape(void)
{
    assert_json_strings_eq("", "");
    assert_json_strings_eq(" ", " ");
    assert_json_strings_eq("\t", "\\t");
    assert_json_strings_eq("\n", "\\n");
    assert_json_strings_eq("\b", "\\b");
    assert_json_strings_eq("\f", "\\f");
    assert_json_strings_eq("\r", "\\r");
    assert_json_strings_eq("abc", "abc");
    assert_json_strings_eq("\"", "\\\"");
    assert_json_strings_eq(
        "Hello, world!\n'Blah', \"blah\".",
        "Hello, world!\\n'Blah', \\\"blah\\\".");
    assert_json_strings_eq(
        "Hello blah",
        "Hello \\u001bblah");

    const char unescaped_short_hex[] = {
        'B', 'l', 'a', 'h', '\\', 'u', '0', '0', '\0'};
    const char escaped_short_hex[] = {
        'B', 'l', 'a', 'h', '\\', '\\', 'u', '0', '0', '\0'};
    assert_json_strings_eq(unescaped_short_hex, escaped_short_hex);

    const char unescaped_invalid_u[] = {
        'B', 'l', 'a', 'h', '\\', 'u', '\0'};
    const char escaped_invalid_u[] = {
        'B', 'l', 'a', 'h', '\\', '\\', 'u', '\0'};
    assert_json_strings_eq(unescaped_invalid_u, escaped_invalid_u);

    const char unescaped_valid_hex[] = {
        'B', 'l', 'a', 'h', '\\', 'u', '0', '0', '1', 'b', '\0'};
    const char escaped_valid_hex[] = {
        'B', 'l', 'a', 'h', '\\', '\\', 'u', '0', '0', '1', 'b', '\0'};
    assert_json_strings_eq(unescaped_valid_hex, escaped_valid_hex);

    const char unescaped_invalid_hex[] = {
        'B', 'l', 'a', 'h', '\\', 'u', '0', 'z', '1', 'b', '\0'};
    const char escaped_invalid_hex[] = {
        'B', 'l', 'a', 'h', '\\', '\\', 'u', '0', 'z', '1', 'b', '\0'};
    assert_json_strings_eq(unescaped_invalid_hex, escaped_invalid_hex);
}

#define assert_json5_data_eq(_size, unescaped, escaped)           \
    {                                                             \
        Slice data = {.data = (void *) unescaped, .size = _size}; \
                                                                  \
        char *const esc = Json5EscapeData(data);                  \
                                                                  \
        assert_string_equal(esc, escaped);                        \
        free(esc);                                                \
    }

#define assert_json5_strings(unescaped, escaped)                     \
    {                                                                \
        assert_json5_data_eq(strlen(unescaped), unescaped, escaped); \
    }

static void test_string_escape_json5(void)
{
    // NUL-terminated strings, check backwards compatibility:
    assert_json5_strings("", "");
    assert_json5_strings(" ", " ");
    assert_json5_strings("\t", "\\t");
    assert_json5_strings("\n", "\\n");
    assert_json5_strings("\b", "\\b");
    assert_json5_strings("\f", "\\f");
    assert_json5_strings("\r", "\\r");
    assert_json5_strings("abc", "abc");
    assert_json5_strings("\"", "\\\"");
    assert_json5_strings(
        "Hello, world!\n'Blah', \"blah\".",
        "Hello, world!\\n'Blah', \\\"blah\\\".");

    // Encoding NUL bytes and strings without NUL-bytes:
    const char *const hello = "Hello";
    assert_json5_data_eq(strlen(hello) + 1, hello, "Hello\\0");

    char tab = '\t';
    char nul = '\0';
    assert_json5_data_eq(1, "", "\\0");
    assert_json5_data_eq(1, &tab, "\\t");
    assert_json5_data_eq(1, &nul, "\\0");

    assert_json5_data_eq(2, " ", " \\0");
    assert_json5_data_eq(4, "\0\0\0", "\\0\\0\\0\\0");

    // Non-printable byte encoding:
    const char arr[] = {1, 2, 3, 4, 0x10, 0xF0, 0xFF};
    assert_false(CharIsPrintableAscii(1));
    assert_false(CharIsPrintableAscii(2));
    assert_false(CharIsPrintableAscii(3));
    assert_false(CharIsPrintableAscii(4));
    assert_false(CharIsPrintableAscii(0x10));
    assert_false(CharIsPrintableAscii(0xF0));
    assert_false(CharIsPrintableAscii(0xFF));
    assert_json5_data_eq(7, arr, "\\x01\\x02\\x03\\x04\\x10\\xF0\\xFF");
}

static void test_json_null_not_null(void)
{
    JsonElement *json = NULL;
    assert_true(NULL_JSON(json));
    assert_false(JSON_NOT_NULL(json));

    json = JsonObjectCreate(3);
    assert_true(JSON_NOT_NULL(json));
    assert_false(NULL_JSON(json));

    JsonObjectAppendInteger(json, "one", 1);

    JsonElement *child = JsonObjectGet(json, "one");
    assert_true(JSON_NOT_NULL(child));
    assert_false(NULL_JSON(child));

    JsonObjectAppendNull(json, "two");
    child = JsonObjectGet(json, "two");
    assert_true(NULL_JSON(child));
    assert_false(JSON_NOT_NULL(child));

    JsonDestroy(json);
}

static bool check_json_object_merge_deep(const char *base_raw, const char *extra_raw, const char *expected_raw)
{
    LogSetGlobalLevel(LOG_LEVEL_DEBUG);
    bool pass = true;
    JsonElement *actual = NULL;
    JsonElement *base = NULL;
    JsonElement *extra = NULL;
    JsonElement *expected = NULL;
    char *expected_string = NULL;
    char *actual_string = NULL;

    if (JsonParse(&base_raw, &base) != JSON_PARSE_OK)
    {
        printf("error: could not parse base: %s\n", base_raw);
        pass = false;
        goto finish;
    }

    if (JsonParse(&extra_raw, &extra) != JSON_PARSE_OK)
    {
        printf("error: could not parse extra: %s\n", extra_raw);
        pass = false;
        goto finish;
    }

    if (JsonParse(&expected_raw, &expected) != JSON_PARSE_OK) {
        printf("error: could not parse expected: %s\n", expected_raw);
        pass = false;
        goto finish;
    }

    expected_string = JsonToString(expected);

    actual = JsonObjectMergeDeep(base, extra);
    if (actual == NULL || actual == base)
    {
        pass = false;
        goto finish;
    }

    actual_string = JsonToString(actual);

    if (JsonCompare(actual, expected) != 0)
    {
        pass = false;
        goto finish;
    }

    DESTROY_AND_NULL(JsonDestroy,actual);
    actual = JsonObjectMergeDeepInplace(base, extra);

    // JsonObjectMergeDeepInplace() returns the first parameter (base) so actual should point to base
    if (actual != base)
    {
        pass = false;
        goto finish;
    }

    if (JsonCompare(actual, expected))
    {
        pass = false;
        goto finish;
    }

    finish:
        if (actual_string)
        {
            if (pass == false)
            {
                printf("merged json  : %s\n", actual_string);
            }
            free(actual_string);
        }
        if (expected_string)
        {
            if (pass == false)
            {
                printf("expected json: %s\n", expected_string);
            }
            free(expected_string);
        }
        // in second step of JsonObjectMergeDeepInplace() actual should point to base
        if (actual != base)
        {
            JsonDestroy(actual);
        }
        JsonDestroy(base);
        JsonDestroy(extra);
        JsonDestroy(expected);

    return pass;
}

static void test_json_object_merge_deep()
{
    /* This unit test tests both JsonMergeObjectDeep and
     * JsonMergeObjectDeepInplace */

    assert_true(check_json_object_merge_deep(
        // base
        "{}",
        // extra
        "{}",
        // expected
        "{}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{}",
        // extra
        "{"
        "  \"variables\": {}"
        "}",
        // expected
        "{"
        "  \"variables\": {}"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {}"
        "}",
        // extra
        "{}",
        // expected
        "{"
        "  \"variables\": {}"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {}"
        "}",
        // extra
        "{"
        "  \"variables\": {}"
        "}",
        // expected
        "{"
        "  \"variables\": {}"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {}"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_one\": {}"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_one\": {}"
        "  }"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_two\": {"
        "      \"value\": [ \"/tmp/virus\" ],"
        "      \"tags\": [ \"foo\", \"bar\", \"bas\" ]"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_two\": {"
        "      \"value\": [ \"/tmp/malicious\" ],"
        "      \"comment\": [ \"Delete dangerous files!\" ]"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_two\": {"
        "      \"comment\": [ \"Delete dangerous files!\" ],"
        "      \"tags\": [ \"foo\", \"bar\", \"bas\" ],"
        "      \"value\": [ \"/tmp/virus\", \"/tmp/malicious\" ]"
        "    }"
        "  }"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"classes\": {"
        "    \"cfbs_delete_files_enable\": {"
        "      \"class_expressions\": ["
        "        \"linux.redhat::\","
        "        \"cfengine|linux::\""
        "      ],"
        "      \"tags\": [ \"bogus\", \"doofus\", \"bonkers\" ]"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_three\": {"
        "      \"value\": [ \"/tmp/malicious\" ],"
        "      \"comment\": [ \"Delete dangerous files!\" ]"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_three\": {"
        "      \"comment\": [ \"Delete dangerous files!\" ],"
        "      \"value\": [ \"/tmp/malicious\" ]"
        "    }"
        "  },"
        "  \"classes\": {"
        "    \"cfbs_delete_files_enable\": {"
        "      \"class_expressions\": ["
        "        \"linux.redhat::\","
        "        \"cfengine|linux::\""
        "      ],"
        "      \"tags\": [ \"bogus\", \"doofus\", \"bonkers\" ]"
        "    }"
        "  }"
        "}"
    ));

    // NOTE: this assert_false() test is the same as the next one with the exception of value:/tmp/foobad to check deep compare
    assert_false(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_bad\": {"
        "      \"value\": [ \"/tmp/foo\", \"/tmp/bar\" ],"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_bad\": {"
        "      \"value\": [ \"/tmp/bar\", \"/tmp/baz\" ],"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_bad\": {"
        "      \"value\": [ \"/tmp/foobad\", \"/tmp/bar\", \"/tmp/bar\", \"/tmp/baz\" ],"
        "    }"
        "  }"
        "}"
    ));

    // this second negative test checks that a mismatch of a second child of an object is found
    // note the "badchild" in the expectation
    // there was a bug as I was working here where only the number of children mattered if the first matched, any more than that were not compared
    assert_false(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables_bad_two\": {"
        "    \"cfbs:delete_files.filenames_bad_two\": {"
        "      \"value\": [ \"/tmp/foo\", \"/tmp/bar\" ],"
        "      \"badchild\": { \"one\": \"uno\", \"two\": \"due\" },"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables_bad_two\": {"
        "    \"cfbs:delete_files.filenames_bad_two\": {"
        "      \"value\": [ \"/tmp/bar\", \"/tmp/baz\" ],"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables_bad_two\": {"
        "    \"cfbs:delete_files.filenames_bad_two\": {"
        "      \"value\": [ \"/tmp/foo\", \"/tmp/bar\", \"/tmp/bar\", \"/tmp/baz\" ],"
        "      \"badchild\": { \"one\": \"uno\", \"two\": \"dva\" },"
        "    }"
        "  }"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables_four\": {"
        "    \"cfbs:delete_files.filenames_four\": {"
        "      \"value\": [ \"/tmp/foo\", \"/tmp/bar\" ],"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables_four\": {"
        "    \"cfbs:delete_files.filenames_four\": {"
        "      \"value\": [ \"/tmp/bar\", \"/tmp/baz\" ],"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables_four\": {"
        "    \"cfbs:delete_files.filenames_four\": {"
        "      \"value\": [ \"/tmp/foo\", \"/tmp/bar\", \"/tmp/bar\", \"/tmp/baz\" ],"
        "    }"
        "  }"
        "}"
    ));
    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_five\": {"
        "      \"value\": [],"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_five\": {"
        "      \"value\": {},"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_five\": {"
        "      \"value\": {},"
        "    }"
        "  }"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_six\": {"
        "      \"value\": {},"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_six\": {"
        "      \"value\": [],"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_six\": {"
        "      \"value\": [],"
        "    }"
        "  }"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_seven\": {"
        "      \"value\": [],"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_seven\": {"
        "      \"value\": 123,"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_seven\": {"
        "      \"value\": 123,"
        "    }"
        "  }"
        "}"
    ));

    assert_true(check_json_object_merge_deep(
        // base
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_eight\": {"
        "      \"value\": 123,"
        "    }"
        "  }"
        "}",
        // extra
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_eight\": {"
        "      \"value\": {},"
        "    }"
        "  }"
        "}",
        // expected
        "{"
        "  \"variables\": {"
        "    \"cfbs:delete_files.filenames_eight\": {"
        "      \"value\": {},"
        "    }"
        "  }"
        "}"
    ));
}

static void test_json_get_type_as_string()
{
    {
        JsonElement *element = JsonStringCreate("foo");
        const char *expected = "string";
        const char *actual = JsonGetTypeAsString(element);
        assert_string_equal(expected, actual);
        JsonDestroy(element);
    }

    {
        JsonElement *element = JsonIntegerCreate(42);
        const char *expected = "number";
        const char *actual = JsonGetTypeAsString(element);
        assert_string_equal(expected, actual);
        JsonDestroy(element);
    }

    {
        JsonElement *element = JsonRealCreate(3.14);
        const char *expected = "number";
        const char *actual = JsonGetTypeAsString(element);
        assert_string_equal(expected, actual);
        JsonDestroy(element);
    }

    {
        JsonElement *element = JsonBoolCreate(true);
        const char *expected = "boolean";
        const char *actual = JsonGetTypeAsString(element);
        assert_string_equal(expected, actual);
        JsonDestroy(element);
    }

    {
        JsonElement *element = JsonNullCreate(true);
        const char *expected = "null";
        const char *actual = JsonGetTypeAsString(element);
        assert_string_equal(expected, actual);
        JsonDestroy(element);
    }

    {
        JsonElement *element = JsonObjectCreate(0);
        const char *expected = "object";
        const char *actual = JsonGetTypeAsString(element);
        assert_string_equal(expected, actual);
        JsonDestroy(element);
    }

    {
        JsonElement *element = JsonArrayCreate(0);
        const char *expected = "array";
        const char *actual = JsonGetTypeAsString(element);
        assert_string_equal(expected, actual);
        JsonDestroy(element);
    }
}

static void test_json_parse_object_missing_comma(void)
{
    {
        const char *data =
            "{ \"first\": \"one\" \"second\": \"two\" }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data =
            "{\"first\":\"one\"\"second\":\"two\"}";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data =
            "{ \"first\": \"one\", \"second\": \"two\" \"third\": \"three\" }";
        JsonElement *json = NULL;
        assert_int_not_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_false(json);
    }

    {
        const char *data =
            "{ \"first\": \"one\", \"second\": \"two\" }";
        JsonElement *json = NULL;
        assert_int_equal(JSON_PARSE_OK, JsonParse(&data, &json));
        assert_true(json);
        assert_string_equal(JsonObjectGetAsString(json, "first"), "one");
        assert_string_equal(JsonObjectGetAsString(json, "second"), "two");
        JsonDestroy(json);
    }
}

static void test_parse_string_trailing_backslash(void)
{
    /* Exact-sized heap buffers so ASan catches any read past the NUL. */
    const char *const cases[] = {
        "\"x\\",
        "\"\\",
        "{\"a\": \"x\\",
        "{\"x\\",
        "[\"x\\",
        NULL
    };

    for (size_t i = 0; cases[i] != NULL; i++)
    {
        const size_t len = strlen(cases[i]);
        char *buf = xmalloc(len + 1);
        memcpy(buf, cases[i], len + 1);

        const char *data = buf;
        JsonElement *json = NULL;
        assert_int_equal(
            JSON_PARSE_ERROR_STRING_NO_DOUBLEQUOTE_END,
            JsonParse(&data, &json));
        assert_true(json == NULL);

        free(buf);
    }

    /* Escaped trailing backslash before the closing quote still parses. */
    const char *valid = "\"x\\\\\"";
    JsonElement *json = NULL;
    assert_int_equal(JSON_PARSE_OK, JsonParse(&valid, &json));
    assert_true(json != NULL);
    assert_string_equal("x\\", JsonPrimitiveGetAsString(json));
    JsonDestroy(json);
}

int main()
{
    PRINT_TEST_BANNER();
    const UnitTest tests[] = {
        unit_test(test_parse_strict_valid),
        unit_test(test_parse_strict_duplicate_member),
        unit_test(test_parse_strict_non_json_forms),
        unit_test(test_parse_strict_commas_and_array_separators),
        unit_test(test_parse_strict_control_chars_and_escapes),
        unit_test(test_parse_strict_negative_leading_zero),
        unit_test(test_parse_strict_utf8),
        unit_test(test_parse_strict_max_members),
        unit_test(test_parse_strict_large_object_linear),
        unit_test(test_parse_strict_max_depth),
        unit_test(test_parse_strict_invalid_earlier_duplicate),
        unit_test(test_parse_strict_repeated_section),
        unit_test(test_parse_strict_trailing_characters),
        unit_test(test_parse_lenient_duplicate_last_wins),
        unit_test(test_parse_strict_error_string),
        unit_test(test_parse_file_strict),
        unit_test(test_parse_file_lenient_unchanged),
        unit_test(test_array_get_string),
        unit_test(test_array_iterator),
        unit_test(test_array_remove_range),
        unit_test(test_array_extend),
        unit_test(test_copy_compare),
        unit_test(test_detach_key_from_object),
        unit_test(test_iterator_current),
        unit_test(test_merge_array),
        unit_test(test_merge_object),
        unit_test(test_new_delete),
        unit_test(test_object_duplicate_key),
        unit_test(test_object_get_array),
        unit_test(test_object_get_string),
        unit_test(test_object_get_bool),
        unit_test(test_object_iterator),
        unit_test(test_json_walk),
        unit_test(test_parse_array_bad_nested_elems),
        unit_test(test_parse_array_comma_after_brace),
        unit_test(test_parse_array_diverse),
        unit_test(test_parse_array_double_and_trailing_commas),
        unit_test(test_parse_array_extra_closing),
        unit_test(test_parse_all),
        unit_test(test_parse_array_garbage),
        unit_test(test_parse_array_nested_garbage),
        unit_test(test_parse_array_object),
        unit_test(test_parse_array_simple),
        unit_test(test_parse_bad_apple2),
        unit_test(test_parse_bad_numbers),
        unit_test(test_parse_empty_containers),
        unit_test(test_parse_empty_string),
        unit_test(test_parse_escaped_string),
        unit_test(test_parse_big_numbers),
        unit_test(test_parse_good_numbers),
        unit_test(test_parse_object_compound),
        unit_test(test_parse_object_diverse),
        unit_test(test_parse_object_double_and_trailing_comma),
        unit_test(test_parse_object_double_colon),
        unit_test(test_parse_object_escaped),
        unit_test(test_parse_object_garbage),
        unit_test(test_parse_object_nested_garbage),
        unit_test(test_parse_object_simple),
        unit_test(test_parse_primitives),
        unit_test(test_parse_trim),
        unit_test(test_parse_tzz_evil_key),
        unit_test(test_remove_key_from_object),
        unit_test(test_select),
        unit_test(test_show_array),
        unit_test(test_show_array_boolean),
        unit_test(test_show_array_compact),
        unit_test(test_show_array_empty),
        unit_test(test_show_array_infinity),
        unit_test(test_show_array_nan),
        unit_test(test_show_array_numeric),
        unit_test(test_show_array_object),
        unit_test(test_show_object_array),
        unit_test(test_show_object_boolean),
        unit_test(test_show_object_compound),
        unit_test(test_show_object_compound_compact),
        unit_test(test_show_object_escaped),
        unit_test(test_show_object_numeric),
        unit_test(test_show_object_simple),
        unit_test(test_show_string),
        unit_test(test_string_escape),
        unit_test(test_string_escape_json5),
        unit_test(test_json_null_not_null),
        unit_test(test_json_object_merge_deep),
        unit_test(test_compare_container_type_mismatch),
        unit_test(test_json_get_type_as_string),
        unit_test(test_json_parse_object_missing_comma),
        unit_test(test_parse_string_trailing_backslash),
    };

    return run_tests(tests);
}
