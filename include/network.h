#ifndef NETWORK_H
#define NETWORK_H

#include <curl/curl.h>
#include <stddef.h>

#include "config.h"

/* Network result structure */
typedef struct {
    char *data;
    size_t size;
    int status_code;
    int error_code;
    char error_message[256];
} NetworkResult;

/* Configuration structure */
typedef struct {
    const char *url;
    long timeout;
    int max_retries;
    int retry_backoff_ms;
} NetworkConfig;

/**
 * Initialize network subsystem
 * @return: 0 if successful, -1 if error
 */
int network_init(void);

/**
 * Perform HTTP GET request with retry logic
 * @param config: network configuration
 * @return: NetworkResult struct with data or error
 */
NetworkResult* network_get(const NetworkConfig *config);

/**
 * Free network result
 * @param result: result to free
 */
void network_free(NetworkResult *result);

/**
 * Cleanup network subsystem
 */
void network_cleanup(void);

#endif /* NETWORK_H */