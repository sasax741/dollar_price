#include "../include/parser.h"
#include "../include/security.h"
#include "../include/config.h"
#include <jansson.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse_dollar_rates(const char *json_str, DollarRate **rates, size_t *count) {
    json_error_t error;
    json_t *root;
    size_t i;

    if (!json_str || !rates || !count) {
        return PARSER_ERROR_NULL_INPUT;
    }

    *rates = NULL;
    *count = 0;

    /* Validate JSON structure before parsing */
    if (validate_json_schema(json_str) != 0) {
        return PARSER_ERROR_INVALID_JSON;
    }

    root = json_loads(json_str, 0, &error);
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return PARSER_ERROR_INVALID_JSON;
    }

    if (!json_is_array(root)) {
        json_decref(root);
        return PARSER_ERROR_INVALID_JSON;
    }

    size_t array_size = json_array_size(root);
    if (array_size > MAX_RATES) {
        json_decref(root);
        return PARSER_ERROR_ARRAY_TOO_LARGE;
    }

    if (array_size == 0) {
        json_decref(root);
        return PARSER_OK;
    }

    *rates = calloc(array_size, sizeof(DollarRate));
    if (!*rates) {
        json_decref(root);
        return PARSER_ERROR_INVALID_JSON;
    }

    for (i = 0; i < array_size; i++) {
        json_t *element = json_array_get(root, i);
        json_t *casa_val = json_object_get(element, "casa");
        json_t *moneda_val = json_object_get(element, "moneda");
        json_t *compra_val = json_object_get(element, "compra");
        json_t *venta_val = json_object_get(element, "venta");
        json_t *fecha_val = json_object_get(element, "fechaActualizacion");
        const char *casa_str, *moneda_str;
        double compra, venta;

        /* Validate required fields exist */
        if (!casa_val || !moneda_val || !compra_val || !venta_val) {
            parser_free(*rates);
            *rates = NULL;
            json_decref(root);
            return PARSER_ERROR_MISSING_FIELD;
        }

        /* Validate string fields */
        if (!json_is_string(casa_val) || !json_is_string(moneda_val)) {
            parser_free(*rates);
            *rates = NULL;
            json_decref(root);
            return PARSER_ERROR_INVALID_TYPE;
        }

        casa_str = json_string_value(casa_val);
        moneda_str = json_string_value(moneda_val);

        if (validate_string_length(casa_str, MAX_CASA_LEN - 1) != 0 ||
            validate_string_length(moneda_str, MAX_MONEDA_LEN - 1) != 0) {
            parser_free(*rates);
            *rates = NULL;
            json_decref(root);
            return PARSER_ERROR_INVALID_TYPE;
        }

        /* Validate numeric fields */
        if (!json_is_number(compra_val) || !json_is_number(venta_val)) {
            parser_free(*rates);
            *rates = NULL;
            json_decref(root);
            return PARSER_ERROR_INVALID_TYPE;
        }

        if (json_is_integer(compra_val)) {
            compra = (double)json_integer_value(compra_val);
        } else {
            compra = json_real_value(compra_val);
        }

        if (json_is_integer(venta_val)) {
            venta = (double)json_integer_value(venta_val);
        } else {
            venta = json_real_value(venta_val);
        }

        /* Validate price ranges */
        if (validate_price_range(compra) != 0 || validate_price_range(venta) != 0) {
            parser_free(*rates);
            *rates = NULL;
            json_decref(root);
            return PARSER_ERROR_PRICE_OUT_OF_RANGE;
        }

        if (is_safe_double(compra) != 0 || is_safe_double(venta) != 0) {
            parser_free(*rates);
            *rates = NULL;
            json_decref(root);
            return PARSER_ERROR_INVALID_TYPE;
        }

        /* Safe copy string fields */
        safe_strncpy((*rates)[i].casa, casa_str, MAX_CASA_LEN);
        safe_strncpy((*rates)[i].moneda, moneda_str, MAX_MONEDA_LEN);
        (*rates)[i].compra = compra;
        (*rates)[i].venta = venta;

        /* Optional fecha field */
        if (fecha_val && json_is_string(fecha_val)) {
            const char *fecha_str = json_string_value(fecha_val);
            if (validate_string_length(fecha_str, MAX_FECHA_LEN - 1) == 0) {
                safe_strncpy((*rates)[i].fecha, fecha_str, MAX_FECHA_LEN);
            }
        }
    }

    *count = array_size;
    json_decref(root);
    return PARSER_OK;
}

void parser_free(DollarRate *rates) {
    free(rates);
}
