# Verificador de Precio del Dólar

Esta es una simple aplicación de línea de comandos para verificar el precio actual del dólar estadounidense en Argentina. Obtiene los datos desde la API pública en `https://dolarapi.com`.

El objetivo de este proyecto es tener un comando simple disponible en el PATH del sistema para consultar rápidamente las cotizaciones del dólar desde cualquier terminal.

## Plataformas Soportadas

- **Linux**
- **macOS** (usando Homebrew)

## Prerrequisitos

Antes de compilar, necesitas tener las siguientes dependencias instaladas:

- **`gcc`**: El Compilador GNU.
- **`make`**: La herramienta para automatizar la compilación.
- **`libcurl`**: Una librería para realizar peticiones HTTP.
- **`jansson`**: Una librería para procesar datos en formato JSON.

### Instalación de Dependencias

- **En Debian/Ubuntu (Linux):**
  ```bash
  sudo apt-get update
  sudo apt-get install build-essential libcurl4-openssl-dev libjansson-dev
  ```

- **En macOS (usando Homebrew):**
  Primero, instala [Homebrew](https://brew.sh) si no lo tienes. Luego, instala las dependencias:
  ```bash
  brew install curl jansson
  ```

## Compilación e Instalación

El proyecto usa un `Makefile` para simplificar todo el proceso. Sigue estos pasos:

1.  **Compilar e Instalar:**
    Abre tu terminal, navega al directorio del proyecto y ejecuta el siguiente comando:
    ```bash
    sudo make install
    ```
    Este comando primero compilará la aplicación y luego la instalará en `/usr/local/bin` para que esté disponible en todo el sistema.

2.  **Verificar la instalación:**
    Una vez finalizado, puedes ejecutar `dollar` desde cualquier lugar en tu terminal para ver las cotizaciones.

## Limpieza

Para eliminar el ejecutable compilado del directorio del proyecto, ejecuta:

```bash
make clean
```

---

## Funcionamiento del Código (`dollar.c`)

El código fuente está escrito en C y su lógica se puede dividir en los siguientes pasos:

1.  **Petición HTTP**: Se utiliza la librería `libcurl` para realizar una petición `GET` a la URL de la API (`https://dolarapi.com/v1/dolares`).

2.  **Recepción de Datos**: Una función `WriteCallback` se encarga de recibir los datos de la respuesta. Esta función reserva memoria dinámicamente (`realloc`) para almacenar la respuesta completa, que llega en formato de texto (JSON).

3.  **Procesamiento de JSON**: Una vez recibida la respuesta, la librería `jansson` se utiliza para interpretar (parsear) el texto JSON y convertirlo en una estructura de datos que C puede manejar. La respuesta de la API es un array de objetos, donde cada objeto representa un tipo de dólar.

4.  **Iteración y Muestra de Datos**: El código recorre cada elemento del array JSON. Para cada tipo de dólar, extrae los valores de las claves `casa`, `moneda`, `compra`, `venta` y `fechaActualizacion`.

5.  **Manejo de Tipos**: La API puede devolver los precios como números enteros o decimales. El código verifica el tipo de dato de los precios (`compra` y `venta`) y los convierte a un formato `double` para poder mostrarlos de manera uniforme con dos decimales.

6.  **Limpieza de Memoria**: Al finalizar, se libera toda la memoria asignada para la respuesta y los objetos JSON, y se cierran las conexiones de `libcurl` para evitar fugas de memoria y mantener el programa eficiente.

## Funcionamiento del `Makefile`

El `Makefile` automatiza el proceso de compilación, instalación y limpieza del proyecto.

- **Variables**: Se definen variables al inicio para el compilador (`CC`), los flags de compilación (`CFLAGS_BASE`), los flags del enlazador (`LDFLAGS_BASE`) y el nombre del ejecutable (`TARGET`). Esto facilita la modificación futura.

- **Detección de Sistema Operativo**: El `Makefile` detecta automáticamente si se está ejecutando en `Linux` o `Darwin` (macOS) usando el comando `uname -s`.

- **Compatibilidad con macOS**: Si detecta macOS, busca la ruta de instalación de Homebrew. Si la encuentra, añade las rutas de las librerías (`/include` y `/lib`) a los flags del compilador y enlazador. Esto permite que `gcc` encuentre `libcurl` y `jansson` instaladas con Homebrew. También verifica si las librerías están instaladas y avisa al usuario si no lo están.

- **Reglas (Targets)**:
    - `all`: Es la regla por defecto. Su única dependencia es el `TARGET` (`dollar`), por lo que al ejecutar `make`, se activa la compilación. En macOS, también depende de la verificación de librerías.
    - `$(TARGET)`: La regla principal de compilación. Se ejecuta si `dollar.c` ha sido modificado. Lanza el compilador `gcc` con todos los flags y librerías necesarios.
    - `install`: Depende de `all`, asegurando que el programa esté compilado antes de instalar. Copia el ejecutable `dollar` a `/usr/local/bin`, requiriendo permisos de `sudo`.
    - `clean`: Elimina el archivo ejecutable (`dollar`) del directorio del proyecto.
