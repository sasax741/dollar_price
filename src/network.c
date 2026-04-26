#include "../include/network.h"
#include "../include/security.h"
#include "../include/config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Write callback with size validation for CURLOPT_WRITEFUNCTION
 * Prevent memory exhaustion by validating each chunk
 */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    NetworkResult *result = (NetworkResult *)userp;
    
    // Validate that adding this chunk won't exceed max size
    size_t new_size = result->size + realsize;
    if (validate_response_size(new_size) != 0) {
        fprintf(stderr, "Error: Response size would exceed maximum allowed (%d bytes)\n", 
                MAX_RESPONSE_SIZE);
        result->error_code = DOLLAR_ERROR_MEMORY;
        safe_strncpy(result->error_message, 
                    "Response size exceeded maximum allowed", 
                    sizeof(result->error_message));
        return 0; // Stop the transfer
    }
    
    char *new_data = realloc(result->data, new_size + 1);
    if (!new_data) {
        fprintf(stderr, "Error: Failed to allocate memory for response\n");
        result->error_code = DOLLAR_ERROR_MEMORY;
        safe_strncpy(result->error_message, 
                    "Failed to allocate memory for response", 
                    sizeof(result->error_message));
        return 0;
    }
    
    result->data = new_data;
    memcpy(result->data + result->size, contents, realsize);
    result->size += realsize;
    result->data[result->size] = '\0';
    
    return realsize;
}

/**
 * Initialize the network subsystem
 */
int network_init(void) {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: curl_global_init() failed: %s\n", 
                curl_easy_strerror(res));
        return -1;
    }
    return 0;
}

/**
 * Perform a single HTTP GET request
 * @param curl: CURL handle
 * @param config: network configuration
 * @param result: result structure to fill
 * @return: 0 if successful, -1 if error
 */
static int perform_request(CURL *curl, const NetworkConfig *config, NetworkResult *result) {
    CURLcode res;
    long response_code;
    
    // Configure CURL handle
    curl_easy_setopt(curl, CURLOPT_URL, config->url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config->timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "dollar-price-checker/1.0.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    
    // Perform the request
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        result->error_code = DOLLAR_ERROR_NETWORK;
        safe_strncpy(result->error_message, 
                    curl_easy_strerror(res),
                    sizeof(result->error_message));
        return -1;
    }
    
    // Get HTTP response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    result->status_code = (int)response_code;
    
    if (response_code != 200) {
        result->error_code = DOLLAR_ERROR_INVALID_RESPONSE;
        snprintf(result->error_message, sizeof(result->error_message),
                 "HTTP error: %ld", response_code);
        return -1;
    }
    
    // Validate response size one final time
    if (validate_response_size(result->size) != 0) {
        result->error_code = DOLLAR_ERROR_INVALID_RESPONSE;
        safe_strncpy(result->error_message,
                    "Response size validation failed",
                    sizeof(result->error_message));
        return -1;
    }
    
    return 0;
}

/**
 * Perform HTTP GET request with retry logic
 */
NetworkResult* network_get(const NetworkConfig *config) {
    if (!config || !config->url) {
        fprintf(stderr, "Error: Invalid network configuration\n");
        return NULL;
    }
    
    // Allocate result structure
    NetworkResult *result = calloc(1, sizeof(NetworkResult));
    if (!result) {
        fprintf(stderr, "Error: Failed to allocate result structure\n");
        return NULL;
    }
    
    // Initialize result
    result->error_code = DOLLAR_OK;
    safe_strncpy(result->error_message, "Success", sizeof(result->error_message));
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        result->error_code = DOLLAR_ERROR_NETWORK;
        safe_strncpy(result->error_message, "Failed to initialize CURL", sizeof(result->error_message));
        return result;
    }
    
    // Retry logic
    int attempts = 0;
    int backoff_ms = config->retry_backoff_ms;
    
    while (attempts <= config->max_retries) {
        if (attempts > 0) {
            // Exponential backoff
            int sleep_ms = backoff_ms * (1 << (attempts - 1));
            fprintf(stderr, "Retry attempt %d in %d ms...\n", attempts, sleep_ms);
            usleep(sleep_ms * 1000);
        }
        
        // Clear previous result
        if (result->data) {
            free(result->data);
            result->data = NULL;
            result->size = 0;
        }
        
        // Perform request
        if (perform_request(curl, config, result) == 0) {
            // Success!
            curl_easy_cleanup(curl);
            return result;
        }
        
        attempts++;
        
        // Check if we should retry
        if (result->status_code >= 500 || 
            strstr(result->error_message, "timeout") ||
            strstr(result->error_message, "connection")) {
            // Server errors and network issues are retryable
            continue;
        }
        
        // Non-retryable error
        break;
    }
    
    curl_easy_cleanup(curl);
    
    if (attempts > config->max_retries) {
        safe_strncpy(result->error_message,
                    "Max retries exceeded",
                    sizeof(result->error_message));
    }
    
    return result;
}

/**
 * Free network result
 */
void network_free(NetworkResult *result) {
    if (result) {
        if (result->data) {
            free(result->data);
        }
        free(result);
    }
}

/**
 * Cleanup network subsystem
 */
void network_cleanup(void) {
    curl_global_cleanup();
}