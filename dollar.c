#include <stdio.h>
#include <curl/curl.h>
#include <jansson.h>
#ifndef _WIN32
#include <net/if.h>
#endif

char *contents;

size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;

    // chequedo de memoria 
    contents = realloc(contents, realsize + 1);
    if (contents == NULL) {
        fprintf(stderr, "Error de asignación de memoria\n");
        return 0;
    }

    // copia los datos 
    memcpy(contents, ptr, realsize);
    contents[realsize] = 0;

    return realsize;
}

int main(void) {
#ifndef _WIN32
    CURL *curl;
    CURLcode res;

    // Inicializo json_t
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

        // asigno memoria para contents
        contents = malloc(1);

        // realizo la solicitud, la guardo en res
        res = curl_easy_perform(curl);
        // verifico errores
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("\nSolicitud exitosa. Respuesta recibida.\n");

            // parseo la respuesta JSON 
            root = json_loads(contents, 0, &error);

            if (root) {
                // modelo el json
                size_t i;
                size_t size = json_array_size(root);
                for (i = 0; i < size; i++) {
                    json_t *element = json_array_get(root, i);
                    const char *casa = json_string_value(json_object_get(element, "casa"));
                    const char *moneda = json_string_value(json_object_get(element, "moneda"));

                    // recibo y empiezo a modelar los datos del de compra y venta
                    json_t *compra_value = json_object_get(element, "compra");
                    json_t *venta_value = json_object_get(element, "venta");

                    // creo las varaibles para almacenar dependiendo los casos
                    int compraint;
                    double compra;
                    int ventaint;
                    double venta;

                    // verifico el tipo de dato y lo guardo segun corresponda, la api devuelve valores into o double dependiendo el precio
                    if (json_typeof(compra_value) == JSON_INTEGER) {
                        compraint = json_integer_value(compra_value);
                        compra = (double)compraint; // convierto a double
                    } else if (json_typeof(compra_value) == JSON_REAL) {
                        compra = json_real_value(compra_value);
                        compraint = (int)compra; // convierto a int
                    } else {
                        fprintf(stderr, "Tipo de dato no reconocido para compra\n");
                        continue;  
                    }

                    if (json_typeof(venta_value) == JSON_INTEGER) {
                        ventaint = json_integer_value(venta_value);
                        venta = (double)ventaint; // convierto a double
                    } else if (json_typeof(venta_value) == JSON_REAL) {
                        venta = json_real_value(venta_value);
                        ventaint = (int)venta; // convierto a int
                    } else {
                        fprintf(stderr, "Tipo de dato no reconocido para venta\n");
                        continue; 
                    }

                    // imprimo los valores siempre en double por preferencia mia
                    printf("\n%s\n-------\n%s\nCOMPRA: %.2f\nVENTA: %.2f\n",
                           casa, moneda, compra, venta);

                    const char *fecha = json_string_value(json_object_get(element, "fechaActualizacion"));
                    printf("FECHA DE ACTUALIZACION: %s\n", fecha);
                }

                // libero la memoria ocupada por el JSON
                json_decref(root);
            } else {
                fprintf(stderr, "Error al parsear JSON: %s\n", error.text);
            }
        }

        // Limpieza
        curl_easy_cleanup(curl);

        // Libera la memoria de contents
        free(contents);
    }

    // Limpia la biblioteca 
    curl_global_cleanup();
#endif
    return 0;
}
