#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define MAX_CASA_LEN 64
#define MAX_MONEDA_LEN 16
#define MAX_FECHA_LEN 128
#define MAX_RATES 20

/* Parser error codes */
#define PARSER_OK 0
#define PARSER_ERROR_INVALID_JSON      -100
#define PARSER_ERROR_INVALID_TYPE      -101
#define PARSER_ERROR_MISSING_FIELD     -102
#define PARSER_ERROR_ARRAY_TOO_LARGE   -103
#define PARSER_ERROR_PRICE_OUT_OF_RANGE -104
#define PARSER_ERROR_NULL_INPUT        -105

typedef struct {
    char casa[MAX_CASA_LEN];
    char moneda[MAX_MONEDA_LEN];
    double compra;
    double venta;
    char fecha[MAX_FECHA_LEN];
} DollarRate;

/**
 * Parse JSON response into DollarRate array
 * @param json_str: JSON string to parse
 * @param rates: pointer to rates array (caller must free with parser_free)
 * @param count: pointer to store count
 * @return: PARSER_OK if successful, negative error code if failed
 */
int parse_dollar_rates(const char *json_str, DollarRate **rates, size_t *count);

/**
 * Free array of DollarRate allocated by parse_dollar_rates
 */
void parser_free(DollarRate *rates);

#endif /* PARSER_H */
