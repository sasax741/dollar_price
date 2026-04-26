#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>
#include <math.h>

#include "../include/security.h"
#include "../include/config.h"

static void test_validate_response_size_valid(void **state) {
    (void)state;
    assert_int_equal(validate_response_size(0), 0);
    assert_int_equal(validate_response_size(100), 0);
    assert_int_equal(validate_response_size(MAX_RESPONSE_SIZE), 0);
}

static void test_validate_response_size_too_large(void **state) {
    (void)state;
    assert_int_equal(validate_response_size(MAX_RESPONSE_SIZE + 1), -1);
    assert_int_equal(validate_response_size(SIZE_MAX), -1);
}

static void test_validate_price_range_valid(void **state) {
    (void)state;
    assert_int_equal(validate_price_range(0.01), 0);
    assert_int_equal(validate_price_range(100.0), 0);
    assert_int_equal(validate_price_range(10000.0), 0);
}

static void test_validate_price_range_invalid(void **state) {
    (void)state;
    assert_int_equal(validate_price_range(0.0), -1);
    assert_int_equal(validate_price_range(0.001), -1);
    assert_int_equal(validate_price_range(-1.0), -1);
    assert_int_equal(validate_price_range(10000.01), -1);
}

static void test_validate_string_length_null(void **state) {
    (void)state;
    assert_int_equal(validate_string_length(NULL, 10), -1);
}

static void test_validate_string_length_valid(void **state) {
    (void)state;
    assert_int_equal(validate_string_length("hola", 10), 0);
    assert_int_equal(validate_string_length("", 10), 0);
    assert_int_equal(validate_string_length("exactamente", 11), 0);
}

static void test_validate_string_length_too_long(void **state) {
    (void)state;
    assert_int_equal(validate_string_length("demasiado largo", 5), -1);
}

static void test_safe_strncpy_normal(void **state) {
    (void)state;
    char buf[16] = {0};
    size_t n = safe_strncpy(buf, "hello", sizeof(buf));
    assert_int_equal(n, 5);
    assert_string_equal(buf, "hello");
}

static void test_safe_strncpy_truncation(void **state) {
    (void)state;
    char buf[8] = {0};
    size_t n = safe_strncpy(buf, "hello world!", sizeof(buf));
    assert_int_equal(n, 7);  /* sizeof(buf) - 1 */
    assert_string_equal(buf, "hello w");
}

static void test_safe_strncpy_null(void **state) {
    (void)state;
    char buf[16] = {0};
    assert_int_equal(safe_strncpy(NULL, "test", 10), 0);
    assert_int_equal(safe_strncpy(buf, NULL, 10), 0);
    assert_int_equal(safe_strncpy(buf, "test", 0), 0);
}

static void test_is_safe_double_normal(void **state) {
    (void)state;
    assert_int_equal(is_safe_double(0.0), 0);
    assert_int_equal(is_safe_double(100.5), 0);
    assert_int_equal(is_safe_double(-1.0), 0);
}

static void test_is_safe_double_nan(void **state) {
    (void)state;
    assert_int_equal(is_safe_double(NAN), -1);
}

static void test_is_safe_double_inf(void **state) {
    (void)state;
    assert_int_equal(is_safe_double(INFINITY), -1);
}

static void test_validate_json_schema_valid_array(void **state) {
    (void)state;
    assert_int_equal(validate_json_schema("[1, 2, 3]"), 0);
    assert_int_equal(validate_json_schema("[{}]"), 0);
}

static void test_validate_json_schema_invalid(void **state) {
    (void)state;
    assert_int_equal(validate_json_schema(""), -1);
    assert_int_equal(validate_json_schema("[]]"), -1);
    assert_int_equal(validate_json_schema("{\"a\":1}"), -1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_validate_response_size_valid),
        cmocka_unit_test(test_validate_response_size_too_large),
        cmocka_unit_test(test_validate_price_range_valid),
        cmocka_unit_test(test_validate_price_range_invalid),
        cmocka_unit_test(test_validate_string_length_null),
        cmocka_unit_test(test_validate_string_length_valid),
        cmocka_unit_test(test_validate_string_length_too_long),
        cmocka_unit_test(test_safe_strncpy_normal),
        cmocka_unit_test(test_safe_strncpy_truncation),
        cmocka_unit_test(test_safe_strncpy_null),
        cmocka_unit_test(test_is_safe_double_normal),
        cmocka_unit_test(test_is_safe_double_nan),
        cmocka_unit_test(test_is_safe_double_inf),
        cmocka_unit_test(test_validate_json_schema_valid_array),
        cmocka_unit_test(test_validate_json_schema_invalid),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
