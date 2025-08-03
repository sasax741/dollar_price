#include <stdio.h>
#include <curl/curl.h>
#include <jansson.h>
#include <string.h>
#ifndef _WIN32
#include <net/if.h>
#endif

char *contents;

size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userp) {
    (void)userp;
    size_t realsize = size * nmemb;

    contents = realloc(contents, realsize + 1);
    if (contents == NULL) {
        fprintf(stderr, "Error de asignación de memoria\n");
        return 0;
    }

    memcpy(contents, ptr, realsize);
    contents[realsize] = 0;

    return realsize;
}

int main(void) {
#ifndef _WIN32
    CURL *curl;
    CURLcode res;

    json_t *root;
    json_error_t error;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        long my_scope_id;
        curl_easy_setopt(curl, CURLOPT_URL, "https://dolarapi.com/v1/dolares");
        my_scope_id = if_nametoindex("eth0");
        curl_easy_setopt(curl, CURLOPT_ADDRESS_SCOPE, my_scope_id);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        contents = malloc(1);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("\nSolicitud exitosa. Respuesta recibida.\n");

            root = json_loads(contents, 0, &error);

            if (root) {
                size_t i;
                size_t size = json_array_size(root);
                for (i = 0; i < size; i++) {
                    json_t *element = json_array_get(root, i);
                    const char *casa = json_string_value(json_object_get(element, "casa"));
                    const char *moneda = json_string_value(json_object_get(element, "moneda"));

                    json_t *compra_value = json_object_get(element, "compra");
                    json_t *venta_value = json_object_get(element, "venta");

                    int compraint;
                    double compra;
                    int ventaint;
                    double venta;

                    if (json_typeof(compra_value) == JSON_INTEGER) {
                        compraint = json_integer_value(compra_value);
                        compra = (double)compraint;
                    } else if (json_typeof(compra_value) == JSON_REAL) {
                        compra = json_real_value(compra_value);
                        compraint = (int)compra;
                    } else {
                        fprintf(stderr, "Tipo de dato no reconocido para compra\n");
                        continue;  
                    }

                    if (json_typeof(venta_value) == JSON_INTEGER) {
                        ventaint = json_integer_value(venta_value);
                        venta = (double)ventaint;
                    } else if (json_typeof(venta_value) == JSON_REAL) {
                        venta = json_real_value(venta_value);
                        ventaint = (int)venta;
                    } else {
                        fprintf(stderr, "Tipo de dato no reconocido para venta\n");
                        continue; 
                    }

                    printf("\n%s\n-------\n%s\nCOMPRA: %.2f\nVENTA: %.2f\n",
                           casa, moneda, compra, venta);

                    const char *fecha = json_string_value(json_object_get(element, "fechaActualizacion"));
                    printf("FECHA DE ACTUALIZACION: %s\n", fecha);
                }

                json_decref(root);
            } else {
                fprintf(stderr, "Error al parsear JSON: %s\n", error.text);
            }
        }

        curl_easy_cleanup(curl);

        free(contents);
    }

    curl_global_cleanup();
#endif
    return 0;
}