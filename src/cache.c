#include "../include/cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define CACHE_DIR  ".dollar"
#define CACHE_FILE "cache.json"

char* cache_get_path(void) {
    const char *home = getenv("HOME");
    if (!home) return NULL;

    /* home + "/" + .dollar + "/" + cache.json + null */
    size_t len = strlen(home) + 1 + strlen(CACHE_DIR) + 1 + strlen(CACHE_FILE) + 1;
    char *path = malloc(len);
    if (!path) return NULL;

    snprintf(path, len, "%s/%s/%s", home, CACHE_DIR, CACHE_FILE);
    return path;
}

int cache_load(const char *path, char **data, size_t *size, int ttl) {
    struct stat st;
    FILE *f;

    if (!path || !data || !size) return -1;

    *data = NULL;
    *size = 0;

    /* Check if file exists and get mtime */
    if (stat(path, &st) != 0) return -1;

    /* Check TTL */
    time_t now = time(NULL);
    if (now == (time_t)-1) return -1;
    if (difftime(now, st.st_mtime) > ttl) return -1;

    /* Read file */
    f = fopen(path, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    if (fsize <= 0) {
        fclose(f);
        return -1;
    }
    rewind(f);

    *data = malloc((size_t)fsize + 1);
    if (!*data) {
        fclose(f);
        return -1;
    }

    size_t nread = fread(*data, 1, (size_t)fsize, f);
    fclose(f);

    if (nread != (size_t)fsize) {
        free(*data);
        *data = NULL;
        return -1;
    }

    (*data)[nread] = '\0';
    *size = nread;

    return 0;
}

int cache_save(const char *path, const char *data, size_t size) {
    FILE *f;
    char *dir = NULL;

    if (!path || !data) return -1;

    /* Create directory ~/.dollar/ if needed */
    dir = strdup(path);
    if (!dir) return -1;

    char *slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        mkdir(dir, 0755);
    }
    free(dir);

    /* Write cache file */
    f = fopen(path, "wb");
    if (!f) return -1;

    size_t written = fwrite(data, 1, size, f);
    fclose(f);

    return (written == size) ? 0 : -1;
}
