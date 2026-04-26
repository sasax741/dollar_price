#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdlib.h>

#include "../include/parser.h"
#include "../include/security.h"

static void test_parse_valid_json(void **state) {
    (void)state;
    const char *json = "["
        "{\"casa\":\"blue\",\"moneda\":\"USD\",\"compra\":1400,\"venta\":1420.5,"
        "\"fechaActualizacion\":\"2026-04-24T18:55:00.000Z\"}"
    "]";
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = parse_dollar_rates(json, &rates, &count);
    assert_int_equal(ret, PARSER_OK);
    assert_int_equal(count, 1);
    if (rates) {
        assert_string_equal(rates[0].casa, "blue");
        assert_string_equal(rates[0].moneda, "USD");
        assert_true(rates[0].compra - 1400.0 < 0.01);
        assert_true(rates[0].venta - 1420.5 < 0.01);
        assert_string_equal(rates[0].fecha, "2026-04-24T18:55:00.000Z");
        parser_free(rates);
    }
}

static void test_parse_multiple_rates(void **state) {
    (void)state;
    const char *json = "["
        "{\"casa\":\"oficial\",\"moneda\":\"USD\",\"compra\":1000,\"venta\":1020},"
        "{\"casa\":\"blue\",\"moneda\":\"USD\",\"compra\":1100,\"venta\":1120}"
    "]";
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = parse_dollar_rates(json, &rates, &count);
    assert_int_equal(ret, PARSER_OK);
    assert_int_equal(count, 2);
    if (rates) {
        assert_string_equal(rates[0].casa, "oficial");
        assert_string_equal(rates[1].casa, "blue");
        parser_free(rates);
    }
}

static void test_parse_empty_array(void **state) {
    (void)state;
    const char *json = "[]";
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = parse_dollar_rates(json, &rates, &count);
    assert_int_equal(ret, PARSER_OK);
    assert_int_equal(count, 0);
    parser_free(rates);
}

static void test_parse_missing_field(void **state) {
    (void)state;
    const char *json = "["
        "{\"moneda\":\"USD\",\"compra\":1000,\"venta\":1020}"
    "]";
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = parse_dollar_rates(json, &rates, &count);
    assert_int_not_equal(ret, PARSER_OK);
    assert_null(rates);
}

static void test_parse_wrong_compra_type(void **state) {
    (void)state;
    const char *json = "["
        "{\"casa\":\"blue\",\"moneda\":\"USD\",\"compra\":\"mil\",\"venta\":1020}"
    "]";
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = parse_dollar_rates(json, &rates, &count);
    assert_int_equal(ret, PARSER_ERROR_INVALID_TYPE);
    assert_null(rates);
}

static void test_parse_price_out_of_range(void **state) {
    (void)state;
    const char *json = "["
        "{\"casa\":\"blue\",\"moneda\":\"USD\",\"compra\":100000,\"venta\":1020}"
    "]";
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = parse_dollar_rates(json, &rates, &count);
    assert_int_equal(ret, PARSER_ERROR_PRICE_OUT_OF_RANGE);
    assert_null(rates);
}

static void test_parse_null_input(void **state) {
    (void)state;
    DollarRate *rates = (DollarRate*)0x1;
    size_t count = 1;
    int ret = parse_dollar_rates(NULL, &rates, &count);
    assert_int_equal(ret, PARSER_ERROR_NULL_INPUT);
    assert_non_null(rates);
}

static void test_parse_invalid_json(void **state) {
    (void)state;
    const char *json = "not json at all";
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = parse_dollar_rates(json, &rates, &count);
    assert_int_equal(ret, PARSER_ERROR_INVALID_JSON);
    assert_null(rates);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parse_valid_json),
        cmocka_unit_test(test_parse_multiple_rates),
        cmocka_unit_test(test_parse_empty_array),
        cmocka_unit_test(test_parse_missing_field),
        cmocka_unit_test(test_parse_wrong_compra_type),
        cmocka_unit_test(test_parse_price_out_of_range),
        cmocka_unit_test(test_parse_null_input),
        cmocka_unit_test(test_parse_invalid_json),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
