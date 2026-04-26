#ifndef SECURITY_H
#define SECURITY_H

#include <stddef.h>
#include <stdint.h>

/* Security validation functions */
int validate_response_size(size_t size);
int validate_json_depth(int depth);
int validate_price_range(double price);
int validate_string_length(const char *str, size_t max_len);

/* Buffer overflow prevention */
size_t safe_strncpy(char *dest, const char *src, size_t dest_size);
int is_safe_integer(int value);
int is_safe_double(double value);

/* JSON sanitization */
int sanitize_json_string(const char *input, char *output, size_t max_len);
int validate_json_schema(const char *json_str);

#endif /* SECURITY_H */