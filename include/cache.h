#ifndef CACHE_H
#define CACHE_H

#include <stddef.h>

/**
 * Get path to cache file (~/.dollar/cache.json)
 * Caller must free the returned string.
 * Returns NULL on failure.
 */
char* cache_get_path(void);

/**
 * Load data from cache file if it exists and is within TTL.
 * @param path: cache file path
 * @param data: output pointer to data (caller must free)
 * @param size: output size
 * @param ttl: max age in seconds
 * @return 0 on success, -1 if expired/missing/error
 */
int cache_load(const char *path, char **data, size_t *size, int ttl);

/**
 * Save data to cache file, creating directory if needed.
 * @param path: cache file path
 * @param data: data to save
 * @param size: data size
 * @return 0 on success, -1 on error
 */
int cache_save(const char *path, const char *data, size_t size);

#endif /* CACHE_H */
