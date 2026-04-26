#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/network.h"
#include "../include/parser.h"
#include "../include/config.h"
#include "../include/cache.h"

#define OUTPUT_TABLE 0
#define OUTPUT_JSON  1

/* ANSI color codes */
#define CLR_BOLD   "\033[1m"
#define CLR_CYAN   "\033[36m"
#define CLR_GREEN  "\033[32m"
#define CLR_YELLOW "\033[33m"
#define CLR_WHITE  "\033[37m"
#define CLR_RESET  "\033[0m"

#define c(p) (use_color ? (p) : "")
#define C   CLR_RESET

static void print_help(void) {
    printf("Uso: dollar [opciones]\n\n");
    printf("Opciones:\n");
    printf("  --help            Mostrar esta ayuda\n");
    printf("  --format=json     Salida en formato JSON\n");
    printf("  --no-colors       Desactivar colores en la salida\n");
    printf("  --timeout=N       Timeout de conexion en segundos (def: %d)\n", DEFAULT_TIMEOUT);
    printf("  --cache-ttl=N     Tiempo de vida del cache en segundos (def: %d)\n", DEFAULT_CACHE_TTL);
    printf("  --no-cache        No usar cache en caso de fallo\n\n");
    printf("Variables de entorno:\n");
    printf("  DOLLAR_NO_COLOR   Desactivar colores (cualquier valor)\n");
    printf("  DOLLAR_TIMEOUT    Timeout de conexion en segundos\n");
    printf("  DOLLAR_CACHE_TTL  Tiempo de vida del cache en segundos\n");
}

/**
 * Parse integer from env var or fallback to default
 */
static int parse_int_env(const char *val, int def) {
    if (!val || !val[0]) return def;
    int n = atoi(val);
    return n > 0 ? n : def;
}

/**
 * "2026-04-24T16:08:00.000Z" -> "24/04/2026 16:08"
 */
static void format_date(const char *iso, char *buf, size_t size) {
    struct tm tm = {0};
    if (strptime(iso, "%Y-%m-%dT%H:%M:%S", &tm)) {
        strftime(buf, size, "%d/%m/%Y %H:%M", &tm);
    } else {
        buf[0] = '\0';
    }
}

static void display_table(const DollarRate *rates, size_t count,
                          int use_color, int cached) {
    for (size_t i = 0; i < count; i++) {
        printf("%s%s%s\n", c(CLR_BOLD), rates[i].casa, c(C));
        for (int j = 0; rates[i].casa[j]; j++) printf("-");
        printf("\n%s\n", rates[i].moneda);
        printf("  %sCOMPRA:%s %s%.2f%s\n", c(CLR_GREEN), c(C), c(CLR_BOLD), rates[i].compra, c(C));
        printf("  %sVENTA:%s  %s%.2f%s\n", c(CLR_YELLOW), c(C), c(CLR_BOLD), rates[i].venta, c(C));
        if (rates[i].fecha[0]) {
            char fecha_fmt[128];
            format_date(rates[i].fecha, fecha_fmt, sizeof(fecha_fmt));
            if (fecha_fmt[0]) {
                printf("  %s%s%s\n", c(CLR_WHITE), fecha_fmt, c(C));
            }
        }
        if (i < count - 1) printf("\n");
    }
    if (cached) {
        printf("%s[cached]%s\n", c(CLR_WHITE), c(C));
    }
}

static void display_json(const DollarRate *rates, size_t count, int cached) {
    printf("[");
    for (size_t i = 0; i < count; i++) {
        if (i > 0) printf(",");
        printf("\n  {");
        printf("\"casa\":\"%s\",", rates[i].casa);
        printf("\"moneda\":\"%s\",", rates[i].moneda);
        printf("\"compra\":%.2f,", rates[i].compra);
        printf("\"venta\":%.2f", rates[i].venta);
        if (rates[i].fecha[0]) {
            printf(",\"fecha\":\"%s\"", rates[i].fecha);
        }
        if (cached) {
            printf(",\"cached\":true");
        }
        printf("}");
    }
    if (count > 0) printf("\n");
    printf("]\n");
}

int main(int argc, char **argv) {
    NetworkResult *res = NULL;
    DollarRate *rates = NULL;
    size_t count = 0;
    int ret = PARSER_ERROR_NULL_INPUT;
    int output_format = OUTPUT_TABLE;
    int use_color = 1;
    int cached = 0;
    int no_cache = 0;
    long timeout = DEFAULT_TIMEOUT;
    int cache_ttl = DEFAULT_CACHE_TTL;
    char *cache_path = NULL;
    char *cached_data = NULL;
    size_t cached_size = 0;
    char *json_data = NULL;

    /* Parse env vars */
    if (getenv("DOLLAR_NO_COLOR") && getenv("DOLLAR_NO_COLOR")[0]) {
        use_color = 0;
    }
    timeout = parse_int_env(getenv("DOLLAR_TIMEOUT"), DEFAULT_TIMEOUT);
    cache_ttl = parse_int_env(getenv("DOLLAR_CACHE_TTL"), DEFAULT_CACHE_TTL);

    /* Parse args */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "--format=json") == 0) {
            output_format = OUTPUT_JSON;
        } else if (strcmp(argv[i], "--no-colors") == 0) {
            use_color = 0;
        } else if (strcmp(argv[i], "--no-cache") == 0) {
            no_cache = 1;
        } else if (sscanf(argv[i], "--timeout=%ld", &timeout) == 1) {
            if (timeout < 1) timeout = DEFAULT_TIMEOUT;
        } else if (sscanf(argv[i], "--cache-ttl=%d", &cache_ttl) == 1) {
            if (cache_ttl < 0) cache_ttl = DEFAULT_CACHE_TTL;
        }
    }

    /* Default network config */
    NetworkConfig config = {
        .url = "https://dolarapi.com/v1/ambito/dolares",
        .timeout = timeout,
        .max_retries = MAX_RETRIES,
        .retry_backoff_ms = RETRY_BACKOFF_MS
    };

    /* Cache path */
    cache_path = cache_get_path();

    /* Initialize network */
    if (network_init() != 0) {
        fprintf(stderr, "Error: Failed to initialize network\n");
        goto cleanup;
    }

    /* Fetch data */
    res = network_get(&config);
    if (!res || res->error_code != DOLLAR_OK) {
        /* Try cache fallback */
        if (!no_cache && cache_path &&
            cache_load(cache_path, &cached_data, &cached_size, cache_ttl) == 0) {
            fprintf(stderr, "API no disponible. Usando datos en cache.\n");
            json_data = cached_data;
            cached = 1;
        } else {
            fprintf(stderr, "Error: %s\n", res ? res->error_message : "API no disponible");
            if (!no_cache) fprintf(stderr, "No hay cache disponible.\n");
            goto cleanup;
        }
    } else {
        json_data = res->data;
        /* Save to cache for next time */
        if (cache_path) {
            cache_save(cache_path, res->data, res->size);
        }
    }

    /* Parse JSON */
    ret = parse_dollar_rates(json_data, &rates, &count);
    if (ret != PARSER_OK) {
        fprintf(stderr, "Error parsing response: code %d\n", ret);
        goto cleanup;
    }

    /* Display results */
    if (output_format == OUTPUT_JSON) {
        display_json(rates, count, cached);
    } else {
        display_table(rates, count, use_color, cached);
    }

cleanup:
    parser_free(rates);
    network_free(res);
    network_cleanup();
    free(cache_path);
    if (cached_data && cached_data != json_data) free(cached_data);

    return ret == PARSER_OK ? 0 : 1;
}
