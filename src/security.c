#include "../include/security.h"
#include "../include/config.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/**
 * Validate response size to prevent memory exhaustion
 * @param size: size to validate
 * @return: 0 if valid, -1 if too large
 */
int validate_response_size(size_t size) {
    if (size > MAX_RESPONSE_SIZE) {
        return -1;
    }
    return 0;
}

/**
 * Validate JSON depth to prevent stack overflow
 * @param depth: current depth
 * @return: 0 if valid, -1 if too deep
 */
int validate_json_depth(int depth) {
    if (depth > MAX_JSON_DEPTH) {
        return -1;
    }
    return 0;
}

/**
 * Validate price range to prevent invalid data
 * @param price: price to validate
 * @return: 0 if valid, -1 if out of range
 */
int validate_price_range(double price) {
    if (price < 0.01 || price > 10000.0) {
        return -1;
    }
    return 0;
}

/**
 * Safe string length validation
 * @param str: string to validate
 * @param max_len: maximum allowed length
 * @return: 0 if valid, -1 if too long
 */
int validate_string_length(const char *str, size_t max_len) {
    if (str == NULL) {
        return -1;
    }
    
    size_t len = strlen(str);
    if (len > max_len) {
        return -1;
    }
    
    return 0;
}

/**
 * Safe string copy with bounds checking
 * @param dest: destination buffer
 * @param src: source string
 * @param dest_size: size of destination buffer
 * @return: number of bytes copied
 */
size_t safe_strncpy(char *dest, const char *src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return 0;
    }
    
    size_t src_len = strlen(src);
    size_t copy_len = (src_len < dest_size - 1) ? src_len : dest_size - 1;
    
    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';
    
    return copy_len;
}

/**
 * Validate integer within safe range
 * @param value: integer to validate
 * @return: 0 if valid, -1 if unsafe
 */
int is_safe_integer(int value) {
    return (value >= INT32_MIN && value <= INT32_MAX) ? 0 : -1;
}

/**
 * Validate double within safe range
 * @param value: double to validate
 * @return: 0 if valid, -1 if unsafe
 */
int is_safe_double(double value) {
    if (isnan(value)) {
        return -1;
    }
    if (isinf(value)) {
        return -1;
    }
    return 0;
}

/**
 * Sanitize JSON string by escaping dangerous characters
 * @param input: input string
 * @param output: output buffer
 * @param max_len: maximum output buffer size
 * @return: 0 if successful, -1 if buffer too small
 */
int sanitize_json_string(const char *input, char *output, size_t max_len) {
    if (input == NULL || output == NULL || max_len == 0) {
        return -1;
    }
    
    size_t out_pos = 0;
    size_t in_len = strlen(input);
    
    for (size_t i = 0; i < in_len && out_pos < max_len - 1; i++) {
        unsigned char c = input[i];
        
        // Allow only printable ASCII and extended characters
        if ((c >= 32 && c <= 126) || c >= 160) {
            output[out_pos++] = c;
        } else {
            // Replace control characters with space
            output[out_pos++] = ' ';
        }
    }
    
    output[out_pos] = '\0';
    return (out_pos < max_len) ? 0 : -1;
}

/**
 * Basic JSON schema validation
 * @param json_str: JSON string to validate
 * @return: 0 if valid, -1 if invalid
 */
int validate_json_schema(const char *json_str) {
    if (json_str == NULL) {
        return -1;
    }
    
    // Basic checks for JSON structure
    if (strlen(json_str) < 2) {
        return -1;
    }
    
    // Check for valid JSON array start
    if (json_str[0] != '[') {
        return -1;
    }
    
    // Check for balanced brackets
    int bracket_count = 0;
    int in_string = 0;
    
    for (size_t i = 0; json_str[i] != '\0'; i++) {
        if (json_str[i] == '"' && (i == 0 || json_str[i-1] != '\\')) {
            in_string = !in_string;
        }
        
        if (!in_string) {
            if (json_str[i] == '[' || json_str[i] == '{') {
                bracket_count++;
            } else if (json_str[i] == ']' || json_str[i] == '}') {
                bracket_count--;
            }
        }
        
        if (bracket_count < 0) {
            return -1;
        }
    }
    
    return (bracket_count == 0) ? 0 : -1;
}