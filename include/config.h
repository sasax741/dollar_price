#ifndef CONFIG_H
#define CONFIG_H

/* Security limits */
#define MAX_RESPONSE_SIZE (1024 * 1024)  /* 1MB max response */
#define MAX_JSON_DEPTH 10                /* Prevent JSON bombs */
#define MAX_STRING_LENGTH 256            /* Max string lengths */

/* Network configuration */
#define DEFAULT_TIMEOUT 30               /* 30 seconds */
#define MAX_RETRIES 3                    /* Retry attempts */
#define RETRY_BACKOFF_MS 1000            /* Base backoff */

/* Cache configuration */
#define DEFAULT_CACHE_TTL 300            /* 5 minutes */
#define CACHE_DIR ".dollar"
#define CONFIG_FILE "config.json"

/* Output formatting */
#define DEFAULT_OUTPUT_FORMAT "table"
#define DEFAULT_COLORS true
#define DEFAULT_CURRENCY "ARS"

/* Error codes */
#define DOLLAR_OK 0
#define DOLLAR_ERROR_NETWORK -1
#define DOLLAR_ERROR_JSON_PARSE -2
#define DOLLAR_ERROR_MEMORY -3
#define DOLLAR_ERROR_INVALID_RESPONSE -4
#define DOLLAR_ERROR_TIMEOUT -5
#define DOLLAR_ERROR_CACHE -6
#define DOLLAR_ERROR_CONFIG -7
#define DOLLAR_ERROR_PERMISSION -8

#endif /* CONFIG_H */